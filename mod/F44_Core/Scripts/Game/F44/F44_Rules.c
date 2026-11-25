// Arma & Heroes - GAP_PLAN battle rules (server side), kept out of F44_GameModeComponent.c.
// The game mode only calls the static hooks below; each rule package lives in its own section/file.
//  - P2-E per-kill war funds: kills counted per killer team and victim type -> BattleResult "teamKills";
//    the web (web/killfunds.go) converts them into faction war funds for the killer team's commander.
//  - P2-F field maintenance load: F44_Maintenance.c
//  - P2-G combat badges: F44_Badges.c

//------------------------------------------------------------------------------------------------
class F44_VictimKills
{
	int m_iPlayers;	// enemy players killed
	int m_iBots;	// enemy bots killed
	ref map<string, int> m_mVehicles = new map<string, int>();	// destroyed enemy pool vehicles, by catalog key
}

//------------------------------------------------------------------------------------------------
class F44_Rules
{
	// killer team id (or "_side_<SIDE>" when only the side is known) -> kills by victim type
	protected static ref map<string, ref F44_VictimKills> s_mTeamKills = new map<string, ref F44_VictimKills>();

	//------------------------------------------------------------------------------------------------
	//! battle JSON keys of the rule packages (called from F44_BattleConfig.FromJson)
	static void ReadConfig(LoadContext root)
	{
		F44_Maintenance.s_Cfg = new F44_MaintenanceConfig();
		F44_Maintenance.s_Cfg.Read(root);
	}

	//------------------------------------------------------------------------------------------------
	static void Init(F44_BattleConfig cfg)
	{
		s_mTeamKills.Clear();
		F44_Maintenance.Reset();
		F44_Badges.Reset();
	}

	//------------------------------------------------------------------------------------------------
	//! deploy gate (PreparePlayerEntity_S, class and team already validated). false = refused, player notified.
	static bool CanDeploy(int pid, string classKey, string teamId, int points)
	{
		string why = F44_Maintenance.CheckDeploy(pid, classKey, points);
		if (!why.IsEmpty())
		{
			Notify(pid, why);
			PrintFormat("[F44] Spawn refused: player %1 %2", pid, why, level: LogLevel.WARNING);
			return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void OnDeployed(int pid, string classKey, string teamId, int points, IEntity character, F44_PlayerLoadout lo)
	{
		F44_Maintenance.OnDeployed(pid, classKey, points);
		F44_Badges.OnDeployed(pid, character, lo);
	}

	//------------------------------------------------------------------------------------------------
	//! any player death (killerId <= 0 = bot / world / unknown)
	static void OnPlayerKilled(int victimId, string victimTeam, int killerId)
	{
		F44_Maintenance.OnKilled(victimId);
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(float dt)
	{
		F44_Maintenance.Tick(dt);
	}

	//------------------------------------------------------------------------------------------------
	//! each tick for every live pool vehicle
	static void WatchVehicle(IEntity veh, string key, int side, string teamId)
	{
		F44_Maintenance.WatchVehicle(veh, F44_Catalog.Get().GetVehicleCost(key));
	}

	//------------------------------------------------------------------------------------------------
	static void OnVehicleDestroyed(IEntity veh, string key)
	{
		F44_Maintenance.OnVehicleDestroyed(veh, F44_Catalog.Get().GetVehicleCost(key));
	}

	//------------------------------------------------------------------------------------------------
	//! fragment appended to each BattleResult player object
	static string PlayerJson(int pid)
	{
		return F44_Maintenance.PlayerJson(pid);
	}

	//------------------------------------------------------------------------------------------------
	//! on-screen hint for one player (server side)
	static void Notify(int pid, string text)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(pid));
		if (pc)
			pc.F44_SendRuleHint(text);
	}

	//------------------------------------------------------------------------------------------------
	//! force a player out of his vehicle seat
	static void Eject(int pid, string text)
	{
		IEntity ch = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
		if (!ch)
			return;
		CompartmentAccessComponent ca = CompartmentAccessComponent.Cast(ch.FindComponent(CompartmentAccessComponent));
		if (ca && ca.IsInCompartment())
		{
			ca.GetOutVehicle(EGetOutType.ANIMATED, -1, ECloseDoorAfterActions.INVALID, false);
			Notify(pid, text);
			PrintFormat("[F44] EJECT player=%1 %2", pid, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! P2-E: victimType = "player" | "bot" | "veh" (vehKey = catalog key)
	static void OnKill(string killerTeam, string victimType, string vehKey = "")
	{
		if (killerTeam.IsEmpty())
			return;
		F44_VictimKills k;
		if (!s_mTeamKills.Find(killerTeam, k))
		{
			k = new F44_VictimKills();
			s_mTeamKills.Set(killerTeam, k);
		}
		if (victimType == "player")
			k.m_iPlayers++;
		else if (victimType == "bot")
			k.m_iBots++;
		else if (victimType == "veh" && !vehKey.IsEmpty())
		{
			int n;
			k.m_mVehicles.Find(vehKey, n);
			k.m_mVehicles.Set(vehKey, n + 1);
		}
		PrintFormat("[F44] KILLFUND team=%1 victim=%2 %3", killerTeam, victimType, vehKey);
	}

	//------------------------------------------------------------------------------------------------
	//! fragment appended to the BattleResult root object (starts with ',')
	static string ResultJson()
	{
		string s = ",\"teamKills\":{";
		bool first = true;
		foreach (string team, F44_VictimKills k : s_mTeamKills)
		{
			if (!first)
				s += ",";
			first = false;
			s += string.Format("\"%1\":{\"players\":%2,\"bots\":%3,\"vehicles\":{", team, k.m_iPlayers, k.m_iBots);
			bool firstV = true;
			foreach (string vk, int vn : k.m_mVehicles)
			{
				if (!firstV)
					s += ",";
				firstV = false;
				s += string.Format("\"%1\":%2", vk, vn);
			}
			s += "}}";
		}
		return s + "}";
	}
}

//------------------------------------------------------------------------------------------------
//! owner-only hint RPC for the rule messages (deploy refused, ejected...)
modded class SCR_PlayerController
{
	void F44_SendRuleHint(string text)
	{
		Rpc(RpcDo_F44RuleHint, text);
		if (GetPlayerId() == SCR_PlayerController.GetLocalPlayerId())
			RpcDo_F44RuleHint(text);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_F44RuleHint(string text)
	{
		if (System.IsConsoleApp())
			return;
		SCR_HintManagerComponent.ShowCustomHint(text, "Arma & Heroes", 8);
	}
}
