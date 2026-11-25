// Arma & Heroes - P3 small spawn rules (GAP_PLAN.md), server side.
//  - Vehicle temporary spawn lock: a destroyed pool vehicle locks the next spawn of the same key for its team (vehicleLockSec).
//  - Mine cap: at most maxMinesPerSide armed mines per faction; arming one more removes that side's oldest mine.
//  - Suicide penalty: a self-kill adds suicidePenaltySec to the field maintenance timer (F44_Maintenance.c).
//  - Join penalty: a player who leaves a running battle and comes back waits joinPenaltySec before deploying.
// Battle JSON (optional): "spawnRules": {"vehicleLockSec": 60, "maxMinesPerSide": 20, "suicidePenaltySec": 30, "joinPenaltySec": 60}

//------------------------------------------------------------------------------------------------
class F44_SpawnRulesConfig
{
	float m_fVehicleLockSec = 60;
	int m_iMaxMinesPerSide = 20;
	float m_fSuicidePenaltySec = 30;
	float m_fJoinPenaltySec = 60;

	void Read(LoadContext root)
	{
		if (!root || !root.StartObject("spawnRules"))
			return;
		root.ReadValue("vehicleLockSec", m_fVehicleLockSec);
		root.ReadValue("maxMinesPerSide", m_iMaxMinesPerSide);
		root.ReadValue("suicidePenaltySec", m_fSuicidePenaltySec);
		root.ReadValue("joinPenaltySec", m_fJoinPenaltySec);
		root.EndObject();
	}
}

//------------------------------------------------------------------------------------------------
class F44_SpawnRules
{
	static ref F44_SpawnRulesConfig s_Cfg = new F44_SpawnRulesConfig();
	protected static ref map<string, float> s_mVehLock = new map<string, float>();	// team|key -> unlock time (s)
	protected static ref map<string, float> s_mLeftAt = new map<string, float>();	// identity -> time the player left (s)
	protected static ref map<string, ref array<IEntity>> s_mMines = new map<string, ref array<IEntity>>();	// faction -> armed mines, oldest first
	protected static ref map<int, int> s_mSuicides = new map<int, int>();
	static bool s_bRunning;

	//------------------------------------------------------------------------------------------------
	static float Now()
	{
		return GetGame().GetWorld().GetWorldTime() * 0.001;
	}

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mVehLock.Clear();
		s_mLeftAt.Clear();
		s_mMines.Clear();
		s_mSuicides.Clear();
		s_bRunning = true;
	}

	//------------------------------------------------------------------------------------------------
	static void OnVehicleDestroyed(string teamId, string key)
	{
		if (s_Cfg.m_fVehicleLockSec <= 0)
			return;
		s_mVehLock.Set(teamId + "|" + key, Now() + s_Cfg.m_fVehicleLockSec);
		PrintFormat("[F44] VEHLOCK team=%1 key=%2 for %3 s", teamId, key, s_Cfg.m_fVehicleLockSec);
	}

	//------------------------------------------------------------------------------------------------
	static bool CanSpawnVehicle(string teamId, string key)
	{
		float until;
		return !s_mVehLock.Find(teamId + "|" + key, until) || Now() >= until;
	}

	//------------------------------------------------------------------------------------------------
	static void OnPlayerKilled(int victimId, int killerId)
	{
		if (killerId != victimId || victimId <= 0)
			return;
		int n;
		s_mSuicides.Find(victimId, n);
		s_mSuicides.Set(victimId, n + 1);
		F44_Maintenance.AddEquipSec(victimId, s_Cfg.m_fSuicidePenaltySec);
		PrintFormat("[F44] SUICIDE player=%1 +%2 s maintenance", victimId, s_Cfg.m_fSuicidePenaltySec);
	}

	//------------------------------------------------------------------------------------------------
	static void OnPlayerLeft(int pid)
	{
		string id = SCR_PlayerIdentityUtils.GetPlayerIdentityId(pid);
		if (!id.IsEmpty())
			s_mLeftAt.Set(id, Now());
	}

	//------------------------------------------------------------------------------------------------
	static string CheckDeploy(int pid)
	{
		float left;
		string id = SCR_PlayerIdentityUtils.GetPlayerIdentityId(pid);
		if (id.IsEmpty() || !s_mLeftAt.Find(id, left))
			return string.Empty;
		float wait = left + s_Cfg.m_fJoinPenaltySec - Now();
		if (wait <= 0)
			return string.Empty;
		return string.Format("You left this battle: you can deploy again in %1 s.", Math.Ceil(wait));
	}

	//------------------------------------------------------------------------------------------------
	//! a mine was armed by user (server)
	static void OnMineArmed(IEntity mine, IEntity user)
	{
		if (!s_bRunning || !mine || s_Cfg.m_iMaxMinesPerSide <= 0)
			return;
		string fk = "?";
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(user);
		if (ch)
			fk = ch.GetFactionKey();
		array<IEntity> list;
		if (!s_mMines.Find(fk, list))
		{
			list = {};
			s_mMines.Set(fk, list);
		}
		for (int i = list.Count() - 1; i >= 0; i--)
		{
			if (!list[i])
				list.Remove(i);
		}
		if (!list.Contains(mine))
			list.Insert(mine);
		while (list.Count() > s_Cfg.m_iMaxMinesPerSide)
		{
			IEntity old = list[0];
			list.Remove(0);
			if (old)
				SCR_EntityHelper.DeleteEntityAndChildren(old);
			PrintFormat("[F44] MINECAP %1: oldest mine removed (cap %2)", fk, s_Cfg.m_iMaxMinesPerSide);
		}
	}

	//------------------------------------------------------------------------------------------------
	static string PlayerJson(int pid)
	{
		int n;
		if (!s_mSuicides.Find(pid, n))
			return string.Empty;
		return ",\"suicides\":" + n.ToString();
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_ActivateMineUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		if (Replication.IsServer())
			F44_SpawnRules.OnMineArmed(pOwnerEntity, pUserEntity);
	}
}
