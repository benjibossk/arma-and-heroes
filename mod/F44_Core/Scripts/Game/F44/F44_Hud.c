// Arma & Heroes - minimal battle HUD (tickets, timer, sectors). Client only.
class F44_Hud
{
	protected Widget m_wRoot;
	protected TextWidget m_wTickets;
	protected TextWidget m_wSectors;
	protected float m_fAccu;

	//------------------------------------------------------------------------------------------------
	bool Create(ResourceName layout)
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws || layout.IsEmpty())
			return false;
		m_wRoot = ws.CreateWidgets(layout);
		if (!m_wRoot)
			return false;
		m_wTickets = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_Tickets"));
		m_wSectors = TextWidget.Cast(m_wRoot.FindAnyWidget("F44_Sectors"));
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
	void Update(F44_GameModeComponent gm, float timeSlice)
	{
		m_fAccu += timeSlice;
		if (m_fAccu < 0.5 || !m_wRoot)
			return;
		m_fAccu = 0;

		int t = gm.GetTimeLeft();
		string time = string.Format("%1:%2", (t / 60).ToString(2), (t % 60).ToString(2));
		string line = string.Format("%1 %2  |  %3  |  %4 %5", gm.GetAttacker(), gm.GetTicketsAttacker(), time, gm.GetTicketsDefender(), gm.GetDefender());
		if (!gm.GetWinner().IsEmpty())
			line = "Arma & Heroes - Victory: " + gm.GetWinner();
		if (m_wTickets)
			m_wTickets.SetText(line);

		if (!m_wSectors)
			return;
		string sectors;
		array<int> owners = gm.GetSectorOwners();
		array<int> progress = gm.GetSectorProgress();
		for (int i = 0; i < owners.Count(); i++)
		{
			string name = i.ToString();
			F44_SectorComponent sc = F44_SectorManager.GetSector(i);
			if (sc)
				name = sc.m_sSectorName;
			string ownerTag = "-";
			if (owners[i] == F44_ESectorOwner.ATTACKER)
				ownerTag = gm.GetAttacker();
			else if (owners[i] == F44_ESectorOwner.DEFENDER)
				ownerTag = gm.GetDefender();
			int p = 0;
			if (i < progress.Count())
				p = progress[i];
			sectors += string.Format("[%1 %2 %3]  ", name, ownerTag, p);
		}
		m_wSectors.SetText(sectors);
	}
}
