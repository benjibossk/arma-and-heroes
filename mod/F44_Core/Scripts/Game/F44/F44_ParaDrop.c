//------------------------------------------------------------------------------------------------
// Paratrooper air drop (server authoritative).
//
// A C-47 (AeroplaneHavens C47 Dakota mod, soft dependency: spawned by resource name at runtime)
// circles the battle layout, flown by a vanilla AI pilot through the Aviation-Toolkit
// TK_AIPilotComponent (a Move waypoint outside any airport = fly there and orbit it).
// Each plane carries an "AIR DROP" SCR_SpawnPoint. Only PARA class soldiers may use it: the
// spawned character is seated in a free cargo seat. The player jumps when he wants (C-47 cargo
// door action) and opens the chute manually (Grey Mans Chute Framework, GMS_FreeFallComponent,
// added to every character by that mod). No class of those mods is referenced here: without the
// mods, the prefab fails to load and the air drop is just disabled.
//
// Battle JSON: "airDrop": { "enabled", "altitude", "radius", "speed", "planeCount": { "<SIDE>": n } }
//------------------------------------------------------------------------------------------------

class F44_AirDropConfig
{
	bool m_bEnabled;
	float m_fAltitude = 350;	// m AGL at spawn (the TK pilot then holds its own cruise AGL, 350 for the C-47)
	float m_fRadius = 1200;	// m from the layout centre at spawn (TK orbit radius is the prefab's, 1200)
	float m_fSpeed = 65;		// m/s initial speed at spawn
	ref map<string, int> m_mPlaneCount = new map<string, int>();	// side key -> planes
	ref array<string> m_aClasses = {"PARA"};	// class keys allowed to use the air drop
	bool m_bTestSeatBot;	// debug: seat a PARA bot through the spawn path after each plane spawn (server log)

	int GetPlaneCount(string sideKey)
	{
		if (!m_bEnabled)
			return 0;
		return m_mPlaneCount.Get(sideKey);
	}

	static F44_AirDropConfig Read(LoadContext root, string attKey, string defKey)
	{
		F44_AirDropConfig c = new F44_AirDropConfig();
		if (!root || !root.StartObject("airDrop"))
			return c;
		root.ReadValue("enabled", c.m_bEnabled);
		root.ReadValue("altitude", c.m_fAltitude);
		root.ReadValue("radius", c.m_fRadius);
		root.ReadValue("speed", c.m_fSpeed);
		root.ReadValue("testSeatBot", c.m_bTestSeatBot);
		array<string> classes = {};
		if (root.ReadValue("classes", classes) && !classes.IsEmpty())
			c.m_aClasses = classes;
		if (root.StartObject("planeCount"))
		{
			int n;
			if (root.ReadValue(attKey, n))
				c.m_mPlaneCount.Set(attKey, n);
			n = 0;
			if (root.ReadValue(defKey, n))
				c.m_mPlaneCount.Set(defKey, n);
			root.EndObject();
		}
		root.EndObject();
		return c;
	}
}

//------------------------------------------------------------------------------------------------
class F44_AirDropPlane
{
	int m_iIndex;
	string m_sSideKey;
	string m_sFaction;
	float m_fAngle;			// spawn bearing around the centre (deg)
	IEntity m_Plane;
	IEntity m_Pilot;
	SCR_AIGroup m_Group;
	AIWaypoint m_Waypoint;
	SCR_SpawnPoint m_Point;
	float m_fRespawnIn;		// > 0: waiting to respawn

	//! Free cargo seats (not occupied, not reserved)
	int FreeSeats()
	{
		if (!m_Plane)
			return 0;
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(m_Plane.FindComponent(BaseCompartmentManagerComponent));
		if (!cm)
			return 0;
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		int n;
		foreach (BaseCompartmentSlot s : slots)
		{
			if (s.GetType() == ECompartmentType.CARGO && !s.IsOccupied() && !s.IsReserved())
				n++;
		}
		return n;
	}

	//! Teleport a freshly spawned character into the first free cargo seat
	bool SeatCharacter(IEntity character)
	{
		if (!m_Plane || !character)
			return false;
		SCR_CompartmentAccessComponent ca = SCR_CompartmentAccessComponent.Cast(character.FindComponent(SCR_CompartmentAccessComponent));
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(m_Plane.FindComponent(BaseCompartmentManagerComponent));
		if (!ca || !cm)
			return false;
		character.SetOrigin(m_Plane.GetOrigin());
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		ChimeraWorld world = GetGame().GetWorld();
		foreach (BaseCompartmentSlot s : slots)
		{
			if (s.GetType() != ECompartmentType.CARGO || s.IsOccupied() || (s.IsReserved() && !s.IsReservedBy(character)))
				continue;
			bool ok = ca.GetInVehicle(m_Plane, s, true, -1, ECloseDoorAfterActions.INVALID, world.IsGameTimePaused());
			PrintFormat("[F44] AirDrop: seat %1 in plane %2 (%3) -> %4", slots.Find(s), m_iIndex, m_sSideKey, ok);
			return ok;
		}
		Print("[F44] AirDrop: plane full", LogLevel.WARNING);
		return false;
	}

	bool IsAlive()
	{
		if (!m_Plane)
			return false;
		SCR_DamageManagerComponent dm = SCR_DamageManagerComponent.GetDamageManager(m_Plane);
		if (dm && dm.GetState() == EDamageState.DESTROYED)
			return false;
		if (!m_Pilot)
			return false;
		SCR_DamageManagerComponent pdm = SCR_DamageManagerComponent.GetDamageManager(m_Pilot);
		if (pdm && pdm.GetState() == EDamageState.DESTROYED)
			return false;
		return true;
	}
}

//------------------------------------------------------------------------------------------------
class F44_AirDrop
{
	static const ResourceName C47_PREFAB = "{A2B54744389B4105}Prefabs/Vehicles/Aircrafts/C47/AeroplaneHaven-C47.et";
	static const string POINT_NAME = "AIR DROP";
	static const float RESPAWN_SEC = 90;
	static const float DESPAWN_WRECK_SEC = 60;

	protected static F44_AirDrop s_Instance;
	protected ref F44_AirDropConfig m_Cfg;
	protected ref array<ref F44_AirDropPlane> m_aPlanes = {};
	protected vector m_vCentre;
	protected float m_fAccu;
	protected bool m_bDisabled;

	static F44_AirDrop GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void Init(F44_AirDropConfig cfg, string attKey, string defKey)
	{
		s_Instance = this;
		m_Cfg = cfg;
		PrintFormat("[F44] AirDrop: init (enabled=%1)", cfg && cfg.m_bEnabled);
		if (!cfg || !cfg.m_bEnabled)
			return;
		int sectors = F44_SectorManager.Count();
		if (sectors == 0)
		{
			Print("[F44] AirDrop: no sector, disabled", LogLevel.WARNING);
			return;
		}
		vector sum;
		for (int i = 0; i < sectors; i++)
			sum = sum + F44_SectorManager.GetSector(i).GetCenter();
		m_vCentre = sum / sectors;
		m_vCentre[1] = GetGame().GetWorld().GetSurfaceY(m_vCentre[0], m_vCentre[2]);

		Resource res = Resource.Load(C47_PREFAB);
		if (!res || !res.IsValid())
		{
			Print("[F44] AirDrop: C-47 prefab not loaded (AeroplaneHavens C47 mod missing?), air drop disabled", LogLevel.WARNING);
			m_bDisabled = true;
			return;
		}

		F44_Catalog cat = F44_Catalog.Get();
		array<string> sides = {attKey, defKey};
		int idx;
		foreach (int s, string sideKey : sides)
		{
			int count = cfg.GetPlaneCount(sideKey);
			for (int p = 0; p < count; p++)
			{
				F44_AirDropPlane pl = new F44_AirDropPlane();
				pl.m_iIndex = idx;
				pl.m_sSideKey = sideKey;
				pl.m_sFaction = cat.GetFactionKey(sideKey);
				pl.m_fAngle = s * 180 + p * 360 / Math.Max(count, 1) / 2;
				m_aPlanes.Insert(pl);
				idx++;
				SpawnPlane(pl);
			}
		}
		PrintFormat("[F44] AirDrop: %1 plane(s) over %2, alt %3 m, radius %4 m", m_aPlanes.Count(), m_vCentre, cfg.m_fAltitude, cfg.m_fRadius);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnPlane(F44_AirDropPlane pl)
	{
		BaseWorld world = GetGame().GetWorld();
		float a = pl.m_fAngle * Math.DEG2RAD;
		vector pos = m_vCentre + Vector(Math.Sin(a), 0, Math.Cos(a)) * m_Cfg.m_fRadius;
		pos[1] = world.GetSurfaceY(pos[0], pos[2]) + m_Cfg.m_fAltitude;
		// heading tangent to the circle (clockwise)
		float yaw = pl.m_fAngle + 90;
		vector mat[4];
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), mat);
		mat[3] = pos;
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform = mat;

		Resource res = Resource.Load(C47_PREFAB);
		pl.m_Plane = GetGame().SpawnEntityPrefab(res, world, params);
		if (!pl.m_Plane)
		{
			Print("[F44] AirDrop: C-47 spawn failed", LogLevel.ERROR);
			pl.m_fRespawnIn = RESPAWN_SEC;
			return;
		}
		Physics ph = pl.m_Plane.GetPhysics();
		if (ph)
			ph.SetVelocity(mat[2] * m_Cfg.m_fSpeed);

		// pilot + spawn point a moment later (not in the plane's init frame)
		GetGame().GetCallqueue().CallLater(SpawnCrew, 2000, false, pl.m_iIndex);
		pl.m_fRespawnIn = 0;
		PrintFormat("[F44] AirDrop: C-47 #%1 (%2) spawned at %3", pl.m_iIndex, pl.m_sSideKey, pos);
		if (m_Cfg.m_bTestSeatBot)
			GetGame().GetCallqueue().CallLater(TestSeat, 10000, false, pl.m_iIndex);
	}

	//! Debug (testSeatBot): same class gate + seating as a player spawn, with a bot
	protected void TestSeat(int index)
	{
		if (index < 0 || index >= m_aPlanes.Count())
			return;
		F44_AirDropPlane pl = m_aPlanes[index];
		if (!pl.m_Plane)
			return;
		ResourceName prefab;
		foreach (string c : m_Cfg.m_aClasses)
		{
			prefab = F44_Catalog.Get().GetClassPrefab(F44_Catalog.MakeClassKey(pl.m_sSideKey, c));
			if (!prefab.IsEmpty())
				break;
		}
		string cls = ClassOfPrefab(pl.m_sFaction, prefab);
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pl.m_Plane.GetOrigin();
		IEntity ch = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), params);
		bool seated = ch && IsAllowedClass(cls) && pl.SeatCharacter(ch);
		PrintFormat("[F44] AirDrop TEST: bot class '%1' allowed=%2 seated=%3 free seats=%4 plane pos=%5 speed=%6 m/s",
			cls, IsAllowedClass(cls), seated, pl.FreeSeats(), pl.m_Plane.GetOrigin(), pl.m_Plane.GetPhysics().GetVelocity().Length());
		GetGame().GetCallqueue().CallLater(TestTrack, 20000, true, index);
	}

	protected void TestTrack(int index)
	{
		F44_AirDropPlane pl = m_aPlanes[index];
		if (!pl.m_Plane)
			return;
		vector p = pl.m_Plane.GetOrigin();
		float agl = p[1] - GetGame().GetWorld().GetSurfaceY(p[0], p[2]);
		PrintFormat("[F44] AirDrop TRACK #%1: pos %2 AGL %3 m, dist centre %4 m, speed %5 m/s, free seats %6",
			index, p, agl, vector.DistanceXZ(p, m_vCentre), pl.m_Plane.GetPhysics().GetVelocity().Length(), pl.FreeSeats());
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnCrew(int index)
	{
		if (index < 0 || index >= m_aPlanes.Count())
			return;
		F44_AirDropPlane pl = m_aPlanes[index];
		if (!pl.m_Plane)
			return;
		SpawnPilot(pl);
		CreatePoint(pl);
	}

	//! Vanilla AI of the side in the pilot seat + Move waypoint on the centre (TK pilot: orbit it)
	protected void SpawnPilot(F44_AirDropPlane pl)
	{
		F44_Catalog cat = F44_Catalog.Get();
		ResourceName groupPrefab;
		F44_CatalogEntry ge = cat.GetEntry(pl.m_sSideKey + "_AIGROUP");
		if (ge)
			groupPrefab = ge.m_sPrefab;
		ResourceName charPrefab;
		array<string> candidates = {"PILOT", "CREW", "PARA", "RIFLEMAN"};
		foreach (string c : candidates)
		{
			ResourceName rn = cat.GetClassPrefab(F44_Catalog.MakeClassKey(pl.m_sSideKey, c));
			if (rn.IsEmpty())
				continue;
			Resource r = Resource.Load(rn);
			if (r && r.IsValid())
			{
				charPrefab = rn;
				break;
			}
		}
		if (groupPrefab.IsEmpty() || charPrefab.IsEmpty())
		{
			PrintFormat("[F44] AirDrop: no %1_AIGROUP / class prefab for the pilot", pl.m_sSideKey, level: LogLevel.WARNING);
			return;
		}
		BaseWorld world = GetGame().GetWorld();
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vCentre;	// group/pilot created on the ground, then teleported in the seat
		Resource gres = Resource.Load(groupPrefab);
		Resource cres = Resource.Load(charPrefab);
		IEntity ge2 = GetGame().SpawnEntityPrefab(gres, world, params);
		pl.m_Group = SCR_AIGroup.Cast(ge2);
		if (!pl.m_Group)
		{
			PrintFormat("[F44] AirDrop: pilot group spawn failed (%1 -> %2)", groupPrefab, ge2, level: LogLevel.WARNING);
			return;
		}
		pl.m_Group.SetFaction(GetGame().GetFactionManager().GetFactionByKey(pl.m_sFaction));
		pl.m_Pilot = GetGame().SpawnEntityPrefab(cres, world, params);
		if (!pl.m_Pilot)
		{
			Print("[F44] AirDrop: pilot spawn failed " + charPrefab, LogLevel.WARNING);
			return;
		}
		AIControlComponent ctrl = AIControlComponent.Cast(pl.m_Pilot.FindComponent(AIControlComponent));
		if (ctrl)
			ctrl.ActivateAI();
		pl.m_Group.AddAIEntityToGroup(pl.m_Pilot);
		SCR_CompartmentAccessComponent ca = SCR_CompartmentAccessComponent.Cast(pl.m_Pilot.FindComponent(SCR_CompartmentAccessComponent));
		bool seated;
		if (ca)
			seated = ca.MoveInVehicle(pl.m_Plane, ECompartmentType.PILOT, true);
		// the pilot must not get out (vanilla AI may want to dismount an aircraft it cannot "drive")
		SCR_DamageManagerComponent pdm = SCR_DamageManagerComponent.GetDamageManager(pl.m_Pilot);
		if (pdm)
			pdm.EnableDamageHandling(false);

		EntitySpawnParams wp = new EntitySpawnParams();
		wp.TransformMode = ETransformMode.WORLD;
		wp.Transform[3] = m_vCentre;
		pl.m_Waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(Resource.Load(F44_AIDirector.WP_MOVE), world, wp));
		if (pl.m_Waypoint)
		{
			pl.m_Waypoint.SetCompletionRadius(1);
			pl.m_Group.AddWaypoint(pl.m_Waypoint);
		}
		PrintFormat("[F44] AirDrop: pilot seated=%1 waypoint=%2", seated, pl.m_Waypoint != null);
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName FindHqPrefab(string factionKey)
	{
		foreach (SCR_SpawnPoint sp : SCR_SpawnPoint.GetSpawnPoints())
		{
			if (sp.GetFactionKey() != factionKey || sp.F44_GetAirDropPlane())
				continue;
			EntityPrefabData pd = sp.GetPrefabData();
			if (pd)
				return pd.GetPrefabName();
		}
		return ResourceName.Empty;
	}

	protected void CreatePoint(F44_AirDropPlane pl)
	{
		ResourceName prefab = FindHqPrefab(pl.m_sFaction);
		if (prefab.IsEmpty())
		{
			Print("[F44] AirDrop: no HQ spawn prefab for " + pl.m_sFaction, LogLevel.WARNING);
			return;
		}
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = m_vCentre;
		pl.m_Point = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), params));
		if (!pl.m_Point)
			return;
		pl.m_Point.SetFactionKey(pl.m_sFaction);
		string name = POINT_NAME;
		if (m_Cfg.GetPlaneCount(pl.m_sSideKey) > 1)
			name = POINT_NAME + " " + (pl.m_iIndex + 1);
		pl.m_Point.SetSpawnPointName(name);
		pl.m_Point.F44_SetAirDropPlane(pl);
		PrintFormat("[F44] AirDrop: spawn point '%1' registered for %2", name, pl.m_sFaction);
	}

	//------------------------------------------------------------------------------------------------
	protected void Despawn(F44_AirDropPlane pl)
	{
		if (pl.m_Point)
			SCR_EntityHelper.DeleteEntityAndChildren(pl.m_Point);
		if (pl.m_Waypoint)
			SCR_EntityHelper.DeleteEntityAndChildren(pl.m_Waypoint);
		IEntity wreck = pl.m_Plane;
		IEntity pilot = pl.m_Pilot;
		IEntity group = pl.m_Group;
		pl.m_Point = null;
		pl.m_Waypoint = null;
		pl.m_Plane = null;
		pl.m_Pilot = null;
		pl.m_Group = null;
		GetGame().GetCallqueue().CallLater(DeleteLater, DESPAWN_WRECK_SEC * 1000, false, wreck, pilot, group);
	}

	protected void DeleteLater(IEntity a, IEntity b, IEntity c)
	{
		if (b)
			SCR_EntityHelper.DeleteEntityAndChildren(b);
		if (a)
			SCR_EntityHelper.DeleteEntityAndChildren(a);
		if (c)
			SCR_EntityHelper.DeleteEntityAndChildren(c);
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (m_aPlanes.IsEmpty() || m_bDisabled)
			return;
		m_fAccu += dt;
		if (m_fAccu < 2)
			return;
		float step = m_fAccu;
		m_fAccu = 0;
		foreach (F44_AirDropPlane pl : m_aPlanes)
		{
			if (pl.m_fRespawnIn > 0)
			{
				pl.m_fRespawnIn -= step;
				if (pl.m_fRespawnIn <= 0)
					SpawnPlane(pl);
				continue;
			}
			if (!pl.IsAlive())
			{
				PrintFormat("[F44] AirDrop: C-47 #%1 lost, respawn in %2 s", pl.m_iIndex, RESPAWN_SEC);
				Despawn(pl);
				pl.m_fRespawnIn = RESPAWN_SEC;
				continue;
			}
			if (pl.m_Point)
			{
				// keep the map icon over the plane; full plane = unavailable
				vector p = pl.m_Plane.GetOrigin();
				p[1] = GetGame().GetWorld().GetSurfaceY(p[0], p[2]);
				pl.m_Point.SetOrigin(p);
				bool en = pl.FreeSeats() > 0;
				if (pl.m_Point.IsSpawnPointEnabled() != en)
					pl.m_Point.SetSpawnPointEnabled_S(en);
			}
		}
	}

	void Cleanup()
	{
		foreach (F44_AirDropPlane pl : m_aPlanes)
		{
			if (pl.m_Point)
				SCR_EntityHelper.DeleteEntityAndChildren(pl.m_Point);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Is this class key allowed to use the air drop?
	bool IsAllowedClass(string classKey)
	{
		if (!m_Cfg || classKey.IsEmpty())
			return false;
		return m_Cfg.m_aClasses.Contains(classKey);
	}

	//! Class of a character prefab for a side faction (catalog)
	static string ClassOfPrefab(string factionKey, ResourceName prefab)
	{
		F44_Catalog cat = F44_Catalog.Get();
		if (!cat)
			return string.Empty;
		string sideKey = cat.GetSideFromFaction(factionKey);
		if (sideKey.IsEmpty())
			return string.Empty;
		return cat.FindClassByPrefab(sideKey, prefab);
	}
}

//------------------------------------------------------------------------------------------------
//! Air drop spawn point behaviour (server side, flag set only on the authority)
modded class SCR_SpawnPoint
{
	protected F44_AirDropPlane m_F44AirPlane;

	void F44_SetAirDropPlane(F44_AirDropPlane pl)
	{
		m_F44AirPlane = pl;
	}

	F44_AirDropPlane F44_GetAirDropPlane()
	{
		return m_F44AirPlane;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanReserveFor_S(playerId, result))
			return false;
		if (!m_F44AirPlane)
			return true;
		if (m_F44AirPlane.FreeSeats() <= 0)
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_VEHICLE_FULL;
			return false;
		}
		// class gate on the selected loadout (the spawned prefab is checked again in PrepareSpawnedEntity_S)
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		SCR_PlayerLoadoutComponent plc;
		if (pc)
			plc = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
		F44_AirDrop ad = F44_AirDrop.GetInstance();
		if (plc && plc.GetLoadout() && ad)
		{
			string cls = F44_AirDrop.ClassOfPrefab(GetFactionKey(), plc.GetLoadout().GetLoadoutResource());
			if (!cls.IsEmpty() && !ad.IsAllowedClass(cls))
			{
				PrintFormat("[F44] AirDrop: player %1 refused (class %2 is not a paratrooper)", playerId, cls);
				result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED;
				return false;
			}
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool PrepareSpawnedEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!m_F44AirPlane)
			return super.PrepareSpawnedEntity_S(requestComponent, data, entity);
		if (!IsSpawnPointEnabled() || !entity)
			return false;
		F44_AirDrop ad = F44_AirDrop.GetInstance();
		string prefab;
		EntityPrefabData pd = entity.GetPrefabData();
		if (pd)
			prefab = pd.GetPrefabName();
		string cls = F44_AirDrop.ClassOfPrefab(GetFactionKey(), prefab);
		if (!ad || !ad.IsAllowedClass(cls))
		{
			PrintFormat("[F44] AirDrop: spawn refused, class '%1' is not a paratrooper", cls, level: LogLevel.WARNING);
			return false;
		}
		return m_F44AirPlane.SeatCharacter(entity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanFinalizeSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!super.CanFinalizeSpawn_S(requestComponent, data, entity))
			return false;
		if (!m_F44AirPlane)
			return true;
		SCR_CompartmentAccessComponent ca = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));
		return !ca || !ca.IsGettingIn();
	}
}

//------------------------------------------------------------------------------------------------
//! Hooks kept in this file (F44_BattleConfig / F44_GameModeComponent are shared with other work)
modded class F44_BattleConfig
{
	ref F44_AirDropConfig m_AirDrop;	// optional "airDrop" object

	override bool FromJson(LoadContext root)
	{
		bool ok = super.FromJson(root);
		m_AirDrop = F44_AirDropConfig.Read(root, m_sAttacker, m_sDefender);
		return ok;
	}
}

modded class F44_GameModeComponent
{
	protected ref F44_AirDrop m_AirDrop = new F44_AirDrop();
	protected float m_fAirDropWait;
	protected bool m_bAirDropInit;

	//! Own 1 s timer (base timers are bound to base methods): init 10 s after the battle starts, then tick
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer())
			GetGame().GetCallqueue().CallLater(F44_AirDropTick, 1000, true);
	}

	protected void F44_AirDropTick()
	{
		if (m_bEnded)
		{
			if (m_bAirDropInit)
				m_AirDrop.Cleanup();
			GetGame().GetCallqueue().Remove(F44_AirDropTick);
			return;
		}
		if (!m_bRunning || !m_Config)
			return;
		if (!m_bAirDropInit)
		{
			m_fAirDropWait += 1;
			if (m_fAirDropWait < 10)
				return;
			m_bAirDropInit = true;
			m_AirDrop.Init(m_Config.m_AirDrop, m_sAttacker, m_sDefender);
			return;
		}
		m_AirDrop.Tick(1);
	}

	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(F44_AirDropTick);
		super.OnDelete(owner);
	}
}
