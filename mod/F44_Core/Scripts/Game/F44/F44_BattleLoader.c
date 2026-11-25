// Arma & Heroes - loads the BattleConfig (REST API, then local file fallbacks)
// Order: GET {apiUrl}/api/v1/battles/{battleId} -> $profile:F44/battle.json -> Configs/F44/battle_sample.json
class F44_BattleLoader
{
	static const string PROFILE_BATTLE = "$profile:F44/battle.json";
	static const string MOD_SAMPLE = "Configs/F44/battle_sample.json";

	protected ref RestCallback m_Callback;
	protected ref ScriptInvoker m_OnLoaded = new ScriptInvoker(); // (F44_BattleConfig cfg)
	protected string m_sApiUrl;
	protected string m_sToken;
	protected string m_sBattleId;

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnLoaded()
	{
		return m_OnLoaded;
	}

	//------------------------------------------------------------------------------------------------
	string GetApiUrl()
	{
		return m_sApiUrl;
	}

	string GetToken()
	{
		return m_sToken;
	}

	string GetBattleId()
	{
		return m_sBattleId;
	}

	//------------------------------------------------------------------------------------------------
	void Load()
	{
		F44_MissionHeader header = F44_MissionHeader.Get();
		if (header)
		{
			m_sApiUrl = header.m_sApiUrl;
			m_sToken = header.m_sToken;
			m_sBattleId = header.m_sBattleId;
		}

		if (!m_sApiUrl.IsEmpty() && !m_sBattleId.IsEmpty())
		{
			RestContext ctx = GetRestContext(m_sApiUrl, m_sToken);
			if (ctx)
			{
				m_Callback = new RestCallback();
				m_Callback.SetOnSuccess(OnRestSuccess);
				m_Callback.SetOnError(OnRestError);
				Print("[F44] Loader: GET battle " + m_sBattleId + " from " + m_sApiUrl);
				ctx.GET(m_Callback, "/api/v1/battles/" + m_sBattleId);
				return;
			}
		}

		LoadFromFiles();
	}

	//------------------------------------------------------------------------------------------------
	static RestContext GetRestContext(string apiUrl, string token)
	{
		RestApi api = GetGame().GetRestApi();
		if (!api)
			return null;
		RestContext ctx = api.GetContext(apiUrl);
		if (!ctx)
			return null;
		string headers = "Content-Type,application/json";
		if (!token.IsEmpty())
			headers = headers + ",Authorization,Bearer " + token;
		ctx.SetHeaders(headers);
		return ctx;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestSuccess(RestCallback cb)
	{
		F44_BattleConfig cfg = F44_BattleConfig.FromString(cb.GetData());
		if (!cfg)
		{
			Print("[F44] Loader: invalid JSON from API, falling back to files", LogLevel.WARNING);
			LoadFromFiles();
			return;
		}
		Finish(cfg, "api");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRestError(RestCallback cb)
	{
		PrintFormat("[F44] Loader: API error (rest %1, http %2), falling back to files", cb.GetRestResult(), cb.GetHttpCode(), level: LogLevel.WARNING);
		LoadFromFiles();
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadFromFiles()
	{
		F44_BattleConfig cfg = F44_BattleConfig.FromFile(PROFILE_BATTLE);
		if (cfg)
		{
			Finish(cfg, PROFILE_BATTLE);
			return;
		}
		cfg = F44_BattleConfig.FromFile(MOD_SAMPLE);
		if (cfg)
		{
			Finish(cfg, MOD_SAMPLE);
			return;
		}
		Print("[F44] Loader: no battle config found", LogLevel.ERROR);
		m_OnLoaded.Invoke(null);
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(F44_BattleConfig cfg, string source)
	{
		if (m_sBattleId.IsEmpty())
			m_sBattleId = cfg.m_sBattleId;
		PrintFormat("[F44] Loader: battle %1 loaded from %2 (%3, %4 vs %5)", cfg.m_sBattleId, source, cfg.m_sMode, cfg.m_sAttacker, cfg.m_sDefender);
		m_OnLoaded.Invoke(cfg);
	}
}
