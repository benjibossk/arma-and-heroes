// Arma & Heroes - mission header carrying the battle parameters.
// Values can be overridden from the dedicated server config:
//   "gameProperties": { "missionHeader": { "m_sBattleId": "b_000123", "m_sApiUrl": "https://heroes.collectifxxl.fr", "m_sToken": "..." } }
class F44_MissionHeader : SCR_MissionHeader
{
	[Attribute("", desc: "Battle id (GET {apiUrl}/api/v1/battles/{battleId})")]
	string m_sBattleId;

	[Attribute("https://heroes.collectifxxl.fr", desc: "Arma & Heroes API base url")]
	string m_sApiUrl;

	[Attribute("", desc: "Bearer token of this server instance")]
	string m_sToken;

	[Attribute("0", desc: "Test only: overrides the battle timeLimitMin when > 0 (set by the orchestrator)")]
	int m_iTimeLimitOverrideMin;

	//------------------------------------------------------------------------------------------------
	static const string PROFILE_SERVER = "$profile:F44/server.json";
	protected bool m_bProfileApplied;

	static F44_MissionHeader Get()
	{
		F44_MissionHeader h = F44_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (h && !h.m_bProfileApplied)
		{
			h.m_bProfileApplied = true;
			h.ApplyProfileOverride();
		}
		return h;
	}

	//! Server started from the command line (-server/-addons, no -config: needed for unpublished
	//! local addons) cannot set gameProperties.missionHeader: the orchestrator writes
	//! $profile:F44/server.json { "battleId", "apiUrl", "token", "timeLimitOverrideMin" } instead.
	protected void ApplyProfileOverride()
	{
		if (!FileIO.FileExists(PROFILE_SERVER))
			return;
		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(PROFILE_SERVER))
		{
			Print("[F44] Header: cannot parse " + PROFILE_SERVER, LogLevel.ERROR);
			return;
		}
		string s;
		int n;
		if (ctx.ReadValue("battleId", s) && !s.IsEmpty())
			m_sBattleId = s;
		s = "";
		if (ctx.ReadValue("apiUrl", s) && !s.IsEmpty())
			m_sApiUrl = s;
		s = "";
		if (ctx.ReadValue("token", s) && !s.IsEmpty())
			m_sToken = s;
		if (ctx.ReadValue("timeLimitOverrideMin", n) && n > 0)
			m_iTimeLimitOverrideMin = n;
		Print("[F44] Header: overridden from " + PROFILE_SERVER + " (battle " + m_sBattleId + ", api " + m_sApiUrl + ")");
	}
}
