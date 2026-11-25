// Arma & Heroes - BattleConfig (PLAN.md section 3).
// Parsed with the engine JsonLoadContext (files AND REST strings), walked manually.
// NOTE: FileIO.OpenFile/FileExists on addon paths froze Workbench, the engine loader does not.

//------------------------------------------------------------------------------------------------
class F44_VehicleEntry
{
	string m_sKey;
	int m_iCount;

	void FromJson(LoadContext n)
	{
		n.ReadValue("key", m_sKey);
		n.ReadValue("count", m_iCount);
	}
}

//------------------------------------------------------------------------------------------------
class F44_Team
{
	string m_sTeamId;
	string m_sDisplayName;	// roleplay unit name from the web ("Easy Company, 2nd Bn, 506th PIR")
	string m_sType;
	int m_iManpower;
	int m_iEquipLevel;
	ref array<string> m_aClasses = {};
	ref array<ref F44_VehicleEntry> m_aVehicles = {};

	void FromJson(LoadContext n)
	{
		n.ReadValue("teamId", m_sTeamId);
		n.ReadValue("displayName", m_sDisplayName);
		if (m_sDisplayName.IsEmpty())
			m_sDisplayName = m_sTeamId;
		n.ReadValue("type", m_sType);
		n.ReadValue("manpower", m_iManpower);
		m_iEquipLevel = 1;
		n.ReadValue("equipLevel", m_iEquipLevel);
		n.ReadValue("classes", m_aClasses);
		int count;
		if (n.StartArray("vehicles", count))
		{
			for (int j = 0; j < count; j++)
			{
				if (!n.StartObject())
					break;
				F44_VehicleEntry v = new F44_VehicleEntry();
				v.FromJson(n);
				m_aVehicles.Insert(v);
				n.EndObject();
			}
			n.EndArray();
		}
	}

	bool AllowsClass(string classKey)
	{
		return m_aClasses.Contains(classKey);
	}
}

//------------------------------------------------------------------------------------------------
class F44_Side
{
	string m_sSideKey;
	ref array<ref F44_Team> m_aTeams = {};
	// AI reinforcements (defaults when absent)
	bool m_bAIEnabled = true;
	int m_iAIMaxAlive = 24;
	float m_fAIPerPlayerScale = 1.0;
	string m_sAISkill = "Regular";	// base: Recruit | Regular | Veteran | Elite
	string m_sAISkillMin = "Recruit";
	string m_sAISkillMax = "Elite";

	void FromJson(LoadContext n)
	{
		// legacy flat keys
		n.ReadValue("aiEnabled", m_bAIEnabled);
		n.ReadValue("aiMaxAlive", m_iAIMaxAlive);
		n.ReadValue("aiPerPlayerScale", m_fAIPerPlayerScale);
		// web BattleConfig: "ai": {enabled, maxAlive, perPlayerScale, skill}
		if (n.StartObject("ai"))
		{
			n.ReadValue("enabled", m_bAIEnabled);
			n.ReadValue("maxAlive", m_iAIMaxAlive);
			n.ReadValue("perPlayerScale", m_fAIPerPlayerScale);
			string sk;
			if (n.ReadValue("skill", sk) && !sk.IsEmpty())
				m_sAISkill = sk;
			sk = "";
			if (n.ReadValue("minSkill", sk) && !sk.IsEmpty())
				m_sAISkillMin = sk;
			sk = "";
			if (n.ReadValue("maxSkill", sk) && !sk.IsEmpty())
				m_sAISkillMax = sk;
			n.EndObject();
		}
		int count;
		if (!n.StartArray("teams", count))
			return;
		for (int i = 0; i < count; i++)
		{
			if (!n.StartObject())
				break;
			F44_Team t = new F44_Team();
			t.FromJson(n);
			m_aTeams.Insert(t);
			n.EndObject();
		}
		n.EndArray();
	}

	//! aliveTarget = max(0, aiMaxAlive - playersOnSide * ratio)
	int GetAITarget(int playersOnSide)
	{
		if (!m_bAIEnabled)
			return 0;
		return Math.Max(0, Math.Round(m_iAIMaxAlive - playersOnSide * m_fAIPerPlayerScale));
	}

	int GetTotalManpower()
	{
		int total;
		foreach (F44_Team t : m_aTeams)
			total += t.m_iManpower;
		return total;
	}
}

//------------------------------------------------------------------------------------------------
//! Persistent per-player equipment decided by the website (keys of the catalog)
class F44_PlayerLoadout
{
	string m_sIdentityId;
	string m_sSoldierId;
	string m_sClass;
	int m_iPoints;
	string m_sRank;
	ref array<string> m_aItems = {};

	void FromJson(LoadContext n)
	{
		n.ReadValue("soldierId", m_sSoldierId);
		n.ReadValue("class", m_sClass);
		n.ReadValue("points", m_iPoints);
		n.ReadValue("rank", m_sRank);
		n.ReadValue("items", m_aItems);
	}

	static F44_PlayerLoadout FromString(string identityId, string json)
	{
		JsonLoadContext ctx = new JsonLoadContext();
		if (json.IsEmpty() || !ctx.LoadFromString(json))
			return null;
		F44_PlayerLoadout l = new F44_PlayerLoadout();
		l.m_sIdentityId = identityId;
		l.FromJson(ctx);
		if (l.m_sClass.IsEmpty() && l.m_aItems.IsEmpty())
			return null;
		return l;
	}
}

//------------------------------------------------------------------------------------------------
enum F44_EBattleMode
{
	ASSAULT,
	SKIRMISH
}

//------------------------------------------------------------------------------------------------
class F44_BattleConfig
{
	string m_sBattleId;
	int m_iWarId;
	string m_sCityId;
	string m_sMode = "ASSAULT";
	string m_sScenario;
	int m_iTimeLimitMin = 45;
	string m_sAttacker = "US";
	string m_sDefender = "GER";
	ref F44_Side m_Attacker;
	ref F44_Side m_Defender;
	int m_iAILimit = -1; // optional global AI budget from the site (-1 = use server attribute)
	ref map<string, ref F44_PlayerLoadout> m_mLoadouts = new map<string, ref F44_PlayerLoadout>();

	//------------------------------------------------------------------------------------------------
	bool FromJson(LoadContext root)
	{
		root.ReadValue("battleId", m_sBattleId);
		root.ReadValue("warId", m_iWarId);
		root.ReadValue("cityId", m_sCityId);
		root.ReadValue("mode", m_sMode);
		root.ReadValue("scenario", m_sScenario);
		root.ReadValue("timeLimitMin", m_iTimeLimitMin);
		root.ReadValue("attacker", m_sAttacker);
		root.ReadValue("defender", m_sDefender);
		root.ReadValue("aiLimit", m_iAILimit);

		m_Attacker = new F44_Side();
		m_Defender = new F44_Side();
		m_Attacker.m_sSideKey = m_sAttacker;
		m_Defender.m_sSideKey = m_sDefender;

		if (!root.StartObject("sides"))
			return false;
		if (root.StartObject(m_sAttacker))
		{
			m_Attacker.FromJson(root);
			root.EndObject();
		}
		if (root.StartObject(m_sDefender))
		{
			m_Defender.FromJson(root);
			root.EndObject();
		}
		root.EndObject();

		int count;
		if (root.StartMap("playerLoadouts", count))
		{
			string key;
			for (int i = 0; i < count; i++)
			{
				if (!root.ReadMapKey(i, key) || !root.StartObject(key))
					continue;
				F44_PlayerLoadout l = new F44_PlayerLoadout();
				l.m_sIdentityId = key;
				l.FromJson(root);
				m_mLoadouts.Set(key, l);
				root.EndObject();
			}
			root.EndMap();
		}
		// playerUnlocks: V3, ignored here
		return !m_Attacker.m_aTeams.IsEmpty() && !m_Defender.m_aTeams.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	F44_EBattleMode GetMode()
	{
		if (m_sMode == "SKIRMISH")
			return F44_EBattleMode.SKIRMISH;
		return F44_EBattleMode.ASSAULT;
	}

	//------------------------------------------------------------------------------------------------
	F44_Side GetSide(string sideKey)
	{
		if (sideKey == m_sAttacker)
			return m_Attacker;
		if (sideKey == m_sDefender)
			return m_Defender;
		return null;
	}

	//------------------------------------------------------------------------------------------------
	static F44_BattleConfig FromString(string json)
	{
		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromString(json))
			return null;
		return Build(ctx);
	}

	//------------------------------------------------------------------------------------------------
	static F44_BattleConfig FromFile(string path)
	{
		if (path.StartsWith("$profile:") && !FileIO.FileExists(path))
			return null;
		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(path))
			return null;
		return Build(ctx);
	}

	//------------------------------------------------------------------------------------------------
	protected static F44_BattleConfig Build(LoadContext root)
	{
		if (!root)
			return null;
		F44_BattleConfig cfg = new F44_BattleConfig();
		if (!cfg.FromJson(root))
		{
			Print("[F44] BattleConfig: invalid content (missing sides/teams)", LogLevel.ERROR);
			return null;
		}
		return cfg;
	}
}
