// Arma & Heroes - main battle logic. Attach to a SCR_BaseGameMode (see Prefabs/F44/F44_GameMode.et).
// Server: loads BattleConfig, tickets, class restriction, vehicle pools, sectors, timer, end, BattleResult POST.
// Clients: replicated state + HUD.

//------------------------------------------------------------------------------------------------
class F44_PlayerStats
{
	int m_iPlayerId;
	string m_sIdentityId;
	string m_sName;
	string m_sSide;
	string m_sTeamId;
	int m_iKills;
	int m_iDeaths;
	int m_iCaptures;
	int m_iResupplyGiven;	// field supply: magazines given from a box this player loaded on a vehicle
	int m_iHealGiven;
	ref map<string, int> m_mKillsByCategory = new map<string, int>();
}

//------------------------------------------------------------------------------------------------
class F44_TeamLosses
{
	int m_iManpower;
	ref map<string, int> m_mVehicles = new map<string, int>();
}

//------------------------------------------------------------------------------------------------
class F44_PoolEntry
{
	string m_sTeamId;
	string m_sKey;
	int m_iRemaining;
}

//------------------------------------------------------------------------------------------------
class F44_TrackedVehicle
{
	IEntity m_Entity;
	string m_sTeamId;
	string m_sKey;
	int m_iSide;
	float m_fSpawnTime;	// world time (s)
	SCR_AIGroup m_AIGroup;	// AI group that took it (null = free for players / AI)
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "F44", description: "Arma & Heroes battle game mode logic")]
class F44_GameModeComponentClass : SCR_BaseGameModeComponentClass
{
}

class F44_GameModeComponent : SCR_BaseGameModeComponent
{
	[Attribute("{F560924A429DE143}UI/layouts/F44_HUD.layout", UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sHudLayout;

	[Attribute("Configs/F44/catalog.json", desc: "Catalog json inside the mod ($profile:F44/catalog.json always wins)")]
	protected string m_sCatalogPath;

	[Attribute("15", desc: "Delay between battle end and EndGameMode (s)")]
	protected float m_fEndDelay;

	[Attribute("48", desc: "Global AI budget on this server (alive bots, both sides)")]
	protected int m_iAILimit;

	[Attribute("1", desc: "Server tick (s)")]
	protected float m_fTickInterval;

	protected static F44_GameModeComponent s_Instance;

	// ---- replicated state ----
	[RplProp()]
	protected int m_iTicketsAtt;
	[RplProp()]
	protected int m_iTicketsDef;
	[RplProp()]
	protected int m_iTimeLeft;
	[RplProp()]
	protected string m_sAttacker;
	[RplProp()]
	protected string m_sDefender;
	[RplProp()]
	protected ref array<int> m_aSectorOwners = {};
	[RplProp()]
	protected ref array<int> m_aSectorProgress = {};
	[RplProp()]
	protected string m_sWinner;
	// HUD / scoreboard (Style B)
	[RplProp()]
	protected string m_sAttName;	// roleplay unit names of the side (web displayName)
	[RplProp()]
	protected string m_sDefName;
	[RplProp()]
	protected int m_iTicketsAttMax;
	[RplProp()]
	protected int m_iTicketsDefMax;
	[RplProp()]
	protected int m_iAIAtt;
	[RplProp()]
	protected int m_iAIDef;
	[RplProp()]
	protected int m_iPlayersAtt;
	[RplProp()]
	protected int m_iPlayersDef;
	[RplProp(onRplName: "OnLayoutRpl")]
	protected string m_sLayoutSectors;	// city layout sectors "name~x~z~r|..." (empty = world sectors)
	protected string m_sLayoutApplied;

	//------------------------------------------------------------------------------------------------
	protected void OnLayoutRpl()
	{
		if (IsServer() || m_sLayoutSectors == m_sLayoutApplied)
			return;
		m_sLayoutApplied = m_sLayoutSectors;
		F44_CityLayout.ApplySectors(m_sLayoutSectors);
		PrintFormat("[F44] Client: city layout applied (%1 sectors)", F44_SectorManager.Count());
	}

	protected ref F44_CityLayout m_Layout;
	protected ref array<vector> m_aSlotAnchorsAtt = {};
	protected ref array<vector> m_aSlotAnchorsDef = {};
	protected int m_iSlotTries;

	//------------------------------------------------------------------------------------------------
	//! vehicle slots: layout road anchors (or the world slots as anchors), several per side from the pool size
	protected void CollectSlotAnchors()
	{
		if (m_Layout)
		{
			foreach (F44_LayoutPoint p : m_Layout.m_aVehAtt)
				m_aSlotAnchorsAtt.Insert(p.Pos());
			foreach (F44_LayoutPoint p2 : m_Layout.m_aVehDef)
				m_aSlotAnchorsDef.Insert(p2.Pos());
		}
		bool attFromWorld = m_aSlotAnchorsAtt.IsEmpty();
		bool defFromWorld = m_aSlotAnchorsDef.IsEmpty();
		array<F44_VehicleSpawnPointComponent> world = {};
		world.Copy(F44_VehicleSpawnPointComponent.GetAll());
		foreach (F44_VehicleSpawnPointComponent w : world)
		{
			if (w.m_eSide == F44_ESectorOwner.ATTACKER && attFromWorld)
				m_aSlotAnchorsAtt.Insert(w.GetOwner().GetOrigin());
			else if (w.m_eSide == F44_ESectorOwner.DEFENDER && defFromWorld)
				m_aSlotAnchorsDef.Insert(w.GetOwner().GetOrigin());
			SCR_EntityHelper.DeleteEntityAndChildren(w.GetOwner());
		}
	}

	protected void SetupVehicleSlots()
	{
		if (!F44_Roads.Get() && m_iSlotTries < 15)
		{
			m_iSlotTries++;
			GetGame().GetCallqueue().CallLater(SetupVehicleSlots, 2000, false);
			return;
		}
		vector facing = "2000 0 2000";
		if (F44_SectorManager.Count() > 0)
			facing = F44_SectorManager.GetSector(0).GetOwner().GetOrigin();
		F44_CityLayout.BuildVehicleSlots(m_aSlotAnchorsAtt, F44_ESectorOwner.ATTACKER, SlotCount(m_aPoolAtt), facing);
		F44_CityLayout.BuildVehicleSlots(m_aSlotAnchorsDef, F44_ESectorOwner.DEFENDER, SlotCount(m_aPoolDef), facing);
	}

	protected static int SlotCount(array<ref F44_PoolEntry> pool)
	{
		int n;
		foreach (F44_PoolEntry e : pool)
			n += e.m_iRemaining;
		return Math.ClampInt(n, 2, 6);
	}

	// ---- server state ----
	protected ref F44_BattleLoader m_Loader;
	protected ref F44_BattleConfig m_Config;
	protected bool m_bRunning;
	protected bool m_bEnded;
	protected float m_fElapsed;
	protected ref map<int, ref F44_PlayerStats> m_mStats = new map<int, ref F44_PlayerStats>();
	protected ref map<int, int> m_mPlayerSide = new map<int, int>(); // playerId -> F44_ESectorOwner
	protected ref map<string, ref F44_TeamLosses> m_mLosses = new map<string, ref F44_TeamLosses>();
	protected ref map<string, int> m_mTeamDeaths = new map<string, int>();
	protected ref array<ref F44_PoolEntry> m_aPoolAtt = {};
	protected ref array<ref F44_PoolEntry> m_aPoolDef = {};
	protected ref array<ref F44_TrackedVehicle> m_aVehicles = {};
	protected ref RestCallback m_ResultCallback;
	protected string m_sResultJson;
	protected int m_iResultAttempt;
	static const int RESULT_MAX_ATTEMPTS = 3;
	protected ref F44_AccountLinkManager m_Link = new F44_AccountLinkManager();
	protected ref F44_AIDirector m_AI = new F44_AIDirector();
	protected ref F44_LoadoutService m_Loadouts = new F44_LoadoutService();
	protected ref F44_FieldSupply m_FieldSupply = new F44_FieldSupply();	// F44_FieldSupply.c
	protected ref F44_CivVehicleSpawner m_CivVehicles = new F44_CivVehicleSpawner();	// F44_CivVehicleSpawner.c
	protected int m_iBotKills;
	protected ref map<string, int> m_mAIDeaths = new map<string, int>();	// side key -> bots lost
	protected ref map<string, int> m_mAIKills = new map<string, int>();	// side key -> kills made by bots
	protected ref map<string, int> m_mTeamAIKills = new map<string, int>();	// team id -> kills made by its bots

	//! average rank index (0 = Private .. 7 = Captain) of connected players of a side, -1 if none known
	float AvgPlayerRank(int side)
	{
		array<string> ranks = {"Private", "Corporal", "Sergeant", "Staff Sergeant", "Warrant Officer", "Second Lieutenant", "Lieutenant", "Captain"};
		float sum;
		int n;
		foreach (int pid, int s : m_mPlayerSide)
		{
			if (s != side || !m_Loadouts)
				continue;
			F44_PlayerLoadout l = m_Loadouts.Get(SCR_PlayerIdentityUtils.GetPlayerIdentityId(pid));
			if (!l)
				continue;
			int r = ranks.Find(l.m_sRank);
			if (r < 0)
				continue;
			sum += r;
			n++;
		}
		if (n == 0)
			return -1;
		return sum / n;
	}

	protected string SideKeyOf(int side)
	{
		if (side == F44_ESectorOwner.ATTACKER)
			return m_sAttacker;
		return m_sDefender;
	}

	protected static void MapAdd(map<string, int> m, string k, int v)
	{
		int cur;
		m.Find(k, cur);
		m.Set(k, cur + v);
	}

	protected static string MapJson(map<string, int> m)
	{
		string s = "{";
		bool first = true;
		foreach (string k, int v : m)
		{
			if (!first)
				s += ",";
			first = false;
			s += string.Format("\"%1\":%2", k, v);
		}
		return s + "}";
	}

	// ---- client ----
	protected ref F44_Hud m_Hud;

	//------------------------------------------------------------------------------------------------
	static F44_GameModeComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;
		if (!System.IsConsoleApp())
			SetEventMask(owner, EntityEvent.FRAME);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_Instance == this)
			s_Instance = null;
		GetGame().GetCallqueue().Remove(ServerTick);
		if (m_Hud)
			m_Hud.Destroy();
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsServer()
	{
		return Replication.IsServer();
	}

	//------------------------------------------------------------------------------------------------
	override void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		if (!IsServer() || !GetGame().InPlayMode())
			return;

		F44_Catalog.Get(m_sCatalogPath);
		F44_WorldDump.RunIfRequested();
		F44_MissionHeader header = F44_MissionHeader.Get();
		if (header)
			m_Link.Init(header.m_sApiUrl, header.m_sToken);
		m_Loader = new F44_BattleLoader();
		m_Loader.GetOnLoaded().Insert(OnConfigLoaded);
		m_Loader.Load();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfigLoaded(F44_BattleConfig cfg)
	{
		if (!cfg)
		{
			Print("[F44] No battle config: battle logic disabled", LogLevel.ERROR);
			return;
		}
		m_Config = cfg;
		F44_MissionHeader hdr = F44_MissionHeader.Get();
		if (hdr && hdr.m_iTimeLimitOverrideMin > 0)
		{
			Print("[F44] Time limit override: " + hdr.m_iTimeLimitOverrideMin + " min");
			cfg.m_iTimeLimitMin = hdr.m_iTimeLimitOverrideMin;
		}
		m_sAttacker = cfg.m_sAttacker;
		m_sDefender = cfg.m_sDefender;
		m_iTicketsAtt = cfg.m_Attacker.GetTotalManpower();
		m_iTicketsDef = cfg.m_Defender.GetTotalManpower();
		m_iTimeLeft = cfg.m_iTimeLimitMin * 60;
		m_iTicketsAttMax = Math.Max(1, m_iTicketsAtt);
		m_iTicketsDefMax = Math.Max(1, m_iTicketsDef);
		m_sAttName = SideDisplayName(cfg.m_Attacker);
		m_sDefName = SideDisplayName(cfg.m_Defender);
		PrintFormat("[F44] HUD: %1 \"%2\" vs %3 \"%4\"", m_sAttacker, m_sAttName, m_sDefender, m_sDefName);

		foreach (F44_Team t : cfg.m_Attacker.m_aTeams)
			InitTeam(t, m_aPoolAtt, cfg.m_sAttacker);
		foreach (F44_Team t : cfg.m_Defender.m_aTeams)
			InitTeam(t, m_aPoolDef, cfg.m_sDefender);

		// test only (orchestrator "testVehicles"): extra pool keys, side = key prefix (US_/GER_)
		if (hdr && !hdr.m_sTestVehicles.IsEmpty())
		{
			array<string> extra = {};
			hdr.m_sTestVehicles.Split(",", extra, true);
			foreach (string xk : extra)
			{
				F44_PoolEntry xe = new F44_PoolEntry();
				xe.m_sTeamId = "test";
				xe.m_sKey = xk.Trim();
				xe.m_iRemaining = 1;
				if (xe.m_sKey.StartsWith(cfg.m_sAttacker + "_"))
					m_aPoolAtt.InsertAt(xe, 0);
				else if (xe.m_sKey.StartsWith(cfg.m_sDefender + "_"))
					m_aPoolDef.InsertAt(xe, 0);
				Print("[F44] Test vehicle added to pool: " + xe.m_sKey);
			}
		}

		// city layout (battle.layout / layoutData / cityMap[cityId]) replaces the world sectors
		m_Layout = cfg.m_InlineLayout;
		if (!m_Layout)
			m_Layout = F44_CityLayout.Resolve(cfg.m_sLayoutId, cfg.m_sCityId);
		if (m_Layout)
		{
			m_sLayoutSectors = m_Layout.SerializeSectors();
			F44_CityLayout.ApplySectors(m_sLayoutSectors);
		}

		// sectors: ASSAULT = defender holds everything, SKIRMISH = neutral
		m_aSectorOwners.Clear();
		m_aSectorProgress.Clear();
		for (int i = 0; i < F44_SectorManager.Count(); i++)
		{
			if (cfg.GetMode() == F44_EBattleMode.ASSAULT)
			{
				m_aSectorOwners.Insert(F44_ESectorOwner.DEFENDER);
				m_aSectorProgress.Insert(-100);
			}
			else
			{
				m_aSectorOwners.Insert(F44_ESectorOwner.NONE);
				m_aSectorProgress.Insert(0);
			}
		}

		ConfigureSpawnPoints();
		if (m_Layout)
		{
			F44_Catalog lcat = F44_Catalog.Get();
			m_Layout.ApplySpawns(lcat.GetFactionKey(m_sAttacker), lcat.GetFactionKey(m_sDefender));
		}
		CollectSlotAnchors();
		GetGame().GetCallqueue().CallLater(SetupVehicleSlots, 5000, false);	// road network ready
		GetGame().GetCallqueue().CallLater(SetupFieldSupply, 8000, false);	// after layout spawns/sectors

		int aiLimit = m_iAILimit;
		if (cfg.m_iAILimit >= 0)
			aiLimit = Math.Min(aiLimit, cfg.m_iAILimit);
		m_AI.Init(this, aiLimit);
		m_Loadouts.Init(cfg, m_Loader.GetApiUrl(), m_Loader.GetToken(), m_Loader.GetBattleId());

		m_bRunning = true;
		Replication.BumpMe();
		GetGame().GetCallqueue().CallLater(ServerTick, m_fTickInterval * 1000, true);
		PrintFormat("[F44] Battle %1 started: %2 (%3) vs %4 (%5), %6 sectors, %7 min",
			cfg.m_sBattleId, m_sAttacker, m_iTicketsAtt, m_sDefender, m_iTicketsDef, F44_SectorManager.Count(), cfg.m_iTimeLimitMin);
	}

	//------------------------------------------------------------------------------------------------
	//! Real terrain extent for the web live map: world bound box + map entity size (what the map texture covers)
	protected string m_sWorldJson;
	protected GenericTerrainEntity m_Terrain;
	protected float ProbeTerrainEdge(bool alongX)
	{
		// outside the terrain the height is clamped to the edge value: walk inward until it changes
		BaseWorld w = GetGame().GetWorld();
		float far = w.GetSurfaceY(16384, 1500);
		if (!alongX)
			far = w.GetSurfaceY(1500, 16384);
		for (float d = 16384; d > 0; d -= 2)
		{
			float y = w.GetSurfaceY(d, 1500);
			if (!alongX)
				y = w.GetSurfaceY(1500, d);
			if (Math.AbsFloat(y - far) > 0.001)
				return Math.Round(d + 2);	// last vertex (2 m grid)
		}
		return -1;
	}
	protected string WorldJson()
	{
		if (!m_sWorldJson.IsEmpty())
			return m_sWorldJson;
		vector mins, maxs;
		GetGame().GetWorld().GetBoundBox(mins, maxs);
		float mapX, mapZ;
		SCR_MapEntity me = SCR_MapEntity.GetMapInstance();
		if (me)
		{
			mapX = me.GetMapSizeX();
			mapZ = me.GetMapSizeY();
		}
		// terrain entity extent (the world bound box also counts objects placed off the terrain)
		GenericWorldEntity we = GetGame().GetWorldEntity();
		if (we)
			m_Terrain = we.GetTerrain(1000, 1000);
		if (m_Terrain)
		{
			// terrain extent: probe where the terrain has a height sample (8 m steps along both axes)
			float tMaxX = ProbeTerrainEdge(true);
			float tMaxZ = ProbeTerrainEdge(false);
			if (tMaxX > 0 && tMaxZ > 0)
			{
				mins = Vector(0, 0, 0);
				maxs = Vector(tMaxX, 0, tMaxZ);
			}
		}
		else
			Print("[F44] Terrain entity not found, using the world bound box", LogLevel.WARNING);
		string wf = GetGame().GetWorldFile();
		m_sWorldJson = string.Format("\"world\":{\"file\":\"%1\",\"minX\":%2,\"minZ\":%3,\"maxX\":%4,\"maxZ\":%5,\"mapSizeX\":%6,\"mapSizeZ\":%7},",
			wf, Math.Round(mins[0]), Math.Round(mins[2]), Math.Round(maxs[0]), Math.Round(maxs[2]), Math.Round(mapX), Math.Round(mapZ));
		Print("[F44] Terrain bounds: " + m_sWorldJson);
		return m_sWorldJson;
	}

	//------------------------------------------------------------------------------------------------
	//! "Easy Company, 2nd Bn, 506th PIR +1" (first team name, + number of other teams)
	protected static string SideDisplayName(F44_Side side)
	{
		if (!side || side.m_aTeams.IsEmpty())
			return "Garrison";
		string n = side.m_aTeams[0].m_sDisplayName;
		if (side.m_aTeams.Count() > 1)
			n += " +" + (side.m_aTeams.Count() - 1).ToString();
		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitTeam(F44_Team t, array<ref F44_PoolEntry> pool, string sideKey)
	{
		F44_Catalog cat = F44_Catalog.Get();
		foreach (string c : t.m_aClasses)
		{
			string full = F44_Catalog.MakeClassKey(sideKey, c);
			if (cat.GetClassPrefab(full).IsEmpty())
				PrintFormat("[F44] Team %1: class %2 unavailable in catalog (no prefab), players cannot pick it", t.m_sTeamId, full, level: LogLevel.WARNING);
		}
		m_mLosses.Set(t.m_sTeamId, new F44_TeamLosses());
		m_mTeamDeaths.Set(t.m_sTeamId, 0);
		foreach (F44_VehicleEntry v : t.m_aVehicles)
		{
			F44_PoolEntry e = new F44_PoolEntry();
			e.m_sTeamId = t.m_sTeamId;
			e.m_sKey = v.m_sKey;
			e.m_iRemaining = v.m_iCount;
			pool.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn points use generic faction keys from the catalog; enable only the two battle factions.
	protected void ConfigureSpawnPoints()
	{
		F44_Catalog cat = F44_Catalog.Get();
		string attFaction = cat.GetFactionKey(m_sAttacker);
		string defFaction = cat.GetFactionKey(m_sDefender);
		array<SCR_SpawnPoint> points = SCR_SpawnPoint.GetSpawnPoints();
		foreach (SCR_SpawnPoint sp : points)
		{
			string fk = sp.GetFactionKey();
			bool used = (fk == attFaction || fk == defFaction);
			sp.SetSpawnPointEnabled_S(used);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int GetSideOfFaction(string factionKey)
	{
		F44_Catalog cat = F44_Catalog.Get();
		string side = cat.GetSideFromFaction(factionKey);
		if (side == m_sAttacker)
			return F44_ESectorOwner.ATTACKER;
		if (side == m_sDefender)
			return F44_ESectorOwner.DEFENDER;
		return F44_ESectorOwner.NONE;
	}

	//------------------------------------------------------------------------------------------------
	protected string SideKey(int side)
	{
		if (side == F44_ESectorOwner.ATTACKER)
			return m_sAttacker;
		if (side == F44_ESectorOwner.DEFENDER)
			return m_sDefender;
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Field supply (F44_FieldSupply.c): spawn crates/boxes and civilian vehicles once the layout is applied
	protected void SetupFieldSupply()
	{
		if (!m_Config || m_bEnded)
			return;
		m_FieldSupply.Init(this, m_Config);
		m_CivVehicles.Init(m_Config, m_FieldSupply.GetConfig());
	}

	//! Logistics credit for the owner of a loaded supply box
	void F44_AddLogistics(int playerId, int resupply, int heal)
	{
		F44_PlayerStats st = GetStats(playerId);
		if (!st)
			return;
		st.m_iResupplyGiven += resupply;
		st.m_iHealGiven += heal;
	}

	//------------------------------------------------------------------------------------------------
	protected F44_PlayerStats GetStats(int playerId)
	{
		F44_PlayerStats st;
		if (m_mStats.Find(playerId, st))
			return st;
		st = new F44_PlayerStats();
		st.m_iPlayerId = playerId;
		st.m_sName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		st.m_sIdentityId = SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId);
		m_mStats.Set(playerId, st);
		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! Class restriction: refuse spawning a character prefab not allowed by the player's side teams.
	override bool PreparePlayerEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnHandlerComponent handlerComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!m_bRunning || m_bEnded || !entity)
			return !m_bEnded;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
			return true;

		int side = GetSideOfFaction(character.GetFactionKey());
		F44_Side cfgSide = m_Config.GetSide(SideKey(side));
		if (!cfgSide)
			return false; // faction not part of this battle

		// no tickets left -> no respawn
		if ((side == F44_ESectorOwner.ATTACKER && m_iTicketsAtt <= 0) || (side == F44_ESectorOwner.DEFENDER && m_iTicketsDef <= 0))
			return false;

		string prefab;
		EntityPrefabData pd = entity.GetPrefabData();
		if (pd)
			prefab = pd.GetPrefabName();

		F44_Catalog cat = F44_Catalog.Get();
		string classKey = cat.FindClassByPrefab(cfgSide.m_sSideKey, prefab);

		// pick the first team allowing this class
		F44_Team team;
		foreach (F44_Team t : cfgSide.m_aTeams)
		{
			if (classKey.IsEmpty() || t.AllowsClass(classKey))
			{
				if (!classKey.IsEmpty() || !team)
					team = t;
				if (!classKey.IsEmpty())
					break;
			}
		}
		if (classKey.IsEmpty() || !team)
		{
			PrintFormat("[F44] Spawn refused: prefab %1 is not an allowed class for %2", prefab, cfgSide.m_sSideKey, level: LogLevel.WARNING);
			return false;
		}

		int playerId = requestComponent.GetPlayerId();
		// linked account: the website decides the side
		string forced = m_Link.GetLinkedSide(playerId);
		if (!forced.IsEmpty() && forced != cfgSide.m_sSideKey)
		{
			PrintFormat("[F44] Spawn refused: player %1 is linked to side %2", playerId, forced, level: LogLevel.WARNING);
			return false;
		}
		F44_PlayerStats st = GetStats(playerId);

		// persistent equipment: linked players only (or when the link service is off, e.g. local tests)
		F44_PlayerLoadout lo = m_Loadouts.Get(st.m_sIdentityId);
		if (lo && (!m_Link.IsEnabled() || !forced.IsEmpty()))
		{
			if (!lo.m_sClass.IsEmpty() && lo.m_sClass != classKey)
			{
				PrintFormat("[F44] Spawn refused: player %1 soldier is a %2", playerId, lo.m_sClass, level: LogLevel.WARNING);
				return false;
			}
			F44_LoadoutService.Apply(entity, lo);
		}

		st.m_sSide = cfgSide.m_sSideKey;
		st.m_sTeamId = team.m_sTeamId;
		m_mPlayerSide.Set(playerId, side);
		F44_Medical.SetPlayerClass(playerId, classKey);	// medical (F44_Medical.c)
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Account link: identity is resolved here, server side, after backend audit
	override void OnPlayerAuditSuccess(int playerId)
	{
		super.OnPlayerAuditSuccess(playerId);
		if (IsServer())
		{
			m_Link.OnPlayerAudited(playerId);
			m_Loadouts.Fetch(SCR_PlayerIdentityUtils.GetPlayerIdentityId(playerId));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerDisconnected(int playerId, KickCauseCode cause, int timeout)
	{
		super.OnPlayerDisconnected(playerId, cause, timeout);
		if (IsServer())
			m_Link.OnPlayerDisconnected(playerId);
	}

	//------------------------------------------------------------------------------------------------
	F44_AccountLinkManager GetLinkManager()
	{
		return m_Link;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilled(instigatorContextData);
		if (!IsServer() || !m_bRunning || m_bEnded)
			return;

		int victimId = instigatorContextData.GetVictimPlayerID();
		int killerId = instigatorContextData.GetKillerPlayerID();
		int side;
		if (!m_mPlayerSide.Find(victimId, side))
			return;

		F44_PlayerStats victim = GetStats(victimId);
		victim.m_iDeaths++;
		AddManpowerLoss(victim.m_sTeamId, side, 1);

		if (killerId > 0 && killerId != victimId)
		{
			int killerSide;
			if (m_mPlayerSide.Find(killerId, killerSide) && killerSide != side)
				RecordKill(killerId, instigatorContextData.GetKillerEntity());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bots: each death costs 1 ticket and 1 manpower of its Assault Team
	override void OnControllableDestroyed(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnControllableDestroyed(instigatorContextData);
		if (!IsServer() || !m_bRunning || m_bEnded)
			return;
		IEntity victim = instigatorContextData.GetVictimEntity();
		int side;
		string teamId;
		if (!m_AI.IsBot(victim, side, teamId))
			return;
		m_AI.OnBotKilled(victim);
		AddManpowerLoss(teamId, side, 1);
		MapAdd(m_mAIDeaths, SideKeyOf(side), 1);
		int kSide;
		string kTeam;
		IEntity killerEnt = instigatorContextData.GetKillerEntity();
		if (killerEnt && m_AI.IsBot(killerEnt, kSide, kTeam) && kSide != side)
		{
			MapAdd(m_mAIKills, SideKeyOf(kSide), 1);
			MapAdd(m_mTeamAIKills, kTeam, 1);
		}
		PrintFormat("[F44] Tickets: bot of side %1 (team %2) killed -> att %3 / def %4", side, teamId, m_iTicketsAtt, m_iTicketsDef);

		int killerId = instigatorContextData.GetKillerPlayerID();
		int killerSide;
		if (killerId > 0 && m_mPlayerSide.Find(killerId, killerSide) && killerSide != side)
		{
			RecordKill(killerId, instigatorContextData.GetKillerEntity());
			m_iBotKills++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Kill + weapon category (rifle, smg, lmg, sniper, at, explosive, vehicle) for ribbons.
	//! Picked-up weapons count for their own category.
	protected void RecordKill(int killerId, IEntity killer)
	{
		F44_PlayerStats st = GetStats(killerId);
		st.m_iKills++;
		string cat = WeaponCategoryOf(killer);
		int n;
		st.m_mKillsByCategory.Find(cat, n);
		st.m_mKillsByCategory.Set(cat, n + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected string WeaponCategoryOf(IEntity killer)
	{
		if (!killer)
			return "rifle";
		if (!ChimeraCharacter.Cast(killer))
			return "vehicle";
		CompartmentAccessComponent cac = CompartmentAccessComponent.Cast(killer.FindComponent(CompartmentAccessComponent));
		if (cac && cac.IsInCompartment())
			return "vehicle";
		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(killer.FindComponent(BaseWeaponManagerComponent));
		if (!wm)
			return "rifle";
		BaseWeaponComponent w = wm.GetCurrentWeapon();
		if (!w || !w.GetOwner() || !w.GetOwner().GetPrefabData())
			return "explosive";
		return F44_Catalog.Get().GetWeaponCategory(w.GetOwner().GetPrefabData().GetPrefabName());
	}

	//------------------------------------------------------------------------------------------------
	protected void AddManpowerLoss(string teamId, int side, int amount)
	{
		F44_TeamLosses l;
		if (m_mLosses.Find(teamId, l))
			l.m_iManpower = l.m_iManpower + amount;
		RemoveTickets(side, amount);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveTickets(int side, int amount)
	{
		if (side == F44_ESectorOwner.ATTACKER)
			m_iTicketsAtt = Math.Max(0, m_iTicketsAtt - amount);
		else if (side == F44_ESectorOwner.DEFENDER)
			m_iTicketsDef = Math.Max(0, m_iTicketsDef - amount);
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	protected float m_fStatusAccu;
	protected float m_fLiveAccu;
	protected ref RestCallback m_LiveCallback;
	static const int LIVE_MAX_UNITS = 200;

	//------------------------------------------------------------------------------------------------
	//! Live snapshot for the website battle view (every 5 s, no identities, coords rounded to 1 m)
	protected void PostLive(array<IEntity> botsAtt, array<IEntity> botsDef)
	{
		if (m_Loader.GetApiUrl().IsEmpty() || m_Loader.GetBattleId().IsEmpty())
			return;
		RestContext ctx = F44_BattleLoader.GetRestContext(m_Loader.GetApiUrl(), m_Loader.GetToken());
		if (!ctx)
			return;
		string json = string.Format("{\"elapsed\":%1,\"timeLeft\":%2,\"tickets\":{\"%3\":%4,\"%5\":%6},",
			Math.Round(m_fElapsed), m_iTimeLeft, m_sAttacker, m_iTicketsAtt, m_sDefender, m_iTicketsDef);
		json += string.Format("\"aiSkill\":{\"%1\":\"%2\",\"%3\":\"%4\"},", m_sAttacker, m_AI.GetSkill(F44_ESectorOwner.ATTACKER), m_sDefender, m_AI.GetSkill(F44_ESectorOwner.DEFENDER));
		json += WorldJson();
		json += "\"sectors\":[";
		bool first = true;
		for (int i = 0; i < m_aSectorOwners.Count(); i++)
		{
			F44_SectorComponent sc = F44_SectorManager.GetSector(i);
			if (!sc)
				continue;
			vector c = sc.GetCenter();
			string owner = "";
			if (m_aSectorOwners[i] == F44_ESectorOwner.ATTACKER)
				owner = m_sAttacker;
			else if (m_aSectorOwners[i] == F44_ESectorOwner.DEFENDER)
				owner = m_sDefender;
			int prog;
			if (i < m_aSectorProgress.Count())
				prog = m_aSectorProgress[i];
			if (!first)
				json += ",";
			first = false;
			string s1 = string.Format("{\"id\":%1,\"name\":\"%2\",\"x\":%3,\"z\":%4,", i, Esc(sc.m_sSectorName), Math.Round(c[0]), Math.Round(c[2]));
			string s2 = string.Format("\"r\":%1,\"owner\":\"%2\",\"captureProgress\":%3}", Math.Round(sc.m_fRadius), owner, prog);
			json += s1 + s2;
		}
		json += "],\"units\":[";
		int n;
		n = AppendUnits(json, botsAtt, m_sAttacker, true, n);
		n = AppendUnits(json, botsDef, m_sDefender, true, n);
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);
		foreach (int pid : players)
		{
			int ps;
			if (!m_mPlayerSide.Find(pid, ps))
				continue;
			IEntity pe = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
			if (!pe)
				continue;
			array<IEntity> one = {pe};
			string sk = m_sDefender;
			if (ps == F44_ESectorOwner.ATTACKER)
				sk = m_sAttacker;
			n = AppendUnits(json, one, sk, false, n);
		}
		json += "]}";
		m_LiveCallback = new RestCallback();
		ctx.POST(m_LiveCallback, "/api/v1/battles/" + m_Loader.GetBattleId() + "/live", json);
	}

	protected int AppendUnits(inout string json, array<IEntity> ents, string side, bool ai, int n)
	{
		string aiStr = "false";
		if (ai)
			aiStr = "true";
		foreach (IEntity e : ents)
		{
			if (n >= LIVE_MAX_UNITS || !e || !F44_SectorManager.IsAlive(e))
				continue;
			vector o = e.GetOrigin();
			string veh = "false";
			ChimeraCharacter ch = ChimeraCharacter.Cast(e);
			if (ch && ch.IsInVehicle())
				veh = "true";
			if (n > 0)
				json += ",";
			json += string.Format("{\"side\":\"%1\",\"isAI\":%2,\"x\":%3,\"z\":%4,\"vehicle\":%5}", side, aiStr, Math.Round(o[0]), Math.Round(o[2]), veh);
			n++;
		}
		return n;
	}

	protected string SectorString()
	{
		string s;
		for (int i = 0; i < m_aSectorOwners.Count(); i++)
			s += m_aSectorOwners[i].ToString() + " ";
		return s;
	}

	protected void ServerTick()
	{
		if (!m_bRunning || m_bEnded)
			return;

		m_fElapsed += m_fTickInterval;
		m_iTimeLeft = Math.Max(0, m_Config.m_iTimeLimitMin * 60 - Math.Round(m_fElapsed));

		// sectors
		array<int> capturers = {};
		int playersAtt, playersDef;
		foreach (int p, int ps : m_mPlayerSide)
		{
			if (!GetGame().GetPlayerManager().IsPlayerConnected(p))
				continue;
			if (ps == F44_ESectorOwner.ATTACKER)
				playersAtt++;
			else if (ps == F44_ESectorOwner.DEFENDER)
				playersDef++;
		}
		m_AI.Tick(m_fTickInterval, m_Config, playersAtt, playersDef, m_iTicketsAtt, m_iTicketsDef, m_aSectorOwners);
		array<IEntity> botsAtt = {};
		array<IEntity> botsDef = {};
		m_AI.GetBots(F44_ESectorOwner.ATTACKER, botsAtt);
		m_AI.GetBots(F44_ESectorOwner.DEFENDER, botsDef);
		F44_SectorManager.Simulate(m_fTickInterval, m_Config.GetMode(), m_aSectorOwners, m_aSectorProgress, m_mPlayerSide, capturers, botsAtt, botsDef);
		foreach (int pid : capturers)
			GetStats(pid).m_iCaptures++;

		UpdateVehicles();
		m_FieldSupply.Tick(m_fTickInterval);
		m_CivVehicles.Tick(m_fTickInterval);
		m_FieldSupply.SelfTest(m_fTickInterval, botsAtt, botsDef, m_CivVehicles);
		m_iAIAtt = botsAtt.Count();
		m_iAIDef = botsDef.Count();
		m_iPlayersAtt = playersAtt;
		m_iPlayersDef = playersDef;
		m_fStatusAccu += m_fTickInterval;
		if (m_fStatusAccu >= 30)
		{
			m_fStatusAccu = 0;
			PrintFormat("[F44] Status t=%1s: tickets att %2 / def %3, bots alive att %4 / def %5, sectors %6, time left %7s",
				Math.Round(m_fElapsed), m_iTicketsAtt, m_iTicketsDef, botsAtt.Count(), botsDef.Count(), SectorString(), m_iTimeLeft);
		}
		m_fLiveAccu += m_fTickInterval;
		if (m_fLiveAccu >= 5)
		{
			m_fLiveAccu = 0;
			PostLive(botsAtt, botsDef);
		}
		Replication.BumpMe();
		CheckEnd();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateVehicles()
	{
		// destroyed / deleted vehicles
		for (int i = m_aVehicles.Count() - 1; i >= 0; i--)
		{
			F44_TrackedVehicle tv = m_aVehicles[i];
			if (!tv.m_Entity)
			{
				m_aVehicles.Remove(i);
				continue;
			}
			DamageManagerComponent dmg = DamageManagerComponent.Cast(tv.m_Entity.FindComponent(DamageManagerComponent));
			if (dmg && dmg.GetState() == EDamageState.DESTROYED)
			{
				F44_TeamLosses l;
				if (m_mLosses.Find(tv.m_sTeamId, l))
				{
					int n;
					l.m_mVehicles.Find(tv.m_sKey, n);
					l.m_mVehicles.Set(tv.m_sKey, n + 1);
				}
				RemoveTickets(tv.m_iSide, F44_Catalog.Get().GetVehicleCost(tv.m_sKey));
				PrintFormat("[F44] Vehicle %1 of %2 destroyed", tv.m_sKey, tv.m_sTeamId);
				m_aVehicles.Remove(i);
			}
		}

		// refill free slots from the pools
		float now = GetGame().GetWorld().GetWorldTime() * 0.001;
		foreach (F44_VehicleSpawnPointComponent sp : F44_VehicleSpawnPointComponent.GetAll())
		{
			if (!sp.IsFree())
			{
				sp.m_fNextSpawnTime = now + sp.m_fRespawnDelay;
				continue;
			}
			if (now < sp.m_fNextSpawnTime)
				continue;

			array<ref F44_PoolEntry> pool = m_aPoolDef;
			if (sp.m_eSide == F44_ESectorOwner.ATTACKER)
				pool = m_aPoolAtt;

			foreach (F44_PoolEntry e : pool)
			{
				if (e.m_iRemaining <= 0 || !sp.AcceptsKey(e.m_sKey))
					continue;
				ResourceName prefab = F44_Catalog.Get().GetVehiclePrefab(e.m_sKey);
				if (prefab.IsEmpty())
					continue;
				IEntity veh = sp.SpawnVehicle(prefab);
				F44_VehicleStorageLock.Apply(veh);
				if (!veh)
					continue;
				e.m_iRemaining--;
				sp.m_Vehicle = veh;
				F44_TrackedVehicle tv2 = new F44_TrackedVehicle();
				tv2.m_Entity = veh;
				tv2.m_sKey = e.m_sKey;
				tv2.m_sTeamId = e.m_sTeamId;
				tv2.m_iSide = sp.m_eSide;
				tv2.m_fSpawnTime = now;
				m_aVehicles.Insert(tv2);
				PrintFormat("[F44] Spawned %1 for %2 (%3 left)", e.m_sKey, e.m_sTeamId, e.m_iRemaining);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckEnd()
	{
		if (F44_SectorManager.Count() > 0 && F44_SectorManager.AllOwnedBy(m_aSectorOwners, F44_ESectorOwner.ATTACKER))
		{
			EndBattle(m_sAttacker, "SECTORS");
			return;
		}
		if (m_Config.GetMode() == F44_EBattleMode.SKIRMISH && F44_SectorManager.Count() > 0
			&& F44_SectorManager.AllOwnedBy(m_aSectorOwners, F44_ESectorOwner.DEFENDER))
		{
			EndBattle(m_sDefender, "SECTORS");
			return;
		}
		if (m_iTicketsAtt <= 0)
		{
			EndBattle(m_sDefender, "TICKETS");
			return;
		}
		if (m_iTicketsDef <= 0)
		{
			EndBattle(m_sAttacker, "TICKETS");
			return;
		}
		if (m_iTimeLeft <= 0)
		{
			// time over: defender wins in ASSAULT, most tickets in SKIRMISH
			string winner = m_sDefender;
			if (m_Config.GetMode() == F44_EBattleMode.SKIRMISH && m_iTicketsAtt > m_iTicketsDef)
				winner = m_sAttacker;
			EndBattle(winner, "TIME");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EndBattle(string winner, string reason)
	{
		if (m_bEnded)
			return;
		m_bEnded = true;
		m_bRunning = false;
		m_sWinner = winner;
		Replication.BumpMe();
		GetGame().GetCallqueue().Remove(ServerTick);
		PrintFormat("[F44] Battle over: winner %1 (%2)", winner, reason);

		string json = BuildResultJson(winner, reason);
		SaveResultFile(json);
		PostResult(json);

		GetGame().GetCallqueue().CallLater(FinishGameMode, m_fEndDelay * 1000, false, winner);
	}

	//------------------------------------------------------------------------------------------------
	protected void FinishGameMode(string winner)
	{
		if (!m_pGameMode)
			return;
		int factionIdx = -1;
		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			Faction f = fm.GetFactionByKey(F44_Catalog.Get().GetFactionKey(winner));
			if (f)
				factionIdx = fm.GetFactionIndex(f);
		}
		m_pGameMode.EndGameMode(SCR_GameModeEndData.CreateSimple(EGameOverTypes.ENDREASON_SCORELIMIT, -1, factionIdx));
	}

	//------------------------------------------------------------------------------------------------
	protected static string Esc(string s)
	{
		s.Replace("\\", "\\\\");
		s.Replace("\"", "\\\"");
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! BattleResult (PLAN.md section 4)
	string BuildResultJson(string winner, string reason)
	{
		string q = "\"";
		string json = "{" + q + "battleId" + q + ":" + q + Esc(m_Loader.GetBattleId()) + q;
		json += "," + q + "winner" + q + ":" + q + Esc(winner) + q;
		json += "," + q + "reason" + q + ":" + q + reason + q;
		json += "," + q + "durationSec" + q + ":" + Math.Round(m_fElapsed).ToString();

		json += "," + q + "losses" + q + ":{";
		bool first = true;
		foreach (string teamId, F44_TeamLosses l : m_mLosses)
		{
			if (!first)
				json += ",";
			first = false;
			json += q + Esc(teamId) + q + ":{" + q + "manpower" + q + ":" + l.m_iManpower.ToString() + "," + q + "vehicles" + q + ":{";
			bool firstV = true;
			foreach (string vk, int vn : l.m_mVehicles)
			{
				if (!firstV)
					json += ",";
				firstV = false;
				json += q + Esc(vk) + q + ":" + vn.ToString();
			}
			json += "}}";
		}
		json += "}";

		json += ",\"aiDeaths\":" + MapJson(m_mAIDeaths) + ",\"aiKills\":" + MapJson(m_mAIKills) + ",\"teamAiKills\":" + MapJson(m_mTeamAIKills)
			+ string.Format(",\"aiSkill\":{\"%1\":{\"final\":\"%2\",\"avg\":\"%3\"},\"%4\":{\"final\":\"%5\",\"avg\":\"%6\"}}", m_sAttacker, m_AI.GetSkill(F44_ESectorOwner.ATTACKER), m_AI.GetAvgSkill(F44_ESectorOwner.ATTACKER), m_sDefender, m_AI.GetSkill(F44_ESectorOwner.DEFENDER), m_AI.GetAvgSkill(F44_ESectorOwner.DEFENDER));
		json += "," + q + "players" + q + ":[";
		first = true;
		foreach (int pid, F44_PlayerStats st : m_mStats)
		{
			if (st.m_sSide.IsEmpty())
				continue;
			if (!first)
				json += ",";
			first = false;
			int xp = st.m_iKills * 30 + st.m_iCaptures * 50 + 20;
			json += "{" + q + "identityId" + q + ":" + q + Esc(st.m_sIdentityId) + q;
			json += "," + q + "name" + q + ":" + q + Esc(st.m_sName) + q;
			json += "," + q + "side" + q + ":" + q + Esc(st.m_sSide) + q;
			json += "," + q + "teamId" + q + ":" + q + Esc(st.m_sTeamId) + q;
			json += "," + q + "kills" + q + ":" + st.m_iKills.ToString();
			json += "," + q + "deaths" + q + ":" + st.m_iDeaths.ToString();
			json += "," + q + "captures" + q + ":" + st.m_iCaptures.ToString();
			json += "," + q + "resupplyGiven" + q + ":" + st.m_iResupplyGiven.ToString();
			json += "," + q + "boxHealGiven" + q + ":" + st.m_iHealGiven.ToString();	// field supply med box (F44_FieldSupply.c), distinct from medical healGiven
			json += "," + q + "killsByCategory" + q + ":{";
			bool firstC = true;
			foreach (string ck, int cv : st.m_mKillsByCategory)
			{
				if (!firstC)
					json += ",";
				firstC = false;
				json += q + Esc(ck) + q + ":" + cv.ToString();
			}
			json += "}";
			json += F44_Medical.ResultJson(pid);	// healGiven, revives, stats (F44_Medical.c)
			json += "," + q + "xp" + q + ":" + xp.ToString() + "}";
		}
		json += "]}";
		return json;
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveResultFile(string json)
	{
		FileIO.MakeDirectory("$profile:F44");
		string path = "$profile:F44/result_" + m_Loader.GetBattleId() + ".json";
		FileHandle fh = FileIO.OpenFile(path, FileMode.WRITE);
		if (!fh)
			return;
		fh.WriteLine(json);
		fh.Close();
		Print("[F44] Result saved to " + path);
	}

	//------------------------------------------------------------------------------------------------
	protected void PostResult(string json)
	{
		if (m_Loader.GetApiUrl().IsEmpty() || m_Loader.GetBattleId().IsEmpty())
			return;
		m_sResultJson = json;
		m_iResultAttempt = 0;
		SendResult();
	}

	// RestContext exposes no timeout setter; a slow site can make the engine report
	// http 0 although the result was saved. We retry with backoff and the site answers
	// 409 (battle already finished) on a duplicate, which counts as success.
	protected void SendResult()
	{
		m_iResultAttempt++;
		RestContext ctx = F44_BattleLoader.GetRestContext(m_Loader.GetApiUrl(), m_Loader.GetToken());
		if (!ctx)
			return;
		m_ResultCallback = new RestCallback();
		m_ResultCallback.SetOnSuccess(OnResultPosted);
		m_ResultCallback.SetOnError(OnResultError);
		ctx.POST(m_ResultCallback, "/api/v1/battles/" + m_Loader.GetBattleId() + "/result", m_sResultJson);
	}

	protected void OnResultPosted(RestCallback cb)
	{
		PrintFormat("[F44] BattleResult posted (attempt %1)", m_iResultAttempt);
	}

	protected void OnResultError(RestCallback cb)
	{
		int http = cb.GetHttpCode();
		if (http == 409)
		{
			Print("[F44] BattleResult already recorded by site (http 409), OK");
			return;
		}
		if (m_iResultAttempt < RESULT_MAX_ATTEMPTS)
		{
			int delayMs = 5000 * m_iResultAttempt;
			PrintFormat("[F44] BattleResult POST attempt %1 failed (rest %2, http %3), retry in %4 ms", m_iResultAttempt, cb.GetRestResult(), http, delayMs, level: LogLevel.WARNING);
			GetGame().GetCallqueue().CallLater(SendResult, delayMs, false);
			return;
		}
		PrintFormat("[F44] BattleResult POST failed (rest %1, http %2), kept in $profile:F44", cb.GetRestResult(), http, level: LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	// ---- client side ----
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (!m_Hud)
		{
			if (m_sAttacker.IsEmpty())
				return;
			m_Hud = new F44_Hud();
			if (!m_Hud.Create(m_sHudLayout))
				return;
		}
		m_Hud.Update(this, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	// ---- getters for HUD ----
	int GetTicketsAttacker() { return m_iTicketsAtt; }
	int GetTicketsDefender() { return m_iTicketsDef; }
	int GetTimeLeft() { return m_iTimeLeft; }
	string GetAttacker() { return m_sAttacker; }
	string GetDefender() { return m_sDefender; }
	string GetWinner() { return m_sWinner; }
	array<int> GetSectorOwners() { return m_aSectorOwners; }
	array<ref F44_TrackedVehicle> GetTrackedVehicles() { return m_aVehicles; }
	array<int> GetSectorProgress() { return m_aSectorProgress; }
	string GetAttackerName() { return m_sAttName; }
	string GetDefenderName() { return m_sDefName; }
	int GetTicketsAttackerMax() { return m_iTicketsAttMax; }
	int GetTicketsDefenderMax() { return m_iTicketsDefMax; }
	int GetAIAttacker() { return m_iAIAtt; }
	int GetAIDefender() { return m_iAIDef; }
	int GetPlayersAttacker() { return m_iPlayersAtt; }
	int GetPlayersDefender() { return m_iPlayersDef; }
}
