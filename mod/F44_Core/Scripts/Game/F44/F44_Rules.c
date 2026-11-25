// Arma & Heroes - GAP_PLAN battle rules (server side), kept out of F44_GameModeComponent.c.
// The game mode only calls the static hooks below; each rule package lives in its own section/file.
//  - P2-E per-kill war funds: kills counted per killer team and victim type -> BattleResult "teamKills";
//    the web (web/killfunds.go) converts them into faction war funds for the killer team's commander.

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
	static void Init(F44_BattleConfig cfg)
	{
		s_mTeamKills.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! any player death (killerId <= 0 = bot / world / unknown)
	static void OnPlayerKilled(int victimId, string victimTeam, int killerId)
	{
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
