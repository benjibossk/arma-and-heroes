// Arma & Heroes - battle HUD "Style B" (client only).
// Top panel: side roleplay names, tickets + bars, AI / player counts, timer, sectors.
// Bottom right: current weapon + rounds in magazine. Scoreboard panel: first seconds of the battle and at the end.
// Palette: charcoal rgba(30,31,32,.9), text #E8E6E1, amber #F0A830, US #3B6FB6, GER #B3302B, uppercase.
class F44_Hud
{
	static const int COL_TEXT = 0xFFE8E6E1;
	static const int COL_AMBER = 0xFFF0A830;
	static const int COL_US = 0xFF3B6FB6;
	static const int COL_GER = 0xFFB3302B;
	static const float BAR_W = 250;

	protected Widget m_wRoot;
	protected TextWidget m_wAttName, m_wAttTickets, m_wAttInfo, m_wDefName, m_wDefTickets, m_wDefInfo;
	protected TextWidget m_wTimer, m_wSectors, m_wWeapon, m_wAmmo, m_wScoreText;
	protected Widget m_wAttBar, m_wDefBar, m_wAmmoPanel, m_wScore;
	protected float m_fAccu;
	protected float m_fShown;

	//------------------------------------------------------------------------------------------------
	bool Create(ResourceName layout)
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws || layout.IsEmpty())
			return false;
		m_wRoot = ws.CreateWidgets(layout);
		if (!m_wRoot)
			return false;
		m_wAttName = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_AttName"));
		m_wAttTickets = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_AttTickets"));
		m_wAttInfo = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_AttInfo"));
		m_wDefName = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_DefName"));
		m_wDefTickets = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_DefTickets"));
		m_wDefInfo = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_DefInfo"));
		m_wTimer = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_Timer"));
		m_wSectors = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_Sectors"));
		m_wWeapon = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_Weapon"));
		m_wAmmo = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_AmmoCount"));
		m_wScoreText = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_ScoreText"));
		m_wAttBar = m_wRoot.FindAnyWidget("F44_AttBar");
		m_wDefBar = m_wRoot.FindAnyWidget("F44_DefBar");
		m_wAmmoPanel = m_wRoot.FindAnyWidget("F44_Ammo");
		m_wScore = m_wRoot.FindAnyWidget("F44_Score");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		m_wRoot = null;
	}

	//------------------------------------------------------------------------------------------------
	//! string.ToUpper() works in place and returns the length: copy first
	static string Up(string s)
	{
		string r = s;
		r.ToUpper();
		return r;
	}

	//------------------------------------------------------------------------------------------------
	static int SideColor(string sideKey)
	{
		if (sideKey == "US")
			return COL_US;
		if (sideKey == "GER")
			return COL_GER;
		return COL_AMBER;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetBar(Widget bar, int value, int max, int color)
	{
		if (!bar)
			return;
		float f = Math.Clamp(value / Math.Max(1, max), 0, 1);
		FrameSlot.SetSizeX(bar, BAR_W * f);
		bar.SetColorInt(color);
	}

	//------------------------------------------------------------------------------------------------
	void Update(F44_GameModeComponent gm, float timeSlice)
	{
		m_fShown += timeSlice;
		m_fAccu += timeSlice;
		if (m_fAccu < 0.25 || !m_wRoot)
			return;
		m_fAccu = 0;

		string att = gm.GetAttacker();
		string def = gm.GetDefender();
		int ta = gm.GetTicketsAttacker();
		int td = gm.GetTicketsDefender();

		if (m_wAttName)
		{
			m_wAttName.SetText(Up(att + " - " + gm.GetAttackerName()));
			m_wAttName.SetColorInt(SideColor(att));
		}
		if (m_wDefName)
		{
			m_wDefName.SetText(Up(def + " - " + gm.GetDefenderName()));
			m_wDefName.SetColorInt(SideColor(def));
		}
		if (m_wAttTickets)
			m_wAttTickets.SetText(ta.ToString());
		if (m_wDefTickets)
			m_wDefTickets.SetText(td.ToString());
		if (m_wAttInfo)
			m_wAttInfo.SetText(string.Format("AI %1  PL %2", gm.GetAIAttacker(), gm.GetPlayersAttacker()));
		if (m_wDefInfo)
			m_wDefInfo.SetText(string.Format("AI %1  PL %2", gm.GetAIDefender(), gm.GetPlayersDefender()));
		SetBar(m_wAttBar, ta, gm.GetTicketsAttackerMax(), SideColor(att));
		SetBar(m_wDefBar, td, gm.GetTicketsDefenderMax(), SideColor(def));

		int t = gm.GetTimeLeft();
		if (m_wTimer)
		{
			if (!gm.GetWinner().IsEmpty())
				m_wTimer.SetText("END");
			else
				m_wTimer.SetText(string.Format("%1:%2", (t / 60).ToString(2), (t % 60).ToString(2)));
		}

		UpdateSectors(gm);
		UpdateAmmo();
		UpdateScoreboard(gm);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateSectors(F44_GameModeComponent gm)
	{
		if (!m_wSectors)
			return;
		string sectors;
		array<int> owners = gm.GetSectorOwners();
		for (int i = 0; i < owners.Count(); i++)
		{
			string name = i.ToString();
			F44_SectorComponent sc = F44_SectorManager.GetSector(i);
			if (sc)
				name = sc.m_sSectorName;
			string tag = "-";
			if (owners[i] == F44_ESectorOwner.ATTACKER)
				tag = gm.GetAttacker();
			else if (owners[i] == F44_ESectorOwner.DEFENDER)
				tag = gm.GetDefender();
			sectors += string.Format("%1:%2  ", name, tag);
		}
		m_wSectors.SetText(Up(sectors));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAmmo()
	{
		if (!m_wAmmoPanel)
			return;
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		BaseWeaponManagerComponent wm;
		if (player)
			wm = BaseWeaponManagerComponent.Cast(player.FindComponent(BaseWeaponManagerComponent));
		BaseWeaponComponent w;
		if (wm)
			w = wm.GetCurrentWeapon();
		if (!w)
		{
			m_wAmmoPanel.SetVisible(false);
			return;
		}
		m_wAmmoPanel.SetVisible(true);
		string wname = w.GetWeaponSlotType();
		UIInfo info = w.GetUIInfo();
		if (info && !info.GetName().IsEmpty())
			wname = info.GetName();
		BaseMagazineComponent mag = w.GetCurrentMagazine();
		string ammo = "--";
		if (mag)
			ammo = string.Format("%1 / %2", mag.GetAmmoCount(), mag.GetMaxAmmoCount());
		if (m_wWeapon)
			m_wWeapon.SetText(Up(WidgetManager.Translate(wname)));
		if (m_wAmmo)
			m_wAmmo.SetText(ammo);
	}

	//------------------------------------------------------------------------------------------------
	//! Scoreboard: shown 10 s after joining and once the battle is over
	protected void UpdateScoreboard(F44_GameModeComponent gm)
	{
		if (!m_wScore)
			return;
		bool show = m_fShown < 10 || !gm.GetWinner().IsEmpty();
		m_wScore.SetVisible(show);
		if (!show || !m_wScoreText)
			return;
		string s = string.Format("%1  %2\n  tickets %3 / %4   AI %5   players %6\n\n", gm.GetAttacker(), gm.GetAttackerName(), gm.GetTicketsAttacker(), gm.GetTicketsAttackerMax(), gm.GetAIAttacker(), gm.GetPlayersAttacker());
		s += string.Format("%1  %2\n  tickets %3 / %4   AI %5   players %6\n\n", gm.GetDefender(), gm.GetDefenderName(), gm.GetTicketsDefender(), gm.GetTicketsDefenderMax(), gm.GetAIDefender(), gm.GetPlayersDefender());
		if (!gm.GetWinner().IsEmpty())
			s += "VICTORY: " + gm.GetWinner();
		m_wScoreText.SetText(Up(s));
	}
}
