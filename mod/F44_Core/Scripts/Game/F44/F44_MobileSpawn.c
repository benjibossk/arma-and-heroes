// Arma & Heroes - mobile spawn points on halftracks / trucks (SERVER authoritative).
// Design: docs/SPAWN_UI.md "Mobile spawn". A tracked battle vehicle whose key matches the config
// gets a copy of its side's HQ SCR_SpawnPoint prefab. The point is NOT parented to the hull: it is
// only usable while the vehicle is stopped, and it is (re)created at the tailgate when the vehicle
// stops somewhere new, so clients always get a correct replicated position (entity creation
// replicates the transform; a later server SetOrigin on a non-moving entity would not).
// Fed by F44_GameModeComponent.ServerTick (Tick) and PreparePlayerEntity_S (OnSpawnAt).

//------------------------------------------------------------------------------------------------
class F44_MobileSpawnConfig
{
	bool m_bEnabled = true;
	int m_iUsesHalftrack = 8;	// spawns per deployment
	int m_iUsesTruck = 3;
	float m_fCooldownSec = 20;	// between two spawns on the same vehicle
	float m_fPenaltySec = 10;	// SCR_SpawnPoint respawn time (deploy delay); H&G half-track 20 s total = base 10 + 10
	float m_fMaxSpeedKmh = 2;
	float m_fStopSec = 3;	// stationary this long before enabling
	float m_fEnemyRadius = 40;
	float m_fDriverAbsentSec = 15;	// no driver this long -> point removed
	bool m_bRequireDriver = true;	// false = tests / empty parked vehicles also spawn
	float m_fBackOffset = 3;	// m behind the vehicle origin
	ref array<string> m_aHalftrackKeys = {"GER_SDKFZ251*", "US_M3A1*"};	// "*" suffix = prefix match
	ref array<string> m_aTruckKeys = {"GER_TRUCK", "GER_TRUCK_OPEN", "US_TRUCK*"};

	//------------------------------------------------------------------------------------------------
	static F44_MobileSpawnConfig Read(LoadContext root)
	{
		F44_MobileSpawnConfig c = new F44_MobileSpawnConfig();
		if (!root || !root.StartObject("mobileSpawn"))
			return c;
		root.ReadValue("enabled", c.m_bEnabled);
		root.ReadValue("usesHalftrack", c.m_iUsesHalftrack);
		root.ReadValue("usesTruck", c.m_iUsesTruck);
		root.ReadValue("cooldownSec", c.m_fCooldownSec);
		root.ReadValue("penaltySec", c.m_fPenaltySec);
		root.ReadValue("maxSpeedKmh", c.m_fMaxSpeedKmh);
		root.ReadValue("stopSec", c.m_fStopSec);
		root.ReadValue("enemyRadius", c.m_fEnemyRadius);
		root.ReadValue("driverAbsentSec", c.m_fDriverAbsentSec);
		root.ReadValue("backOffset", c.m_fBackOffset);
		root.ReadValue("requireDriver", c.m_bRequireDriver);
		array<string> ht = {};
		if (root.ReadValue("halftrackKeys", ht) && !ht.IsEmpty())
			c.m_aHalftrackKeys = ht;
		array<string> tr = {};
		if (root.ReadValue("truckKeys", tr) && !tr.IsEmpty())
			c.m_aTruckKeys = tr;
		root.EndObject();
		return c;
	}

	//------------------------------------------------------------------------------------------------
	static bool MatchKey(string key, array<string> patterns)
	{
		foreach (string p : patterns)
		{
			if (p.EndsWith("*"))
			{
				if (key.StartsWith(p.Substring(0, p.Length() - 1)))
					return true;
			}
			else if (key == p)
				return true;
		}
		return false;
	}

	//! uses per deployment for this vehicle key, 0 = not a mobile spawn vehicle
	int UsesFor(string key)
	{
		if (MatchKey(key, m_aHalftrackKeys))
			return m_iUsesHalftrack;
		if (MatchKey(key, m_aTruckKeys))
			return m_iUsesTruck;
		return 0;
	}
}

//------------------------------------------------------------------------------------------------
class F44_MobileSpawnEntry
{
	IEntity m_Vehicle;
	string m_sKey;
	string m_sFaction;
	SCR_SpawnPoint m_Point;
	int m_iUses;
	float m_fNextUse;	// world time s
	float m_fStopped;	// s stationary
	float m_fNoDriver;	// s without friendly driver
	bool m_bHadDriver;
	string m_sState;	// last logged state
}

//------------------------------------------------------------------------------------------------
class F44_MobileSpawns
{
	static const string NAME_PREFIX = "MOBILE ";
	protected static const float STEP = 1;

	protected ref F44_MobileSpawnConfig m_Cfg = new F44_MobileSpawnConfig();
	protected ref array<ref F44_MobileSpawnEntry> m_aEntries = {};
	protected float m_fAccu;
	protected string m_sQueryFaction;
	protected bool m_bEnemyFound;

	//------------------------------------------------------------------------------------------------
	void SetConfig(F44_MobileSpawnConfig cfg)
	{
		if (cfg)
			m_Cfg = cfg;
	}

	//------------------------------------------------------------------------------------------------
	//! API: register a battle vehicle (ignored if its key is not a mobile spawn vehicle)
	void Register(IEntity veh, string key, string factionKey)
	{
		if (!m_Cfg.m_bEnabled || !veh || factionKey.IsEmpty())
			return;
		int uses = m_Cfg.UsesFor(key);
		if (uses <= 0)
			return;
		DamageManagerComponent dmg = DamageManagerComponent.Cast(veh.FindComponent(DamageManagerComponent));
		if (dmg && dmg.GetState() == EDamageState.DESTROYED)
			return;
		foreach (F44_MobileSpawnEntry x : m_aEntries)
		{
			if (x.m_Vehicle == veh)
				return;
		}
		F44_MobileSpawnEntry e = new F44_MobileSpawnEntry();
		e.m_Vehicle = veh;
		e.m_sKey = key;
		e.m_sFaction = factionKey;
		e.m_iUses = uses;
		m_aEntries.Insert(e);
		PrintFormat("[F44] Mobile spawn registered on %1 (%2, %3 uses)", key, factionKey, uses);
	}

	//------------------------------------------------------------------------------------------------
	//! Called from PreparePlayerEntity_S: consume one use if the player picked a mobile point
	void OnSpawnAt(SCR_SpawnPoint sp)
	{
		if (!sp)
			return;
		foreach (F44_MobileSpawnEntry e : m_aEntries)
		{
			if (e.m_Point != sp)
				continue;
			e.m_iUses--;
			e.m_fNextUse = Now() + m_Cfg.m_fCooldownSec;
			PrintFormat("[F44] Mobile spawn used on %1 (%2 left)", e.m_sKey, e.m_iUses);
			if (sp.IsSpawnPointEnabled())
				sp.SetSpawnPointEnabled_S(false);	// cooldown; Tick re-enables
			UpdateName(e);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> owners, array<int> progress)
	{
		m_fAccu += dt;
		if (m_fAccu < STEP)
			return;
		float step = m_fAccu;
		m_fAccu = 0;

		for (int i = m_aEntries.Count() - 1; i >= 0; i--)
		{
			F44_MobileSpawnEntry e = m_aEntries[i];
			string reason = Evaluate(e, step, owners, progress);
			if (reason == "REMOVE")
			{
				DeletePoint(e, "vehicle destroyed/abandoned");
				m_aEntries.Remove(i);
				continue;
			}
			if (reason == "NODRIVER")
			{
				DeletePoint(e, "driver absent");
				continue;
			}
			bool want = reason.IsEmpty();
			bool parked = want || (reason != "moving" && reason != "no driver");	// stopped: point exists, maybe disabled
			if (parked && !e.m_Point)
				CreatePoint(e);
			else if (parked && e.m_Point && vector.DistanceSq(e.m_Point.GetOrigin(), PointPos(e.m_Vehicle)) > 25)
			{
				DeletePoint(e, "vehicle moved");
				CreatePoint(e);
			}
			if (e.m_Point && e.m_Point.IsSpawnPointEnabled() != want)
				e.m_Point.SetSpawnPointEnabled_S(want);
			string st = "ENABLED";
			if (!want)
				st = "DISABLED (" + reason + ")";
			if (e.m_Point && st != e.m_sState)
			{
				e.m_sState = st;
				PrintFormat("[F44] Mobile spawn %1 %2", e.m_sKey, st);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "" = should be enabled; "REMOVE" / "NODRIVER" = delete point; anything else = disable reason
	protected string Evaluate(F44_MobileSpawnEntry e, float step, array<int> owners, array<int> progress)
	{
		IEntity veh = e.m_Vehicle;
		if (!veh)
			return "REMOVE";
		DamageManagerComponent dmg = DamageManagerComponent.Cast(veh.FindComponent(DamageManagerComponent));
		if (dmg && dmg.GetState() == EDamageState.DESTROYED)
			return "REMOVE";

		// driver
		bool driver;
		Vehicle v = Vehicle.Cast(veh);
		if (v)
		{
			SCR_ChimeraCharacter pilot = SCR_ChimeraCharacter.Cast(v.GetPilot());
			if (pilot && pilot.GetFactionKey() == e.m_sFaction)
			{
				CharacterControllerComponent cc = pilot.GetCharacterController();
				driver = cc && !cc.IsDead();
			}
		}
		if (driver)
		{
			e.m_fNoDriver = 0;
			e.m_bHadDriver = true;
		}
		else
		{
			e.m_fNoDriver += step;
			if (e.m_bHadDriver && e.m_fNoDriver > m_Cfg.m_fDriverAbsentSec)
				return "NODRIVER";
		}

		// speed
		Physics ph = veh.GetPhysics();
		float kmh;
		if (ph)
			kmh = ph.GetVelocity().Length() * 3.6;
		if (kmh < m_Cfg.m_fMaxSpeedKmh)
			e.m_fStopped += step;
		else
			e.m_fStopped = 0;

		if (!driver && m_Cfg.m_bRequireDriver)
			return "no driver";
		if (e.m_fStopped < m_Cfg.m_fStopSec)
			return "moving";
		if (e.m_iUses <= 0)
			return "no uses left";
		if (Now() < e.m_fNextUse)
			return "cooldown";

		vector pos = veh.GetOrigin();
		int n = F44_SectorManager.Count();
		for (int s = 0; s < n; s++)
		{
			F44_SectorComponent sc = F44_SectorManager.GetSector(s);
			if (sc && sc.IsInside(pos) && F44_SectorSpawns.IsContested(s, owners, progress))
				return "contested sector";
		}
		if (EnemyNear(pos, e.m_sFaction))
			return "enemy near";
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected bool EnemyNear(vector pos, string faction)
	{
		m_sQueryFaction = faction;
		m_bEnemyFound = false;
		GetGame().GetWorld().QueryEntitiesBySphere(pos, m_Cfg.m_fEnemyRadius, QueryEnemy, null, EQueryEntitiesFlags.DYNAMIC);
		return m_bEnemyFound;
	}

	protected bool QueryEnemy(IEntity ent)
	{
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(ent);
		if (!ch)
			return true;
		string fk = ch.GetFactionKey();
		if (fk.IsEmpty() || fk == m_sQueryFaction)
			return true;
		CharacterControllerComponent cc = ch.GetCharacterController();
		if (cc && cc.IsDead())
			return true;
		m_bEnemyFound = true;
		return false;	// stop query
	}

	//------------------------------------------------------------------------------------------------
	protected vector PointPos(IEntity veh)
	{
		vector mat[4];
		veh.GetWorldTransform(mat);
		vector pos = mat[3] - mat[2] * m_Cfg.m_fBackOffset;
		pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		return pos;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreatePoint(F44_MobileSpawnEntry e)
	{
		ResourceName prefab = F44_SectorSpawns.FindHqPrefabPublic(e.m_sFaction);
		if (prefab.IsEmpty())
			return;
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return;
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = PointPos(e.m_Vehicle);
		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
		if (!sp)
			return;
		sp.SetFactionKey(e.m_sFaction);
		sp.SetRespawnTime(m_Cfg.m_fPenaltySec);
		sp.SetSpawnPointEnabled_S(false);
		e.m_Point = sp;
		e.m_sState = string.Empty;
		UpdateName(e);
		PrintFormat("[F44] Mobile spawn created on %1 at %2", e.m_sKey, params.Transform[3]);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateName(F44_MobileSpawnEntry e)
	{
		if (!e.m_Point)
			return;
		string label = e.m_sKey;
		Vehicle v = Vehicle.Cast(e.m_Vehicle);
		if (v)
		{
			int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(v.GetPilot());
			if (pid > 0)
				label += " - " + GetGame().GetPlayerManager().GetPlayerName(pid);
		}
		e.m_Point.SetSpawnPointName(string.Format("%1%2 (%3)", NAME_PREFIX, label, e.m_iUses));
	}

	//------------------------------------------------------------------------------------------------
	protected void DeletePoint(F44_MobileSpawnEntry e, string why)
	{
		if (!e.m_Point)
			return;
		SCR_EntityHelper.DeleteEntityAndChildren(e.m_Point);
		e.m_Point = null;
		e.m_sState = string.Empty;
		PrintFormat("[F44] Mobile spawn removed on %1 (%2)", e.m_sKey, why);
	}
}
