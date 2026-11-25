// AH Icon Renderer - DEV ONLY. Never a dependency of F44_Core, never published.
// Launched with the CLI flag -ahStudio: reads $profile:ah_jobs.txt, renders every job through the
// engine's inventory item preview (ItemPreviewManagerEntity + ItemPreviewWidget, the same renderer
// as the inventory/arsenal), takes two screenshots per job (black clear colour, white clear colour,
// so the post-process can rebuild a true alpha channel) and quits.
// Job line:  KEY|item|<prefab>
//            KEY|char|<character prefab>|<cloth prefab>,<cloth prefab>,...

modded class ArmaReforgerScripted
{
	protected ref AH_IconStudio m_AHStudio;

	override bool OnGameStart()
	{
		bool r = super.OnGameStart();
		if (System.IsCLIParam("ahStudio") && !m_AHStudio)
		{
			m_AHStudio = new AH_IconStudio();
			GetCallqueue().CallLater(m_AHStudio.Begin, 8000, false);
		}
		return r;
	}
}

class AH_IconJob
{
	string m_sKey;
	string m_sMode;
	ResourceName m_sPrefab;
	ref array<string> m_aCloth = {};
	string m_sCat = "item";
}

class AH_IconStudio
{
	protected ref array<ref AH_IconJob> m_aJobs = {};
	protected int m_iIndex = -1;
	protected ItemPreviewWidget m_wPreview;
	protected Widget m_wBack;
	protected IEntity m_Spawned;
	protected int m_iWaitMs = 3500;

	//------------------------------------------------------------------------------------------------
	void Begin()
	{
		ReadJobs();
		string s;
		if (System.GetCLIParam("ahWait", s))
			m_iWaitMs = s.ToInt();
		FileIO.MakeDirectory("$profile:ah_shots");
		PrintFormat("[AHS] %1 jobs, wait %2 ms, screen %3x%4", m_aJobs.Count(), m_iWaitMs, GetGame().GetWorkspace().GetWidth(), GetGame().GetWorkspace().GetHeight());

		WorkspaceWidget ws = GetGame().GetWorkspace();
		m_wPreview = ItemPreviewWidget.Cast(ws.CreateWidget(GameWidgetType.ItemPreviewWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR, Color.FromInt(0xFFFFFFFF), 9999, null));
		if (!m_wPreview)
		{
			Print("[AHS] ERROR cannot create ItemPreviewWidget", LogLevel.ERROR);
			Quit();
			return;
		}
		FrameSlot.SetAnchorMin(m_wPreview, 0, 0);
		FrameSlot.SetAnchorMax(m_wPreview, 1, 1);
		FrameSlot.SetOffsets(m_wPreview, 0, 0, 0, 0);

		Next();
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadJobs()
	{
		FileHandle f = FileIO.OpenFile("$profile:ah_jobs.txt", FileMode.READ);
		if (!f)
		{
			Print("[AHS] ERROR no $profile:ah_jobs.txt", LogLevel.ERROR);
			return;
		}
		string line;
		while (f.ReadLine(line) >= 0)
		{
			line.Trim();
			if (line.IsEmpty() || line.StartsWith("#"))
				continue;
			array<string> p = {};
			line.Split("|", p, false);
			if (p.Count() < 3)
				continue;
			AH_IconJob j = new AH_IconJob();
			j.m_sKey = p[0];
			j.m_sMode = p[1];
			j.m_sPrefab = p[2];
			if (p.Count() > 3 && !p[3].IsEmpty())
				p[3].Split(",", j.m_aCloth, true);
			if (p.Count() > 4 && !p[4].IsEmpty())
				j.m_sCat = p[4];
			m_aJobs.Insert(j);
		}
		f.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected ItemPreviewManagerEntity GetManager()
	{
		ChimeraWorld w = ChimeraWorld.CastFrom(GetGame().GetWorld());
		ItemPreviewManagerEntity m;
		if (w)
			m = w.GetItemPreviewManager();
		if (!m)
		{
			Resource r = Resource.Load("{9F18C476AB860F3B}Prefabs/World/Game/ItemPreviewManager.et");
			if (!r || !r.IsValid())
				r = Resource.Load("Prefabs/World/Game/ItemPreviewManager.et");
			if (r && r.IsValid())
				GetGame().SpawnEntityPrefab(r, GetGame().GetWorld());
			if (w)
				m = w.GetItemPreviewManager();
		}
		return m;
	}

	//------------------------------------------------------------------------------------------------
	protected void Next()
	{
		GetGame().GetMenuManager().CloseAllMenus();
		if (m_Spawned)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_Spawned);
			m_Spawned = null;
		}
		m_iIndex++;
		if (m_iIndex >= m_aJobs.Count())
		{
			Print("[AHS] DONE");
			GetGame().GetCallqueue().CallLater(Quit, 2000, false);
			return;
		}
		AH_IconJob j = m_aJobs[m_iIndex];
		ItemPreviewManagerEntity mgr = GetManager();
		if (!mgr)
		{
			Print("[AHS] ERROR no ItemPreviewManager", LogLevel.ERROR);
			Quit();
			return;
		}

		Layout(j.m_sCat);
		if (j.m_sMode == "char")
		{
			m_Spawned = SpawnDressed(j);
			if (!m_Spawned)
			{
				PrintFormat("[AHS] FAIL spawn %1", j.m_sKey);
				GetGame().GetCallqueue().CallLater(Next, 10, false);
				return;
			}
			// give the inventory a moment to attach the clothes before previewing
			GetGame().GetCallqueue().CallLater(PreviewSpawned, 1500, false);
			return;
		}

		mgr.SetPreviewItemFromPrefab(m_wPreview, j.m_sPrefab, null, true, 0);
		PrintFormat("[AHS] job %1/%2 %3", m_iIndex + 1, m_aJobs.Count(), j.m_sKey);
		GetGame().GetCallqueue().CallLater(ShotBlack, m_iWaitMs, false);
	}

	//------------------------------------------------------------------------------------------------
	//! centred preview rectangle per category; the backing ellipse is 1.6x larger so it covers it
	protected void Layout(string cat)
	{
		float w = 520, h = 520;
		if (cat == "weapon") { w = 1200; h = 400; }
		else if (cat == "char") { w = 380; h = 700; }
		else if (cat == "veh") { w = 1100; h = 680; }
		Place(m_wPreview, w, h);
	}

	protected void Place(Widget wd, float w, float h)
	{
		FrameSlot.SetAnchorMin(wd, 0.5, 0.5);
		FrameSlot.SetAnchorMax(wd, 0.5, 0.5);
		FrameSlot.SetAlignment(wd, 0.5, 0.5);
		FrameSlot.SetPos(wd, 0, 0);
		FrameSlot.SetSize(wd, w, h);
	}

	//------------------------------------------------------------------------------------------------
	protected void PreviewSpawned()
	{
		ItemPreviewManagerEntity mgr = GetManager();
		if (mgr && m_Spawned)
			mgr.SetPreviewItem(m_wPreview, m_Spawned, null, true, 0);
		PrintFormat("[AHS] job %1/%2 %3 (char)", m_iIndex + 1, m_aJobs.Count(), m_aJobs[m_iIndex].m_sKey);
		GetGame().GetCallqueue().CallLater(ShotBlack, m_iWaitMs, false);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnDressed(AH_IconJob j)
	{
		Resource res = Resource.Load(j.m_sPrefab);
		if (!res || !res.IsValid())
			return null;
		EntitySpawnParams sp = new EntitySpawnParams();
		sp.TransformMode = ETransformMode.WORLD;
		sp.Transform[3] = Vector(0, 2000, 0);
		IEntity ch = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), sp);
		if (!ch)
			return null;
		EquipedLoadoutStorageComponent storage = EquipedLoadoutStorageComponent.Cast(ch.FindComponent(EquipedLoadoutStorageComponent));
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(ch.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storage || !inv)
			return ch;
		foreach (string prefab : j.m_aCloth)
		{
			Resource r = Resource.Load(prefab);
			if (!r || !r.IsValid())
				continue;
			EntitySpawnParams ip = new EntitySpawnParams();
			ip.Transform[3] = ch.GetOrigin();
			IEntity item = GetGame().SpawnEntityPrefab(r, GetGame().GetWorld(), ip);
			if (!item)
				continue;
			BaseLoadoutClothComponent cloth = BaseLoadoutClothComponent.Cast(item.FindComponent(BaseLoadoutClothComponent));
			if (!cloth || !cloth.GetAreaType())
			{
				SCR_EntityHelper.DeleteEntityAndChildren(item);
				continue;
			}
			IEntity old = storage.GetClothFromArea(cloth.GetAreaType().Type());
			if (old)
			{
				inv.TryRemoveItemFromStorage(old, storage);
				SCR_EntityHelper.DeleteEntityAndChildren(old);
			}
			if (!inv.TryInsertItemInStorage(item, storage))
				SCR_EntityHelper.DeleteEntityAndChildren(item);
		}
		return ch;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShotBlack()
	{
		System.MakeScreenshot("$profile:ah_shots/" + m_aJobs[m_iIndex].m_sKey);
		GetGame().GetCallqueue().CallLater(Next, 400, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Quit()
	{
		GetGame().RequestClose();
	}
}
