// Arma & Heroes - P2-F field maintenance load (GAP_PLAN.md), server side.
// Two per-player timers (equipment, vehicle), 0..maxSec, reset each battle:
//  - a death adds the load of the deployed soldier (class base + loadout points x perPointSec) to the equipment timer;
//  - losing a pool vehicle you were last driving adds cost x vehicleSecPerTicket to the vehicle timer;
//  - both timers count down while the player is alive in the battle;
//  - deploying is refused when "timer + load of that soldier" would exceed maxSec (one more death must fit);
//    driving a pool vehicle is refused (ejected) under the same rule for the vehicle timer.
// Battle JSON (optional): "maintenance": {"enabled": true, "maxSec": 240, "perPointSec": 1.5, "vehicleSecPerTicket": 6,
//   "baseSec": {"INF": 2, "PARA": 7, "RECON": 15, "CREW": 25, "PILOT": 25}}

//------------------------------------------------------------------------------------------------
class F44_MaintenanceConfig
{
	bool m_bEnabled = true;
	float m_fMaxSec = 240;
	float m_fPerPointSec = 1.5;
	float m_fVehicleSecPerTicket = 6;
	ref map<string, float> m_mBaseSec = new map<string, float>();

	void F44_MaintenanceConfig()
	{
		m_mBaseSec.Set("INF", 2);
		m_mBaseSec.Set("PARA", 7);
		m_mBaseSec.Set("RECON", 15);
		m_mBaseSec.Set("CREW", 25);
		m_mBaseSec.Set("PILOT", 25);
	}

	void Read(LoadContext root)
	{
		if (!root || !root.StartObject("maintenance"))
			return;
		root.ReadValue("enabled", m_bEnabled);
		root.ReadValue("maxSec", m_fMaxSec);
		root.ReadValue("perPointSec", m_fPerPointSec);
		root.ReadValue("vehicleSecPerTicket", m_fVehicleSecPerTicket);
		if (root.StartObject("baseSec"))
		{
			foreach (string k, float v : m_mBaseSec)
			{
				float nv = v;
				root.ReadValue(k, nv);
				m_mBaseSec.Set(k, nv);
			}
			root.EndObject();
		}
		root.EndObject();
	}
}

//------------------------------------------------------------------------------------------------
class F44_MaintState
{
	float m_fEquip;	// s
	float m_fVehicle;	// s
	float m_fDeployedLoad;	// load added on the next death
	bool m_bAlive;
}

//------------------------------------------------------------------------------------------------
class F44_Maintenance
{
	static ref F44_MaintenanceConfig s_Cfg = new F44_MaintenanceConfig();
	protected static ref map<int, ref F44_MaintState> s_mState = new map<int, ref F44_MaintState>();
	protected static ref map<IEntity, int> s_mLastDriver = new map<IEntity, int>();

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mState.Clear();
		s_mLastDriver.Clear();
	}

	//------------------------------------------------------------------------------------------------
	static F44_MaintState State(int pid)
	{
		F44_MaintState s;
		if (!s_mState.Find(pid, s))
		{
			s = new F44_MaintState();
			s_mState.Set(pid, s);
		}
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! class family of a catalog class key (US_PARA_RIFLEMAN -> PARA)
	static string ClassFamily(string classKey)
	{
		string k = classKey;
		k.ToUpper();
		if (k.Contains("PILOT"))
			return "PILOT";
		if (k.Contains("CREW") || k.Contains("TANK"))
			return "CREW";
		if (k.Contains("RECON") || k.Contains("SNIPER") || k.Contains("SCOUT"))
			return "RECON";
		if (k.Contains("PARA"))
			return "PARA";
		return "INF";
	}

	//------------------------------------------------------------------------------------------------
	static float LoadOf(string classKey, int points)
	{
		float base;
		s_Cfg.m_mBaseSec.Find(ClassFamily(classKey), base);
		return base + Math.Max(points, 0) * s_Cfg.m_fPerPointSec;
	}

	//------------------------------------------------------------------------------------------------
	//! deploy check; returns "" or the refusal reason
	static string CheckDeploy(int pid, string classKey, int points)
	{
		if (!s_Cfg.m_bEnabled)
			return string.Empty;
		F44_MaintState s = State(pid);
		float load = LoadOf(classKey, points);
		if (s.m_fEquip + load <= s_Cfg.m_fMaxSec)
			return string.Empty;
		int wait = Math.Ceil(s.m_fEquip + load - s_Cfg.m_fMaxSec);
		return string.Format("Field maintenance: this soldier is available in %1 s (load %2 s / %3 s). Pick a lighter loadout or wait.", wait, Math.Round(s.m_fEquip), Math.Round(s_Cfg.m_fMaxSec));
	}

	//------------------------------------------------------------------------------------------------
	static void OnDeployed(int pid, string classKey, int points)
	{
		F44_MaintState s = State(pid);
		s.m_fDeployedLoad = LoadOf(classKey, points);
		s.m_bAlive = true;
	}

	//------------------------------------------------------------------------------------------------
	static void OnKilled(int pid)
	{
		if (!s_Cfg.m_bEnabled)
			return;
		F44_MaintState s = State(pid);
		if (!s.m_bAlive)
			return;
		s.m_bAlive = false;
		s.m_fEquip = Math.Min(s_Cfg.m_fMaxSec, s.m_fEquip + s.m_fDeployedLoad);
		PrintFormat("[F44] MAINT player=%1 equip=%2s (+%3)", pid, Math.Round(s.m_fEquip), s.m_fDeployedLoad);
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(float dt)
	{
		foreach (int pid, F44_MaintState s : s_mState)
		{
			if (!s.m_bAlive)
				continue;
			s.m_fEquip = Math.Max(0, s.m_fEquip - dt);
			s.m_fVehicle = Math.Max(0, s.m_fVehicle - dt);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! player id in the pilot seat of a vehicle (0 = none)
	static int DriverOf(IEntity veh)
	{
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		if (!cm)
			return 0;
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		foreach (BaseCompartmentSlot sl : slots)
		{
			if (sl.GetType() != ECompartmentType.PILOT || !sl.GetOccupant())
				continue;
			return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(sl.GetOccupant());
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! every tick for each live pool vehicle: remembers the driver, ejects a driver over the vehicle cap
	static void WatchVehicle(IEntity veh, int cost)
	{
		int pid = DriverOf(veh);
		if (pid <= 0)
			return;
		if (s_Cfg.m_bEnabled)
		{
			F44_MaintState s = State(pid);
			float load = cost * s_Cfg.m_fVehicleSecPerTicket;
			int last;
			bool already = s_mLastDriver.Find(veh, last) && last == pid;
			if (!already && s.m_fVehicle + load > s_Cfg.m_fMaxSec)
			{
				F44_Rules.Eject(pid, string.Format("Vehicle maintenance: you can drive this vehicle in %1 s.", Math.Ceil(s.m_fVehicle + load - s_Cfg.m_fMaxSec)));
				return;
			}
		}
		s_mLastDriver.Set(veh, pid);
	}

	//------------------------------------------------------------------------------------------------
	static void OnVehicleDestroyed(IEntity veh, int cost)
	{
		int pid;
		if (!veh || !s_mLastDriver.Find(veh, pid))
			return;
		s_mLastDriver.Remove(veh);
		if (!s_Cfg.m_bEnabled)
			return;
		F44_MaintState s = State(pid);
		s.m_fVehicle = Math.Min(s_Cfg.m_fMaxSec, s.m_fVehicle + cost * s_Cfg.m_fVehicleSecPerTicket);
		PrintFormat("[F44] MAINT player=%1 vehicle=%2s", pid, Math.Round(s.m_fVehicle));
	}

	//------------------------------------------------------------------------------------------------
	static string PlayerJson(int pid)
	{
		F44_MaintState s;
		if (!s_mState.Find(pid, s))
			return string.Empty;
		return string.Format(",\"maintenance\":{\"equipSec\":%1,\"vehicleSec\":%2}", Math.Round(s.m_fEquip), Math.Round(s.m_fVehicle));
	}
}
