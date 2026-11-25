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

	//------------------------------------------------------------------------------------------------
	static F44_MissionHeader Get()
	{
		return F44_MissionHeader.Cast(GetGame().GetMissionHeader());
	}
}
