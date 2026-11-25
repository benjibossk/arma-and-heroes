// Arma & Heroes - anti-spoof account link.
// SERVER ONLY: identityId comes from the backend (never from the client).
//   GET  {apiUrl}/api/v1/players/by-identity/{identityId}  -> 200 {"linked":true,"side":"US",...} | 404 / {"linked":false}
//   POST {apiUrl}/api/v1/link/request {"identityId","playerName"} -> {"code":"AH-XXXX","expiresAt":"..."}
// The code is sent to the owning client only (owner RPC on SCR_PlayerController).
// The server token stays on the server (mission header overrides are server side, never replicated).

//------------------------------------------------------------------------------------------------
class F44_LinkRequest
{
	int m_iPlayerId;
	string m_sIdentityId;
	ref RestCallback m_Callback;
	F44_AccountLinkManager m_Manager;

	void OnLookupSuccess(RestCallback cb)
	{
		if (m_Manager)
			m_Manager.OnLookup(this, cb.GetHttpCode(), cb.GetData());
	}

	void OnLookupError(RestCallback cb)
	{
		if (m_Manager)
			m_Manager.OnLookup(this, cb.GetHttpCode(), string.Empty);
	}

	void OnCodeSuccess(RestCallback cb)
	{
		if (m_Manager)
			m_Manager.OnCode(this, cb.GetData());
	}

	void OnCodeError(RestCallback cb)
	{
		PrintFormat("[F44] Link: code request failed for player %1 (http %2)", m_iPlayerId, cb.GetHttpCode(), level: LogLevel.WARNING);
		if (m_Manager)
			m_Manager.Release(this);
	}
}

//------------------------------------------------------------------------------------------------
class F44_AccountLinkManager
{
	static const float RECHECK_MS = 30000;

	protected string m_sApiUrl;
	protected string m_sToken;
	protected ref array<ref F44_LinkRequest> m_aPending = {};
	// playerId -> side key imposed by the website (US / GER)
	protected ref map<int, string> m_mLinkedSide = new map<int, string>();
	// playerId -> last link code shown
	protected ref map<int, string> m_mCodes = new map<int, string>();

	//------------------------------------------------------------------------------------------------
	void Init(string apiUrl, string token)
	{
		m_sApiUrl = apiUrl;
		m_sToken = token;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return !m_sApiUrl.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! Side imposed for this player, empty if not linked / unknown
	string GetLinkedSide(int playerId)
	{
		string side;
		m_mLinkedSide.Find(playerId, side);
		return side;
	}

	//------------------------------------------------------------------------------------------------
	string GetCode(int playerId)
	{
		string c;
		m_mCodes.Find(playerId, c);
		return c;
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerAudited(int playerId)
	{
		if (!IsEnabled())
			return;
		string identityId = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		if (identityId.IsEmpty())
		{
			PrintFormat("[F44] Link: no identityId for player %1", playerId, level: LogLevel.WARNING);
			return;
		}
		Lookup(playerId, identityId);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerDisconnected(int playerId)
	{
		m_mLinkedSide.Remove(playerId);
		m_mCodes.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Lookup(int playerId, string identityId)
	{
		RestContext ctx = F44_BattleLoader.GetRestContext(m_sApiUrl, m_sToken);
		if (!ctx)
			return;
		F44_LinkRequest r = new F44_LinkRequest();
		r.m_iPlayerId = playerId;
		r.m_sIdentityId = identityId;
		r.m_Manager = this;
		r.m_Callback = new RestCallback();
		r.m_Callback.SetOnSuccess(r.OnLookupSuccess);
		r.m_Callback.SetOnError(r.OnLookupError);
		m_aPending.Insert(r);
		ctx.GET(r.m_Callback, "/api/v1/players/by-identity/" + identityId);
	}

	//------------------------------------------------------------------------------------------------
	void OnLookup(F44_LinkRequest r, int httpCode, string data)
	{
		if (!GetGame().GetPlayerManager().IsPlayerConnected(r.m_iPlayerId))
		{
			Release(r);
			return;
		}

		bool linked;
		string side;
		if (httpCode == 200 && !data.IsEmpty())
		{
			F44_JsonNode root = F44_Json.Parse(data);
			if (root)
			{
				F44_JsonNode l = root.Get("linked");
				linked = (l && l.m_eType == F44_EJsonType.JBOOL && l.m_bValue);
				side = root.GetString("side");
				if (!l && !side.IsEmpty())
					linked = true;
			}
		}

		if (linked)
		{
			m_mLinkedSide.Set(r.m_iPlayerId, side);
			m_mCodes.Remove(r.m_iPlayerId);
			PrintFormat("[F44] Link: player %1 linked, side %2", r.m_iPlayerId, side);
			SendToOwner(r.m_iPlayerId, string.Empty, side);
			Release(r);
			return;
		}

		// not linked: ask a code (only once per session, then re-check periodically)
		if (GetCode(r.m_iPlayerId).IsEmpty())
		{
			RequestCode(r);
			return;
		}
		ScheduleRecheck(r.m_iPlayerId, r.m_sIdentityId);
		Release(r);
	}

	//------------------------------------------------------------------------------------------------
	protected void RequestCode(F44_LinkRequest r)
	{
		RestContext ctx = F44_BattleLoader.GetRestContext(m_sApiUrl, m_sToken);
		if (!ctx)
		{
			Release(r);
			return;
		}
		string name = GetGame().GetPlayerManager().GetPlayerName(r.m_iPlayerId);
		name.Replace("\\", "");
		name.Replace("\"", "");
		string body = "{\"identityId\":\"" + r.m_sIdentityId + "\",\"playerName\":\"" + name + "\"}";
		r.m_Callback = new RestCallback();
		r.m_Callback.SetOnSuccess(r.OnCodeSuccess);
		r.m_Callback.SetOnError(r.OnCodeError);
		ctx.POST(r.m_Callback, "/api/v1/link/request", body);
	}

	//------------------------------------------------------------------------------------------------
	void OnCode(F44_LinkRequest r, string data)
	{
		F44_JsonNode root = F44_Json.Parse(data);
		string code;
		if (root)
			code = root.GetString("code");
		if (!code.IsEmpty())
		{
			m_mCodes.Set(r.m_iPlayerId, code);
			PrintFormat("[F44] Link: code issued for player %1", r.m_iPlayerId);
			SendToOwner(r.m_iPlayerId, code, string.Empty);
		}
		ScheduleRecheck(r.m_iPlayerId, r.m_sIdentityId);
		Release(r);
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleRecheck(int playerId, string identityId)
	{
		GetGame().GetCallqueue().CallLater(Recheck, RECHECK_MS, false, playerId, identityId);
	}

	protected void Recheck(int playerId, string identityId)
	{
		if (GetGame().GetPlayerManager().IsPlayerConnected(playerId) && GetLinkedSide(playerId).IsEmpty())
			Lookup(playerId, identityId);
	}

	//------------------------------------------------------------------------------------------------
	void Release(F44_LinkRequest r)
	{
		m_aPending.RemoveItem(r);
	}

	//------------------------------------------------------------------------------------------------
	//! Re-send the current code to the owner (e.g. "!link" chat command on PC)
	void ResendCode(int playerId)
	{
		string code = GetCode(playerId);
		if (!code.IsEmpty())
			SendToOwner(playerId, code, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendToOwner(int playerId, string code, string linkedSide)
	{
		SCR_PlayerController pc = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
		if (pc)
			pc.F44_SendLinkState(code, linkedSide);
	}
}

//------------------------------------------------------------------------------------------------
//! Owner-only RPC + client panel (controller friendly: nothing to click, big code, auto shown)
modded class SCR_PlayerController
{
	protected ref F44_LinkPanel m_F44LinkPanel;

	//------------------------------------------------------------------------------------------------
	//! Server side helper
	void F44_SendLinkState(string code, string linkedSide)
	{
		Rpc(RpcDo_F44LinkState, code, linkedSide);
		// listen server / workbench: owner is local
		if (GetPlayerId() == SCR_PlayerController.GetLocalPlayerId())
			RpcDo_F44LinkState(code, linkedSide);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_F44LinkState(string code, string linkedSide)
	{
		if (System.IsConsoleApp())
			return;
		if (!m_F44LinkPanel)
			m_F44LinkPanel = new F44_LinkPanel();
		if (code.IsEmpty())
			m_F44LinkPanel.ShowLinked(linkedSide);
		else
			m_F44LinkPanel.ShowCode(code);
	}
}

//------------------------------------------------------------------------------------------------
class F44_LinkPanel
{
	static const ResourceName LAYOUT = "{E7A31C5B90D24F68}UI/layouts/F44_LinkPanel.layout";
	static const string SHORT_URL = "heroes.collectifxxl.fr/link";

	protected Widget m_wRoot;

	//------------------------------------------------------------------------------------------------
	protected bool Ensure()
	{
		if (m_wRoot)
			return true;
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws)
			return false;
		m_wRoot = ws.CreateWidgets(LAYOUT);
		if (!m_wRoot)
			return false;
		// above deploy / pause menus
		m_wRoot.SetZOrder(10000);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void ShowCode(string code)
	{
		if (!Ensure())
			return;
		SetText("F44_LinkTitle", "LINK YOUR ARMA & HEROES ACCOUNT");
		SetText("F44_LinkUrl", "Go to " + SHORT_URL);
		SetText("F44_LinkCode", code);
		SetText("F44_LinkHint", "Enter this code on your phone or PC");
		m_wRoot.SetVisible(true);
		// TODO: QR code (https://heroes.collectifxxl.fr/link?code=...) not rendered yet
	}

	//------------------------------------------------------------------------------------------------
	void ShowLinked(string side)
	{
		if (m_wRoot)
			m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetText(string name, string text)
	{
		TextWidget w = TextWidget.Cast(m_wRoot.FindAnyWidget(name));
		if (w)
			w.SetText(text);
	}
}
