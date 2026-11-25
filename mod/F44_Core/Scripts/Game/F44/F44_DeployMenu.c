// Arma & Heroes - tactical deploy screen (client only, purely cosmetic).
// Extends the VANILLA deploy menu (SCR_DeployMenuMain): the vanilla map, spawn point icons,
// loadout / group selectors, gamepad navigation and the server-side spawn request are kept as is.
// We only add an overlay: fog outside the battle area, sector diamonds + front links, side ticket
// bars, SOLDIER / ASSAULT TEAM cards and a DEPLOYING panel driven by the vanilla respawn timer.
// See docs/SPAWN_UI.md.
modded class SCR_DeployMenuMain
{
	protected ref F44_DeployOverlay m_F44Overlay;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		if (!F44_GameModeComponent.GetInstance())
			return;
		m_F44Overlay = new F44_DeployOverlay();
		if (!m_F44Overlay.Create())
			m_F44Overlay = null;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		if (!m_F44Overlay)
			return;

		SCR_SpawnPoint sp;
		if (m_iSelectedSpawnPointId.IsValid())
			sp = SCR_SpawnPoint.GetSpawnPointByRplId(m_iSelectedSpawnPointId);
		float remaining = -1;
		float total = 0;
		if (m_ActiveRespawnTimer && sp)
		{
			remaining = m_ActiveRespawnTimer.GetPlayerRemainingTime(m_iPlayerId, sp.GetRespawnTime());
			total = m_ActiveRespawnTimer.GetRespawnTime() + sp.GetRespawnTime();
		}
		string loadout;
		if (m_PlyLoadoutComp && m_PlyLoadoutComp.GetLoadout())
			loadout = m_PlyLoadoutComp.GetLoadout().GetLoadoutName();
		string factionKey;
		if (m_PlyFactionAffilComp && m_PlyFactionAffilComp.GetAffiliatedFaction())
			factionKey = m_PlyFactionAffilComp.GetAffiliatedFaction().GetFactionKey();

		m_F44Overlay.Update(tDelta, m_MapEntity, sp, remaining, total, m_bRespawnRequested, loadout, factionKey, m_iPlayerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		if (m_F44Overlay)
			m_F44Overlay.Destroy();
		m_F44Overlay = null;
		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		if (m_F44Overlay)
			m_F44Overlay.SetVisible(false);
		super.OnMenuHide();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		if (m_F44Overlay)
			m_F44Overlay.SetVisible(true);
	}
}

//------------------------------------------------------------------------------------------------
class F44_DeployOverlay
{
	static const ResourceName LAYOUT = "{F44D0E5A1B2C3D01}UI/layouts/F44_DeployOverlay.layout";
	static const ResourceName MARKER = "{F44D0E5A1B2C3D02}UI/layouts/F44_DeployMarker.layout";
	static const ResourceName LINK = "{F44D0E5A1B2C3D03}UI/layouts/F44_DeployLink.layout";
	static const int COL_NEUTRAL = 0xFF9A9A8F;
	static const int COL_CONTESTED = 0xFFF0A830;
	static const float BAR_W = 330;
	static const float MARGIN = 120;	// m of map kept clear of fog around the sectors

	protected Widget m_wRoot;
	protected ref array<Widget> m_aMarkers = {};
	protected ref array<Widget> m_aLinks = {};
	protected ref array<Widget> m_aFog = {};
	protected float m_fAccu;

	//------------------------------------------------------------------------------------------------
	bool Create()
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		if (!ws)
			return false;
		m_wRoot = ws.CreateWidgets(LAYOUT);
		if (!m_wRoot)
			return false;
		m_wRoot.SetZOrder(5000);
		for (int i = 0; i < 4; i++)
			m_aFog.Insert(m_wRoot.FindAnyWidget("F44_Fog" + i.ToString()));
		IgnoreCursor(m_wRoot);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! the overlay must never eat clicks meant for the vanilla map / buttons
	protected void IgnoreCursor(Widget w)
	{
		while (w)
		{
			w.SetFlags(WidgetFlags.IGNORE_CURSOR);
			IgnoreCursor(w.GetChildren());
			w = w.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	void Destroy()
	{
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		m_wRoot = null;
	}

	void SetVisible(bool v)
	{
		if (m_wRoot)
			m_wRoot.SetVisible(v);
	}

	//------------------------------------------------------------------------------------------------
	static string Up(string s)
	{
		s.ToUpper();
		return s;
	}

	protected void SetText(string name, string text, int color = 0)
	{
		TextWidget t = TextWidget.Cast(m_wRoot.FindAnyWidget(name));
		if (!t)
			return;
		t.SetText(text);
		if (color != 0)
			t.SetColorInt(color);
	}

	protected void SetBar(string name, float f, float width, int color = 0)
	{
		Widget w = m_wRoot.FindAnyWidget(name);
		if (!w)
			return;
		FrameSlot.SetSizeX(w, width * Math.Clamp(f, 0, 1));
		if (color != 0)
			w.SetColorInt(color);
	}

	//------------------------------------------------------------------------------------------------
	void Update(float dt, SCR_MapEntity mapEnt, SCR_SpawnPoint sp, float remaining, float total, bool requested, string loadout, string factionKey, int playerId)
	{
		if (!m_wRoot)
			return;
		F44_GameModeComponent gm = F44_GameModeComponent.GetInstance();
		if (!gm)
			return;

		// map-anchored widgets every frame (pan / zoom), text at 4 Hz
		UpdateMap(mapEnt, gm);
		UpdateDeploy(sp, remaining, total, requested, gm);
		m_fAccu += dt;
		if (m_fAccu < 0.25)
			return;
		m_fAccu = 0;
		UpdateTop(gm);
		UpdateCards(loadout, factionKey, playerId, gm);
	}

	//------------------------------------------------------------------------------------------------
	protected bool ToScreen(SCR_MapEntity mapEnt, vector world, out float x, out float y)
	{
		int sx, sy;
		mapEnt.WorldToScreen(world[0], world[2], sx, sy, true);
		WorkspaceWidget ws = GetGame().GetWorkspace();
		x = ws.DPIUnscale(sx);
		y = ws.DPIUnscale(sy);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected int OwnerColor(int owner, F44_GameModeComponent gm)
	{
		if (owner == F44_ESectorOwner.ATTACKER)
			return F44_Hud.SideColor(gm.GetAttacker());
		if (owner == F44_ESectorOwner.DEFENDER)
			return F44_Hud.SideColor(gm.GetDefender());
		return COL_NEUTRAL;
	}

	//------------------------------------------------------------------------------------------------
	protected Widget GetPooled(array<Widget> pool, int i, ResourceName layout)
	{
		while (pool.Count() <= i)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(layout, m_wRoot);
			if (!w)
				return null;
			IgnoreCursor(w);
			pool.Insert(w);
		}
		return pool[i];
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateMap(SCR_MapEntity mapEnt, F44_GameModeComponent gm)
	{
		bool mapOk = mapEnt && mapEnt.IsOpen();
		array<int> owners = gm.GetSectorOwners();
		array<int> progress = gm.GetSectorProgress();
		int n = F44_SectorManager.Count();
		if (!mapOk)
			n = 0;

		// battle area bounds (world) -> fog around it
		float minX = 1e9, minZ = 1e9, maxX = -1e9, maxZ = -1e9;
		array<float> px = {};
		array<float> py = {};
		for (int i = 0; i < n; i++)
		{
			F44_SectorComponent sc = F44_SectorManager.GetSector(i);
			vector c = sc.GetCenter();
			minX = Math.Min(minX, c[0]); maxX = Math.Max(maxX, c[0]);
			minZ = Math.Min(minZ, c[2]); maxZ = Math.Max(maxZ, c[2]);
			float x, y;
			ToScreen(mapEnt, c, x, y);
			px.Insert(x);
			py.Insert(y);

			Widget mk = GetPooled(m_aMarkers, i, MARKER);
			if (!mk)
				continue;
			mk.SetVisible(true);
			FrameSlot.SetPos(mk, x - 16, y - 16);
			FrameSlot.SetSize(mk, 32, 32);
			int owner = F44_ESectorOwner.NONE;
			if (i < owners.Count())
				owner = owners[i];
			bool contested = i < progress.Count() && F44_SectorSpawns.IsContested(i, owners, progress);
			ImageWidget dia = ImageWidget.Cast(mk.FindAnyWidget("Dia"));
			if (dia)
			{
				dia.SetPivot(0.5, 0.5);
				dia.SetRotation(45);
				dia.SetColorInt(OwnerColor(owner, gm));
			}
			ImageWidget ring = ImageWidget.Cast(mk.FindAnyWidget("Ring"));
			if (ring)
			{
				ring.SetPivot(0.5, 0.5);
				ring.SetRotation(45);
				if (contested)
					ring.SetColorInt(COL_CONTESTED);
				else
					ring.SetColor(Color.FromRGBA(15, 16, 15, 230));
			}
			string letter = sc.m_sSectorName;
			if (letter.Length() > 1)
				letter = letter.Substring(0, 1);
			TextWidget lbl = TextWidget.Cast(mk.FindAnyWidget("Lbl"));
			if (lbl)
				lbl.SetText(letter);
			TextWidget sub = TextWidget.Cast(mk.FindAnyWidget("Sub"));
			if (sub)
			{
				string s = sc.m_sSectorName;
				if (contested)
					s = "CONTESTED";
				sub.SetText(Up(s));
			}
		}
		for (int j = n; j < m_aMarkers.Count(); j++)
			m_aMarkers[j].SetVisible(false);

		// front links between consecutive sectors (assault order)
		int links = Math.Max(0, n - 1);
		for (int l = 0; l < links; l++)
		{
			Widget ln = GetPooled(m_aLinks, l, LINK);
			if (!ln)
				continue;
			float dx = px[l + 1] - px[l];
			float dy = py[l + 1] - py[l];
			float len = Math.Sqrt(dx * dx + dy * dy);
			ln.SetVisible(len > 40);
			FrameSlot.SetPos(ln, (px[l] + px[l + 1]) * 0.5 - len * 0.5, (py[l] + py[l + 1]) * 0.5 - 2);
			FrameSlot.SetSize(ln, len, 4);
			ImageWidget li = ImageWidget.Cast(ln);
			if (li)
			{
				li.SetPivot(0.5, 0.5);
				li.SetRotation(Math.Atan2(dy, dx) * Math.RAD2DEG);
			}
			// front line = owners differ on each side of the link
			bool front = l + 1 < owners.Count() && owners[l] != owners[l + 1];
			if (front)
				ln.SetColorInt(COL_CONTESTED);
			else
				ln.SetColorInt(OwnerColor(owners[l], gm));
			ln.SetZOrder(-1);
		}
		for (int k = links; k < m_aLinks.Count(); k++)
			m_aLinks[k].SetVisible(false);

		UpdateFog(mapEnt, n > 0, minX - MARGIN, minZ - MARGIN, maxX + MARGIN, maxZ + MARGIN);
	}

	//------------------------------------------------------------------------------------------------
	//! four dark panels around the battle rectangle = "clouded" map outside the area
	protected void UpdateFog(SCR_MapEntity mapEnt, bool show, float minX, float minZ, float maxX, float maxZ)
	{
		foreach (Widget f : m_aFog)
		{
			if (f)
				f.SetVisible(show);
		}
		if (!show || m_aFog.Count() < 4)
			return;
		float x0, y0, x1, y1;
		ToScreen(mapEnt, Vector(minX, 0, maxZ), x0, y0);	// top-left (map y flipped)
		ToScreen(mapEnt, Vector(maxX, 0, minZ), x1, y1);	// bottom-right
		float W = 10000, H = 10000;
		Place(m_aFog[0], -W, -H, 3 * W, H + y0);		// above
		Place(m_aFog[1], -W, y1, 3 * W, H);			// below
		Place(m_aFog[2], -W, y0, W + x0, y1 - y0);	// left
		Place(m_aFog[3], x1, y0, W, y1 - y0);		// right
	}

	protected void Place(Widget w, float x, float y, float sw, float sh)
	{
		if (!w)
			return;
		FrameSlot.SetPos(w, x, y);
		FrameSlot.SetSize(w, Math.Max(0, sw), Math.Max(0, sh));
		w.SetZOrder(-2);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateTop(F44_GameModeComponent gm)
	{
		string att = gm.GetAttacker();
		string def = gm.GetDefender();
		SetText("F44_DAttName", Up(att + "  " + gm.GetAttackerName()), F44_Hud.SideColor(att));
		SetText("F44_DDefName", Up(def + "  " + gm.GetDefenderName()), F44_Hud.SideColor(def));
		SetText("F44_DAttTickets", gm.GetTicketsAttacker().ToString());
		SetText("F44_DDefTickets", gm.GetTicketsDefender().ToString());
		SetBar("F44_DAttBar", gm.GetTicketsAttacker() / (float)gm.GetTicketsAttackerMax(), BAR_W, F44_Hud.SideColor(att));
		SetBar("F44_DDefBar", gm.GetTicketsDefender() / (float)gm.GetTicketsDefenderMax(), BAR_W, F44_Hud.SideColor(def));

		// sector pips: one letter per sector, [X] = held by that side
		string pa, pd;
		array<int> owners = gm.GetSectorOwners();
		for (int i = 0; i < owners.Count(); i++)
		{
			string letter = i.ToString();
			F44_SectorComponent sc = F44_SectorManager.GetSector(i);
			if (sc && !sc.m_sSectorName.IsEmpty())
				letter = sc.m_sSectorName.Substring(0, 1);
			if (owners[i] == F44_ESectorOwner.ATTACKER)
				pa += "[" + letter + "] ";
			else
				pa += " .  ";
			if (owners[i] == F44_ESectorOwner.DEFENDER)
				pd += "[" + letter + "] ";
			else
				pd += " .  ";
		}
		SetText("F44_DAttSectors", pa);
		SetText("F44_DDefSectors", pd);
		int t = gm.GetTimeLeft();
		SetText("F44_DTimer", string.Format("%1:%2", (t / 60).ToString(2), (t % 60).ToString(2)));
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateCards(string loadout, string factionKey, int playerId, F44_GameModeComponent gm)
	{
		if (loadout.IsEmpty())
			loadout = "NO SOLDIER SELECTED";
		SetText("F44_SoldClass", Up(WidgetManager.Translate(loadout)));
		SetText("F44_SoldKit", "KIT: CAMPAIGN LOADOUT (A&H PROFILE)\nAPPLIED BY THE SERVER ON DEPLOY");

		string team = "NO TEAM";
		string type = "INFANTRY";
		SCR_GroupsManagerComponent gmc = SCR_GroupsManagerComponent.GetInstance();
		if (gmc)
		{
			SCR_AIGroup grp = gmc.GetPlayerGroup(playerId);
			if (grp)
			{
				team = grp.GetCustomName();
				if (team.IsEmpty())
				{
					string company, platoon, squad, character, format;
					grp.GetCallsigns(company, platoon, squad, character, format);
					team = WidgetManager.Translate(format, company, platoon, squad, character);
				}
				type = string.Format("INFANTRY  -  %1 / %2 MEN", grp.GetPlayerCount(), grp.GetMaxMembers());
			}
		}
		SetText("F44_TeamName", Up(team));
		SetText("F44_TeamType", type);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDeploy(SCR_SpawnPoint sp, float remaining, float total, bool requested, F44_GameModeComponent gm)
	{
		if (!sp)
		{
			SetText("F44_DepTitle", "SELECT A SPAWN POINT");
			SetText("F44_DepInfo", "HQ, FORWARD SECTORS AND VEHICLES ARE SHOWN ON THE MAP");
			SetBar("F44_DepBar", 0, 432);
			return;
		}
		string name = sp.GetSpawnPointName();
		if (name.IsEmpty())
			name = "HQ";
		bool forward = name.StartsWith(F44_SectorSpawns.NAME_PREFIX);
		bool mobile = name.StartsWith(F44_MobileSpawns.NAME_PREFIX);
		string info = Up(WidgetManager.Translate(name));
		if (mobile)
			info = "[TRUCK] " + info + string.Format("   MOBILE SPAWN - DEPLOY DELAY PENALTY +%1 S", Math.Round(sp.GetRespawnTime()));
		else if (forward)
			info += string.Format("   DEPLOY DELAY PENALTY +%1 S", Math.Round(sp.GetRespawnTime()));

		float f = 1;
		if (total > 0 && remaining > 0)
			f = 1 - remaining / total;
		if (requested)
			SetText("F44_DepTitle", "DEPLOYING...", F44_Hud.COL_AMBER);
		else if (remaining > 0)
			SetText("F44_DepTitle", string.Format("DEPLOY IN %1 S", Math.Ceil(remaining)), F44_Hud.COL_TEXT);
		else
			SetText("F44_DepTitle", "READY - PRESS DEPLOY", F44_Hud.COL_TEXT);
		SetText("F44_DepInfo", info);
		SetBar("F44_DepBar", f, 432);
	}
}
