// Arma & Heroes - P3 air layer in battle (GAP_PLAN.md), server side: airfields, air teams, fighter spawn, plane tickets.
// Battle JSON (optional): "air": {"enabled": true, "respawnSec": 60, "maxActivePerTeam": 1,
//   "airfields": [{"side": "GER", "x": 1200, "z": 3400, "yaw": 90}]}
//  - Air team = a team whose vehicles have catalog type "AIR" (web team type FTR / AIR). Its planes never use the ground
//    vehicle slots: they spawn parked on an airfield of their side (one plane of the team active at a time by default).
//  - Plane tickets = the pool count sent by the web (one sortie per plane); a destroyed plane removes its catalog cost
//    in battle tickets and is reported in losses like any pool vehicle (F44_GameModeComponent.UpdateVehicles).
//  - Pilot seat reserved to the PILOT family (F44_SeatRules.c).
//  - Aircraft: GER Ju 87 Stuka (GER_STUKA), Ju 88 (GER_JU88) from RMS WW2 WarBirds II. US: only the C-47 (US_C47) exists,
//    used by the air drop (F44_ParaDrop.c); there is NO US fighter yet (gap: RMS WarBirds II dropped its P-51).
//  - No airfield for a side = its air pool stays unused (warning logged once).

//------------------------------------------------------------------------------------------------
class F44_Airfield
{
	string m_sSide;
	vector m_vPos;
	float m_fYaw;
	float m_fFreeAt;	// world time (s) the airfield may spawn again
}

//------------------------------------------------------------------------------------------------
class F44_AirLayer
{
	static bool s_bEnabled = true;
	static float s_fRespawnSec = 60;
	static int s_iMaxActivePerTeam = 1;
	static ref array<ref F44_Airfield> s_aAirfields = {};
	protected static ref map<string, int> s_mActive = new map<string, int>();	// team -> active planes (recount each tick)
	protected static ref set<string> s_Warned = new set<string>();

	//------------------------------------------------------------------------------------------------
	static void Read(LoadContext root)
	{
		s_aAirfields.Clear();
		s_bEnabled = true;
		if (!root || !root.StartObject("air"))
			return;
		root.ReadValue("enabled", s_bEnabled);
		root.ReadValue("respawnSec", s_fRespawnSec);
		root.ReadValue("maxActivePerTeam", s_iMaxActivePerTeam);
		int n;
		if (root.StartArray("airfields", n))
		{
			for (int i = 0; i < n; i++)
			{
				if (!root.StartObject())
					break;
				F44_Airfield a = new F44_Airfield();
				float x, z;
				root.ReadValue("side", a.m_sSide);
				root.ReadValue("x", x);
				root.ReadValue("z", z);
				root.ReadValue("yaw", a.m_fYaw);
				root.EndObject();
				a.m_vPos = Vector(x, 0, z);
				s_aAirfields.Insert(a);
			}
			root.EndArray();
		}
		root.EndObject();
	}

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mActive.Clear();
		s_Warned.Clear();
		foreach (F44_Airfield a : s_aAirfields)
		{
			a.m_fFreeAt = 0;
			a.m_vPos[1] = GetGame().GetWorld().GetSurfaceY(a.m_vPos[0], a.m_vPos[2]);
		}
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAirKey(string key)
	{
		return F44_Catalog.Get().GetVehicleType(key) == "AIR";
	}

	//------------------------------------------------------------------------------------------------
	//! called by the game mode with the active air vehicles of a team, before TrySpawn
	static void CountActive(string teamId)
	{
		int n;
		s_mActive.Find(teamId, n);
		s_mActive.Set(teamId, n + 1);
	}

	static void ClearActive()
	{
		s_mActive.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! spawn one parked plane of the pool entry if the team / airfield allow it; returns the plane or null
	static IEntity TrySpawn(F44_PoolEntry e, string sideKey, float now)
	{
		if (!s_bEnabled || e.m_iRemaining <= 0)
			return null;
		int active;
		s_mActive.Find(e.m_sTeamId, active);
		if (active >= s_iMaxActivePerTeam)
			return null;
		F44_Airfield field;
		bool any;
		foreach (F44_Airfield a : s_aAirfields)
		{
			if (a.m_sSide != sideKey)
				continue;
			any = true;
			if (now >= a.m_fFreeAt)
			{
				field = a;
				break;
			}
		}
		if (!any)
		{
			if (!s_Warned.Contains(sideKey))
			{
				s_Warned.Insert(sideKey);
				PrintFormat("[F44] AIR no airfield for side %1: air pool unused", sideKey, level: LogLevel.WARNING);
			}
			return null;
		}
		if (!field)
			return null;
		ResourceName prefab = e.m_sPrefab;
		if (prefab.IsEmpty())
			prefab = F44_Catalog.Get().GetVehiclePrefab(e.m_sKey);
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			if (!s_Warned.Contains(e.m_sKey))
			{
				s_Warned.Insert(e.m_sKey);
				PrintFormat("[F44] AIR prefab of %1 not loaded (WarBirds / C-47 mod missing?)", e.m_sKey, level: LogLevel.WARNING);
			}
			return null;
		}
		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(Vector(field.m_fYaw, 0, 0), p.Transform);
		p.Transform[3] = field.m_vPos + Vector(0, 0.5, 0);
		IEntity plane = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		if (!plane)
			return null;
		e.m_iRemaining--;
		field.m_fFreeAt = now + s_fRespawnSec;
		s_mActive.Set(e.m_sTeamId, active + 1);
		PrintFormat("[F44] AIRSPAWN side=%1 team=%2 key=%3 pos=%4 left=%5", sideKey, e.m_sTeamId, e.m_sKey, field.m_vPos, e.m_iRemaining);
		return plane;
	}
}
