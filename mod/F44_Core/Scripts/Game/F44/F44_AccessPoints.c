// Arma & Heroes - P3 access points + multi-line layouts + "pushed off the map" (GAP_PLAN.md), server side.
// Battle JSON (optional, ASSAULT only):
//   "lines": [{"sectors": [0, 1, 2]}, {"sectors": [3, 4, 5]}]  - sector indices (assault order) of each attack line; each line is captured in
//                                        order on its own (F44_SectorManager.IsActive). Absent = one chain (vanilla F44).
//   "accessPoints": [{"id": "A1", "sector": 0, "teams": ["at_12"]}, ...]
//      - the access point sector starts ATTACKER-owned and stays contestable: defenders can take it back;
//      - while the defender owns it, the attacker teams listed on it cannot deploy ("pushed off the map");
//      - when every access point is defender-owned the battle ends: defender wins, reason PUSHED_OFF.
//   A team listed on no access point is never blocked.

//------------------------------------------------------------------------------------------------
class F44_AccessPoint
{
	string m_sId;
	int m_iSector = -1;
	ref array<string> m_aTeams = {};
	bool m_bLost;
}

//------------------------------------------------------------------------------------------------
class F44_AccessPoints
{
	static ref array<ref F44_AccessPoint> s_aPoints = {};
	static ref array<ref array<int>> s_aLines = {};

	//------------------------------------------------------------------------------------------------
	static void Read(LoadContext root)
	{
		s_aPoints.Clear();
		s_aLines.Clear();
		if (!root)
			return;
		int n;
		if (root.StartArray("lines", n))
		{
			for (int i = 0; i < n; i++)
			{
				if (!root.StartObject())
					break;
				array<int> line = {};
				root.ReadValue("sectors", line);
				root.EndObject();
				if (!line.IsEmpty())
					s_aLines.Insert(line);
			}
			root.EndArray();
		}
		if (root.StartArray("accessPoints", n))
		{
			for (int k = 0; k < n; k++)
			{
				if (!root.StartObject())
					break;
				F44_AccessPoint ap = new F44_AccessPoint();
				root.ReadValue("id", ap.m_sId);
				root.ReadValue("sector", ap.m_iSector);
				root.ReadValue("teams", ap.m_aTeams);
				root.EndObject();
				if (ap.m_iSector >= 0)
					s_aPoints.Insert(ap);
			}
			root.EndArray();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! after the game mode initialised the sector owners
	static void Init(F44_BattleConfig cfg, array<int> owners, array<int> progress)
	{
		F44_SectorManager.s_aLines.Clear();
		F44_SectorManager.s_aAlwaysActive.Clear();
		if (cfg.GetMode() != F44_EBattleMode.ASSAULT)
			return;
		foreach (array<int> line : s_aLines)
			F44_SectorManager.s_aLines.Insert(line);
		foreach (F44_AccessPoint ap : s_aPoints)
		{
			ap.m_bLost = false;
			if (ap.m_iSector >= owners.Count())
				continue;
			owners[ap.m_iSector] = F44_ESectorOwner.ATTACKER;
			progress[ap.m_iSector] = 100;
			F44_SectorManager.s_aAlwaysActive.Insert(ap.m_iSector);
			PrintFormat("[F44] ACCESS point %1 = sector %2, teams %3", ap.m_sId, ap.m_iSector, ap.m_aTeams);
		}
		PrintFormat("[F44] ACCESS %1 lines, %2 access points", s_aLines.Count(), s_aPoints.Count());
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(array<int> owners)
	{
		foreach (F44_AccessPoint ap : s_aPoints)
		{
			if (ap.m_iSector >= owners.Count())
				continue;
			bool lost = owners[ap.m_iSector] == F44_ESectorOwner.DEFENDER;
			if (lost != ap.m_bLost)
			{
				ap.m_bLost = lost;
				PrintFormat("[F44] ACCESS point %1 %2", ap.m_sId, lost);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "" = team may deploy
	static string CheckDeploy(string teamId)
	{
		bool listed;
		foreach (F44_AccessPoint ap : s_aPoints)
		{
			if (!ap.m_aTeams.Contains(teamId))
				continue;
			listed = true;
			if (!ap.m_bLost)
				return string.Empty;
		}
		if (!listed)
			return string.Empty;
		return "Your assault team was pushed off the map: its access point is held by the enemy. Retake it with another team.";
	}

	//------------------------------------------------------------------------------------------------
	//! true when every access point is defender-owned
	static bool AllLost()
	{
		if (s_aPoints.IsEmpty())
			return false;
		foreach (F44_AccessPoint ap : s_aPoints)
		{
			if (!ap.m_bLost)
				return false;
		}
		return true;
	}
}
