// Arma & Heroes - mobile spawn INSIDE team vehicles (SERVER authoritative). Mod 1.2.1.
// Design: docs/SPAWN_UI.md "Mobile spawn" (H&G wiki "Action Rules"): the player spawns directly in a free
// seat of an active team vehicle (half-track / truck, or any tank of an Armor/TD team). No driver, no
// stationary and no enemy-distance rule. Disabled when the vehicle is destroyed, full or out of uses;
// "lost" for good when an enemy entered it, or a member of another assault team entered it while empty.
// The SCR_SpawnPoint only carries the menu entry; PrepareSpawnedEntity_S seats the character in the
// vehicle (F44_VehicleSeats, also used by the C-47 cargo seats of F44_ParaDrop.c).
// Fed by F44_GameModeComponent.ServerTick (Tick), PreparePlayerEntity_S (OnSpawnAt, SetPlayerTeam) and the
// pool refill (CanSpawnForTeam = max active vehicles per team).

//------------------------------------------------------------------------------------------------
//! Seat helper shared by mobile spawns and the air drop plane
class F44_VehicleSeats
{
	//! free seats (not occupied / reserved); cargoOnly = CARGO compartments only
	static int Free(IEntity veh, bool cargoOnly)
	{
		if (!veh)
			return 0;
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		if (!cm)
			return 0;
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		int n;
		foreach (BaseCompartmentSlot s : slots)
		{
			if (cargoOnly && s.GetType() != ECompartmentType.CARGO)
				continue;
			if (!s.IsOccupied() && !s.IsReserved())
				n++;
		}
		return n;
	}

	//! Teleport a freshly spawned character into a free seat (cargo first, then the others unless cargoOnly)
	static bool Seat(IEntity veh, IEntity character, bool cargoOnly, string tag)
	{
		if (!veh || !character)
			return false;
		SCR_CompartmentAccessComponent ca = SCR_CompartmentAccessComponent.Cast(character.FindComponent(SCR_CompartmentAccessComponent));
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		if (!ca || !cm)
			return false;
		character.SetOrigin(veh.GetOrigin());
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		ChimeraWorld world = GetGame().GetWorld();
		for (int pass = 0; pass < 2; pass++)
		{
			if (pass == 1 && cargoOnly)
				break;
			foreach (BaseCompartmentSlot s : slots)
			{
				bool cargo = s.GetType() == ECompartmentType.CARGO;
				if ((pass == 0) != cargo)
					continue;
				if (s.IsOccupied() || (s.IsReserved() && !s.IsReservedBy(character)))
					continue;
				bool ok = ca.GetInVehicle(veh, s, true, -1, ECloseDoorAfterActions.INVALID, world.IsGameTimePaused());
				PrintFormat("[F44] %1: seat %2 (%3) -> %4", tag, slots.Find(s), typename.EnumToString(ECompartmentType, s.GetType()), ok);
				return ok;
			}
		}
		PrintFormat("[F44] %1: vehicle full", tag, level: LogLevel.WARNING);
		return false;
	}
}

//------------------------------------------------------------------------------------------------
class F44_MobileSpawnConfig
{
	bool m_bEnabled = true;
	int m_iUsesHalftrack = 8;	// spawns per vehicle life
	int m_iUsesTruck = 3;
	int m_iUsesTank = 99;	// tanks of Armor/TD teams (their only spawn)
	float m_fCooldownSec = 0;	// between two spawns on the same vehicle (reference: none)
	float m_fPenaltySec = 10;	// SCR_SpawnPoint respawn time; H&G mobile spawn 20 s total = base 10 + 10
	int m_iMaxActiveMech = 1;	// active half-tracks per Mech-inf team (0 = no cap)
	int m_iMaxActiveArmor = 3;	// active tanks per Armor/TD team (0 = no cap)
	float m_fMoveRefresh = 30;	// m: the menu point is re-created at the vehicle when it moved this far
	ref array<string> m_aHalftrackKeys = {"GER_SDKFZ251*", "US_M3A1*"};	// "*" suffix = prefix match
	ref array<string> m_aTruckKeys = {"GER_TRUCK", "GER_TRUCK_OPEN", "US_TRUCK*"};
	ref array<string> m_aMechTypes = {"MECH", "MOT"};	// web team types capped by maxActiveMech
	ref array<string> m_aArmorTypes = {"ARM", "TD"};	// web team types: every vehicle is a mobile spawn

	//------------------------------------------------------------------------------------------------
	static F44_MobileSpawnConfig Read(LoadContext root)
	{
		F44_MobileSpawnConfig c = new F44_MobileSpawnConfig();
		if (!root || !root.StartObject("mobileSpawn"))
			return c;
		root.ReadValue("enabled", c.m_bEnabled);
		root.ReadValue("usesHalftrack", c.m_iUsesHalftrack);
		root.ReadValue("usesTruck", c.m_iUsesTruck);
		root.ReadValue("usesTank", c.m_iUsesTank);
		root.ReadValue("cooldownSec", c.m_fCooldownSec);
		root.ReadValue("penaltySec", c.m_fPenaltySec);
		root.ReadValue("maxActiveMech", c.m_iMaxActiveMech);
		root.ReadValue("maxActiveArmor", c.m_iMaxActiveArmor);
		root.ReadValue("moveRefresh", c.m_fMoveRefresh);
		// 1.2.0 keys maxSpeedKmh / stopSec / enemyRadius / driverAbsentSec / backOffset / requireDriver: obsolete, ignored
		array<string> ht = {};
		if (root.ReadValue("halftrackKeys", ht) && !ht.IsEmpty())
			c.m_aHalftrackKeys = ht;
		array<string> tr = {};
		if (root.ReadValue("truckKeys", tr) && !tr.IsEmpty())
			c.m_aTruckKeys = tr;
		array<string> mt = {};
		if (root.ReadValue("mechTypes", mt) && !mt.IsEmpty())
			c.m_aMechTypes = mt;
		array<string> at = {};
		if (root.ReadValue("armorTypes", at) && !at.IsEmpty())
			c.m_aArmorTypes = at;
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

	bool IsArmor(string teamType)
	{
		return !teamType.IsEmpty() && m_aArmorTypes.Contains(teamType);
	}

	bool IsMech(string teamType)
	{
		return !teamType.IsEmpty() && m_aMechTypes.Contains(teamType);
	}

	//! uses per vehicle life for this vehicle key / team type, 0 = not a mobile spawn vehicle
	int UsesFor(string key, string teamType)
	{
		if (MatchKey(key, m_aHalftrackKeys))
			return m_iUsesHalftrack;
		if (MatchKey(key, m_aTruckKeys))
			return m_iUsesTruck;
		if (IsArmor(teamType))
			return m_iUsesTank;
		return 0;
	}
}

//------------------------------------------------------------------------------------------------
class F44_MobileSpawnEntry
{
	IEntity m_Vehicle;
	string m_sKey;
	string m_sFaction;
	string m_sTeamId;
	SCR_SpawnPoint m_Point;
	int m_iUses;
	float m_fNextUse;	// world time s
	bool m_bWasEmpty = true;
	string m_sState;	// last logged state
}

//------------------------------------------------------------------------------------------------
class F44_MobileSpawns
{
	static const string NAME_PREFIX = "MOBILE ";
	protected static const float STEP = 1;

	protected ref F44_MobileSpawnConfig m_Cfg = new F44_MobileSpawnConfig();
	protected ref array<ref F44_MobileSpawnEntry> m_aEntries = {};
	protected ref array<IEntity> m_aLost = {};	// vehicles lost for mobile spawn (never registered again)
	protected ref map<int, string> m_mPlayerTeam = new map<int, string>();
	protected float m_fAccu;

	//------------------------------------------------------------------------------------------------
	void SetConfig(F44_MobileSpawnConfig cfg)
	{
		if (cfg)
			m_Cfg = cfg;
	}

	//------------------------------------------------------------------------------------------------
	void SetPlayerTeam(int playerId, string teamId)
	{
		m_mPlayerTeam.Set(playerId, teamId);
	}

	//------------------------------------------------------------------------------------------------
	//! Pool refill gate: Mech teams keep at most maxActiveMech half-tracks, Armor/TD teams maxActiveArmor
	//! vehicles. When the team has none left on the map the pool spawns a new one at once, and the player
	//! spawns with it (in it) through its MOBILE point = "spawn with vehicle".
	bool CanSpawnForTeam(string teamId, string teamType, string key)
	{
		int cap;
		bool armor = m_Cfg.IsArmor(teamType);
		if (armor)
			cap = m_Cfg.m_iMaxActiveArmor;
		else if (m_Cfg.IsMech(teamType) && F44_MobileSpawnConfig.MatchKey(key, m_Cfg.m_aHalftrackKeys))
			cap = m_Cfg.m_iMaxActiveMech;
		if (cap <= 0 || !m_Cfg.m_bEnabled)
			return true;
		int n;
		foreach (F44_MobileSpawnEntry e : m_aEntries)
		{
			if (e.m_sTeamId != teamId || !e.m_Vehicle)
				continue;
			if (armor || F44_MobileSpawnConfig.MatchKey(e.m_sKey, m_Cfg.m_aHalftrackKeys))
				n++;
		}
		return n < cap;
	}

	//------------------------------------------------------------------------------------------------
	//! API: register a battle vehicle (ignored if it is not a mobile spawn vehicle)
	void Register(IEntity veh, string key, string factionKey, string teamId = "", string teamType = "")
	{
		if (!m_Cfg.m_bEnabled || !veh || factionKey.IsEmpty() || m_aLost.Contains(veh))
			return;
		int uses = m_Cfg.UsesFor(key, teamType);
		if (uses <= 0)
			return;
		if (IsDestroyed(veh))
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
		e.m_sTeamId = teamId;
		e.m_iUses = uses;
		m_aEntries.Insert(e);
		PrintFormat("[F44] Mobile spawn registered on %1 (%2, team %3 %4, %5 uses)", key, factionKey, teamId, teamType, uses);
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
			if (m_Cfg.m_fCooldownSec > 0 && sp.IsSpawnPointEnabled())
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

	protected static bool IsDestroyed(IEntity veh)
	{
		DamageManagerComponent dmg = DamageManagerComponent.Cast(veh.FindComponent(DamageManagerComponent));
		return dmg && dmg.GetState() == EDamageState.DESTROYED;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> owners, array<int> progress)
	{
		m_fAccu += dt;
		if (m_fAccu < STEP)
			return;
		m_fAccu = 0;

		for (int i = m_aEntries.Count() - 1; i >= 0; i--)
		{
			F44_MobileSpawnEntry e = m_aEntries[i];
			string reason = Evaluate(e);
			if (reason.StartsWith("REMOVE"))
			{
				DeletePoint(e, reason.Substring(6, reason.Length() - 6));
				if (e.m_Vehicle)
					m_aLost.Insert(e.m_Vehicle);
				m_aEntries.Remove(i);
				continue;
			}
			if (!e.m_Point)
				CreatePoint(e);
			else if (vector.DistanceSq(e.m_Point.GetOrigin(), e.m_Vehicle.GetOrigin()) > m_Cfg.m_fMoveRefresh * m_Cfg.m_fMoveRefresh)
			{
				DeletePoint(e, "vehicle moved");
				CreatePoint(e);
			}
			bool want = reason.IsEmpty();
			if (e.m_Point && e.m_Point.IsSpawnPointEnabled() != want)
				e.m_Point.SetSpawnPointEnabled_S(want);
			string st = "ENABLED";
			if (!want)
				st = "DISABLED (" + reason + ")";
			if (e.m_Point && st != e.m_sState)
			{
				e.m_sState = st;
				PrintFormat("[F44] Mobile spawn %1 %2", e.m_sKey, st);
				UpdateName(e);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "" = enabled; "REMOVE<why>" = delete for good; anything else = disable reason
	protected string Evaluate(F44_MobileSpawnEntry e)
	{
		IEntity veh = e.m_Vehicle;
		if (!veh)
			return "REMOVE vehicle deleted";
		if (IsDestroyed(veh))
			return "REMOVE vehicle destroyed";

		// occupants: enemy inside, or another assault team took the empty vehicle -> lost
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		bool empty = true;
		if (cm)
		{
			array<BaseCompartmentSlot> slots = {};
			cm.GetCompartments(slots);
			foreach (BaseCompartmentSlot s : slots)
			{
				SCR_ChimeraCharacter occ = SCR_ChimeraCharacter.Cast(s.GetOccupant());
				if (!occ)
					continue;
				CharacterControllerComponent cc = occ.GetCharacterController();
				if (cc && cc.IsDead())
					continue;
				empty = false;
				string fk = occ.GetFactionKey();
				if (!fk.IsEmpty() && fk != e.m_sFaction)
					return "REMOVE lost: enemy entered";
				if (e.m_bWasEmpty && !e.m_sTeamId.IsEmpty())
				{
					int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(occ);
					string team;
					if (pid > 0 && m_mPlayerTeam.Find(pid, team) && !team.IsEmpty() && team != e.m_sTeamId)
						return "REMOVE lost: taken empty by team " + team;
				}
			}
		}
		e.m_bWasEmpty = empty;

		if (e.m_iUses <= 0)
			return "no uses left";
		if (Now() < e.m_fNextUse)
			return "cooldown";
		if (F44_VehicleSeats.Free(veh, false) <= 0)
			return "full";
		return string.Empty;
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
		params.Transform[3] = e.m_Vehicle.GetOrigin();
		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
		if (!sp)
			return;
		sp.SetFactionKey(e.m_sFaction);
		sp.SetRespawnTime(m_Cfg.m_fPenaltySec);
		sp.SetSpawnPointEnabled_S(false);
		sp.F44_SetMobileVehicle(e.m_Vehicle);
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
		{
			PrintFormat("[F44] Mobile spawn %1 dropped (%2)", e.m_sKey, why);
			return;
		}
		SCR_EntityHelper.DeleteEntityAndChildren(e.m_Point);
		e.m_Point = null;
		e.m_sState = string.Empty;
		PrintFormat("[F44] Mobile spawn removed on %1 (%2)", e.m_sKey, why);
	}
}

//------------------------------------------------------------------------------------------------
//! Mobile spawn point behaviour: the spawned character is seated in the vehicle (server side)
modded class SCR_SpawnPoint
{
	protected IEntity m_F44MobileVeh;

	void F44_SetMobileVehicle(IEntity veh)
	{
		m_F44MobileVeh = veh;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanReserveFor_S(playerId, result))
			return false;
		if (!m_F44MobileVeh)
			return true;
		if (F44_VehicleSeats.Free(m_F44MobileVeh, false) <= 0)
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_VEHICLE_FULL;
			return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool PrepareSpawnedEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!m_F44MobileVeh)
			return super.PrepareSpawnedEntity_S(requestComponent, data, entity);
		if (!IsSpawnPointEnabled() || !entity)
			return false;
		return F44_VehicleSeats.Seat(m_F44MobileVeh, entity, false, "Mobile spawn");
	}

	//------------------------------------------------------------------------------------------------
	override bool CanFinalizeSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!super.CanFinalizeSpawn_S(requestComponent, data, entity))
			return false;
		if (!m_F44MobileVeh)
			return true;
		SCR_CompartmentAccessComponent ca = SCR_CompartmentAccessComponent.Cast(entity.FindComponent(SCR_CompartmentAccessComponent));
		return !ca || !ca.IsGettingIn();
	}
}
