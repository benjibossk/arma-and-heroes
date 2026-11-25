// Arma & Heroes - AI reinforcements (server only).
// Each side tops up its strength with AI groups built from its Assault Teams:
//   aliveTarget = max(0, aiMaxAlive - playersOnSide * aiPerPlayerScale), capped by the global aiLimit.
// Groups spawn on the side's spawn points, use the team classes (catalog) and get waypoints
// re-evaluated every 30 s: attackers -> active sector (ASSAULT) or nearest non-owned sector (SKIRMISH),
// defenders -> defend the threatened / owned sectors. Bots capture sectors (F44_SectorManager.Simulate).
// Bot deaths cost 1 ticket and count in the team manpower losses (handled by the game mode).
// Vehicles (1.1.0): mounted groups follow a chain of road-snapped move waypoints (small completion radius,
// cruise speed set on the vehicle AI). Transports unload ~150 m before the sector, then the driver alone
// takes the vehicle back to its spawn where it is freed for the next group. Armor drives to the sector
// edge facing the attackers and supports from there (defend waypoint at the edge).

//------------------------------------------------------------------------------------------------
class F44_AIGroupInfo
{
	SCR_AIGroup m_Group;
	int m_iSide;
	string m_sTeamId;
	string m_sSideKey;
	int m_iTargetSector = -1;
	AIWaypoint m_Waypoint;	// last waypoint of the current order
	ref array<AIWaypoint> m_aWaypoints = {};
	// vehicle use
	F44_TrackedVehicle m_Vehicle;
	bool m_bTank;
	bool m_bDismounted;
	bool m_bBoarded;	// crew seated (Board ran)
	bool m_bReturning;	// driver bringing the transport back to base
	bool m_bInPosition;	// tank reached its support position
	vector m_vHome;	// spawn position of the group
	vector m_vDest;	// current mounted destination
	// movement telemetry
	vector m_vLastPos;
	float m_fLogAccu;
	float m_fStillTime;
	int m_iStuck;	// re-routes without progress
	float m_fBestDest = float.MAX;	// closest distance to the destination reached so far
}

//------------------------------------------------------------------------------------------------
class F44_AIDirector
{
	static const ResourceName WP_ATTACK = "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
	static const ResourceName WP_MOVE = "{750A8D1695BD6998}Prefabs/AI/Waypoints/AIWaypoint_Move.et";
	static const ResourceName WP_FORCEDMOVE = "{06E1B6EBD480C6E0}Prefabs/AI/Waypoints/AIWaypoint_ForcedMove.et";
	static const ResourceName WP_DEFEND = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
	static const int GROUP_SIZE = 4;
	static const float UNLOAD_DIST = 150;	// transports unload this far from the sector center
	static const float ARMOR_EDGE = 60;	// armor holds this far beyond the capture radius
	static const float CRUISE_TRANSPORT = 45;	// km/h
	static const float CRUISE_TANK = 28;
	static const float VEH_LOG_PERIOD = 10;	// s

	protected F44_GameModeComponent m_GameMode;
	protected F44_EBattleMode m_eMode;
	protected int m_iPlayersAtt, m_iPlayersDef;
	protected ref array<ref F44_AIGroupInfo> m_aGroups = {};
	// living bot -> (side, teamId)
	protected ref map<IEntity, int> m_mBotSide = new map<IEntity, int>();
	protected ref map<int, int> m_mSkill = new map<int, int>();	// side -> effective skill level 1..4
	protected ref map<int, int> m_mSkillSum = new map<int, int>();	// side -> sum of levels at spawns (average)
	protected ref map<int, int> m_mSkillN = new map<int, int>();
	static const ref array<string> SKILLS = {"", "Recruit", "Regular", "Veteran", "Elite"};

	//! Dynamic skill: base preset, +1 if outnumbered by enemy players (+2 if by 3+), -1 if own side has 3+ more players,
	//! +1 if enemy players' average rank >= Sergeant (+2 >= Second Lieutenant); clamped to [min..max].
	protected void UpdateSkill(F44_Side side, int sideId, int ownPlayers, int enemyPlayers, float enemyRank)
	{
		int lvl = Math.Max(1, SKILLS.Find(side.m_sAISkill));
		int diff = enemyPlayers - ownPlayers;
		if (diff >= 3)
			lvl += 2;
		else if (diff >= 1)
			lvl += 1;
		else if (diff <= -3)
			lvl -= 1;
		if (enemyRank >= 5)
			lvl += 2;
		else if (enemyRank >= 2)
			lvl += 1;
		int lo = Math.Max(1, SKILLS.Find(side.m_sAISkillMin));
		int hi = SKILLS.Find(side.m_sAISkillMax);
		if (hi < 1)
			hi = 4;
		lvl = Math.ClampInt(lvl, lo, hi);
		int cur;
		if (m_mSkill.Find(sideId, cur) && cur == lvl)
			return;
		m_mSkill.Set(sideId, lvl);
		PrintFormat("[F44] AI skill side=%1 players=%2 enemyPlayers=%3 avgEnemyRank=%4 -> %5", side.m_sSideKey, ownPlayers, enemyPlayers, enemyRank, SKILLS[lvl]);
		EAISkill sk = SkillFromName(SKILLS[lvl]);
		foreach (IEntity e, int s : m_mBotSide)
		{
			if (!e || s != sideId)
				continue;
			SCR_AICombatComponent c = SCR_AICombatComponent.Cast(e.FindComponent(SCR_AICombatComponent));
			if (c)
				c.SetAISkill(sk);
		}
	}

	string GetSkill(int sideId)
	{
		int l;
		m_mSkill.Find(sideId, l);
		return SKILLS[Math.ClampInt(l, 0, 4)];
	}

	string GetAvgSkill(int sideId)
	{
		int s, n;
		m_mSkillSum.Find(sideId, s);
		m_mSkillN.Find(sideId, n);
		if (n == 0)
			return GetSkill(sideId);
		return SKILLS[Math.ClampInt(Math.Round(s / (float)n), 1, 4)];
	}
	protected ref map<IEntity, string> m_mBotTeam = new map<IEntity, string>();
	protected int m_iLimit = 48;
	protected float m_fRetargetAccu = 1000;
	protected int m_iTeamCursorAtt;
	protected int m_iTeamCursorDef;
	protected bool m_bTickLogged;
	protected ref array<ref F44_AIGroupInfo> m_aPending = {};	// groups created during the vehicle update (drivers)
	protected vector m_vAttackerRef;	// attacker spawn: armor holds the sector edge facing it

	protected F44_Team FindTeam(F44_BattleConfig cfg, F44_AIGroupInfo gi)
	{
		F44_Side side = cfg.m_Defender;
		if (gi.m_iSide == F44_ESectorOwner.ATTACKER)
			side = cfg.m_Attacker;
		foreach (F44_Team t : side.m_aTeams)
		{
			if (t.m_sTeamId == gi.m_sTeamId)
				return t;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	void Init(F44_GameModeComponent gm, int limit)
	{
		m_GameMode = gm;
		m_iLimit = limit;
	}

	//------------------------------------------------------------------------------------------------
	//! Is this entity one of our bots? Returns side through out param
	bool IsBot(IEntity ent, out int side, out string teamId)
	{
		if (!ent || !m_mBotSide.Find(ent, side))
			return false;
		m_mBotTeam.Find(ent, teamId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnBotKilled(IEntity ent)
	{
		m_mBotSide.Remove(ent);
		m_mBotTeam.Remove(ent);
	}

	//------------------------------------------------------------------------------------------------
	void GetBots(int side, notnull array<IEntity> outBots)
	{
		foreach (IEntity e, int s : m_mBotSide)
		{
			if (s == side && e)
				outBots.Insert(e);
		}
	}

	//------------------------------------------------------------------------------------------------
	int CountAlive(int side)
	{
		int n;
		foreach (IEntity e, int s : m_mBotSide)
		{
			if (e && (side < 0 || s == side) && F44_SectorManager.IsAlive(e))
				n++;
		}
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Called by the game mode tick (dt seconds)
	void Tick(float dt, F44_BattleConfig cfg, int playersAtt, int playersDef, int ticketsAtt, int ticketsDef, notnull array<int> owners)
	{
		Cleanup();
		if (!m_bTickLogged)
		{
			m_bTickLogged = true;
			PrintFormat("[F44] AI director running (limit %1, targets %2/%3)", m_iLimit, cfg.m_Attacker.GetAITarget(playersAtt), cfg.m_Defender.GetAITarget(playersDef));
		}

		UpdateSkill(cfg.m_Attacker, F44_ESectorOwner.ATTACKER, playersAtt, playersDef, m_GameMode.AvgPlayerRank(F44_ESectorOwner.DEFENDER));
		UpdateSkill(cfg.m_Defender, F44_ESectorOwner.DEFENDER, playersDef, playersAtt, m_GameMode.AvgPlayerRank(F44_ESectorOwner.ATTACKER));
		// spawn at most one group per side per tick (keeps frame cost low)
		int globalAlive = CountAlive(-1);
		TrySpawnSide(cfg, cfg.m_Attacker, F44_ESectorOwner.ATTACKER, playersAtt, ticketsAtt, globalAlive);
		globalAlive = CountAlive(-1);
		TrySpawnSide(cfg, cfg.m_Defender, F44_ESectorOwner.DEFENDER, playersDef, ticketsDef, globalAlive);

		m_eMode = cfg.GetMode();
		m_iPlayersAtt = playersAtt;
		m_iPlayersDef = playersDef;
		if (m_vAttackerRef == vector.Zero)
		{
			array<SCR_SpawnPoint> ap = SCR_SpawnPoint.GetSpawnPointsForFaction(F44_Catalog.Get().GetFactionKey(cfg.m_sAttacker));
			if (ap && !ap.IsEmpty())
				m_vAttackerRef = ap[0].GetOrigin();
		}
		foreach (F44_AIGroupInfo vg : m_aGroups)
			UpdateVehicle(vg, dt);
		foreach (F44_AIGroupInfo pg : m_aPending)
			m_aGroups.Insert(pg);
		m_aPending.Clear();

		m_fRetargetAccu += dt;
		if (m_fRetargetAccu >= 30)
		{
			m_fRetargetAccu = 0;
			foreach (F44_AIGroupInfo gi : m_aGroups)
			{
				// groups still at their spawn pick up vehicles that respawned meanwhile
				if (!gi.m_Vehicle && !gi.m_bDismounted && gi.m_Group && vector.DistanceXZ(GroupPos(gi), gi.m_vHome) < 150)
				{
					F44_Team gt = FindTeam(cfg, gi);
					if (gt && TryAssignVehicle(gi, gt, gi.m_vHome))
						continue;
				}
				Retarget(gi, cfg.GetMode(), owners, true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Cleanup()
	{
		array<IEntity> dead = {};
		foreach (IEntity e, int s : m_mBotSide)
		{
			if (!e || !F44_SectorManager.IsAlive(e))
				dead.Insert(e);
		}
		foreach (IEntity d : dead)
		{
			m_mBotSide.Remove(d);
			m_mBotTeam.Remove(d);
		}
		for (int i = m_aGroups.Count() - 1; i >= 0; i--)
		{
			SCR_AIGroup g = m_aGroups[i].m_Group;
			if (!g || g.GetAgentsCount() == 0)
			{
				if (g)
					SCR_EntityHelper.DeleteEntityAndChildren(g);
				m_aGroups.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TrySpawnSide(F44_BattleConfig cfg, F44_Side side, int sideId, int players, int tickets, int globalAlive)
	{
		if (!side || tickets <= 0)
			return;
		int target = side.GetAITarget(players);
		int alive = CountAlive(sideId);
		int room = Math.Min(target - alive, m_iLimit - globalAlive);
		// a bot needs a ticket too
		room = Math.Min(room, tickets - alive);
		if (room <= 0)
			return;

		F44_Team team = PickTeam(side, sideId);
		if (!team)
			return;
		SpawnGroup(cfg, side, team, sideId, Math.Min(room, GROUP_SIZE));
	}

	//------------------------------------------------------------------------------------------------
	protected F44_Team PickTeam(F44_Side side, int sideId)
	{
		int n = side.m_aTeams.Count();
		if (n == 0)
			return null;
		int cursor = m_iTeamCursorDef;
		if (sideId == F44_ESectorOwner.ATTACKER)
			cursor = m_iTeamCursorAtt;
		for (int i = 0; i < n; i++)
		{
			F44_Team t = side.m_aTeams[(cursor + i) % n];
			if (!t.m_aClasses.IsEmpty())
			{
				if (sideId == F44_ESectorOwner.ATTACKER)
					m_iTeamCursorAtt = cursor + i + 1;
				else
					m_iTeamCursorDef = cursor + i + 1;
				return t;
			}
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool FindSpawnPosition(string factionKey, out vector pos)
	{
		array<SCR_SpawnPoint> points = SCR_SpawnPoint.GetSpawnPointsForFaction(factionKey);
		if (!points || points.IsEmpty())
			return false;
		SCR_SpawnPoint sp = points.GetRandomElement();
		vector p = sp.GetOrigin();
		p[0] = p[0] + Math.RandomFloat(-15, 15);
		p[2] = p[2] + Math.RandomFloat(-15, 15);
		p[1] = GetGame().GetWorld().GetSurfaceY(p[0], p[2]);
		pos = p;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnGroup(F44_BattleConfig cfg, F44_Side side, F44_Team team, int sideId, int size)
	{
		F44_Catalog cat = F44_Catalog.Get();
		string factionKey = cat.GetFactionKey(side.m_sSideKey);
		vector pos;
		if (!FindSpawnPosition(factionKey, pos))
		{
			Print("[F44] AI: no spawn point for faction " + factionKey, LogLevel.WARNING);
			return;
		}

		// group prefab: catalog key <SIDE>_AIGROUP (empty faction group, e.g. FH group base)
		ResourceName groupPrefab;
		F44_CatalogEntry ge = cat.GetEntry(side.m_sSideKey + "_AIGROUP");
		if (ge)
			groupPrefab = ge.m_sPrefab;
		if (groupPrefab.IsEmpty())
		{
			Print("[F44] AI: no " + side.m_sSideKey + "_AIGROUP prefab in catalog, AI disabled for this side", LogLevel.WARNING);
			side.m_bAIEnabled = false;
			return;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = pos;
		Resource gres = Resource.Load(groupPrefab);
		if (!gres || !gres.IsValid())
			return;
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(gres, GetGame().GetWorld(), params));
		if (!group)
			return;
		group.SetFaction(GetGame().GetFactionManager().GetFactionByKey(factionKey));

		// members: cycle through the classes allowed by the team
		int spawned;
		for (int i = 0; i < size; i++)
		{
			string classKey = team.m_aClasses[i % team.m_aClasses.Count()];
			ResourceName prefab = cat.GetClassPrefab(F44_Catalog.MakeClassKey(side.m_sSideKey, classKey));
			if (prefab.IsEmpty())
				prefab = cat.GetClassPrefab(F44_Catalog.MakeClassKey(side.m_sSideKey, team.m_aClasses[0]));
			if (prefab.IsEmpty())
				continue;
			Resource cres = Resource.Load(prefab);
			if (!cres || !cres.IsValid())
				continue;
			EntitySpawnParams cp = new EntitySpawnParams();
			cp.TransformMode = ETransformMode.WORLD;
			vector cpos = pos;
			cpos[0] = cpos[0] + i * 1.5;
			cpos[1] = GetGame().GetWorld().GetSurfaceY(cpos[0], cpos[2]);
			cp.Transform[3] = cpos;
			IEntity ch = GetGame().SpawnEntityPrefab(cres, GetGame().GetWorld(), cp);
			if (!ch)
				continue;
			AIControlComponent ctrl = AIControlComponent.Cast(ch.FindComponent(AIControlComponent));
			if (ctrl)
				ctrl.ActivateAI();
			SCR_AICombatComponent combat = SCR_AICombatComponent.Cast(ch.FindComponent(SCR_AICombatComponent));
			if (combat)
				combat.SetAISkill(SkillFromName(GetSkill(sideId)));
			ApplyAppearance(ch, side.m_sSideKey, GetSkill(sideId), team);
			group.AddAIEntityToGroup(ch);
			m_mBotSide.Set(ch, sideId);
			m_mBotTeam.Set(ch, team.m_sTeamId);
			spawned++;
		}
		if (spawned == 0)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(group);
			return;
		}

		F44_AIGroupInfo gi = new F44_AIGroupInfo();
		gi.m_Group = group;
		gi.m_iSide = sideId;
		gi.m_sTeamId = team.m_sTeamId;
		gi.m_sSideKey = side.m_sSideKey;
		gi.m_vHome = pos;
		m_aGroups.Insert(gi);
		Retarget(gi, cfg.GetMode(), m_GameMode.GetSectorOwners(), false);
		TryAssignVehicle(gi, team, pos);
		MapAddI(m_mSkillSum, sideId, SKILLS.Find(GetSkill(sideId)));
		MapAddI(m_mSkillN, sideId, 1);
		PrintFormat("[F44] AI: %1 group of %2 spawned (team %3 \"%4\", skill %5)", side.m_sSideKey, spawned, team.m_sTeamId, team.m_sDisplayName, GetSkill(sideId));
	}

	//------------------------------------------------------------------------------------------------
	//! swap the bot's clothing slots for the tier/team-type loadout of the catalog "aiAppearance"
	protected void ApplyAppearance(IEntity ch, string faction, string tier, F44_Team team)
	{
		array<string> items = F44_Catalog.Get().GetAppearance(faction, tier, team.m_sType);
		if (!items)
			return;
		EquipedLoadoutStorageComponent storage = EquipedLoadoutStorageComponent.Cast(ch.FindComponent(EquipedLoadoutStorageComponent));
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(ch.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!storage || !inv)
			return;
		int ok;
		foreach (string prefab : items)
		{
			Resource res = Resource.Load(prefab);
			if (!res || !res.IsValid())
				continue;
			EntitySpawnParams sp = new EntitySpawnParams();
			sp.Transform[3] = ch.GetOrigin();
			IEntity item = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), sp);
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
			if (inv.TryInsertItemInStorage(item, storage))
				ok++;
			else
				SCR_EntityHelper.DeleteEntityAndChildren(item);
		}
		if (!m_mAppearLogged.Contains(faction + tier + team.m_sTeamId))
		{
			m_mAppearLogged.Insert(faction + tier + team.m_sTeamId);
			PrintFormat("[F44] AI appearance tier=%1 team=%2 (%3, %4/%5 items)", tier, team.m_sTeamId, team.m_sType, ok, items.Count());
		}
	}
	protected ref set<string> m_mAppearLogged = new set<string>();

	protected static void MapAddI(map<int, int> m, int k, int v)
	{
		int c;
		m.Find(k, c);
		m.Set(k, c + v);
	}

	static EAISkill SkillFromName(string s)
	{
		switch (s)
		{
			case "Recruit": return EAISkill.ROOKIE;
			case "Veteran": return EAISkill.VETERAN;
			case "Elite": return EAISkill.EXPERT;
		}
		return EAISkill.REGULAR;
	}

	//------------------------------------------------------------------------------------------------
	protected static vector GroupPos(F44_AIGroupInfo gi)
	{
		if (gi.m_Vehicle && gi.m_Vehicle.m_Entity && !gi.m_bDismounted)
			return gi.m_Vehicle.m_Entity.GetOrigin();
		IEntity leader = gi.m_Group.GetLeaderEntity();
		if (leader)
			return leader.GetOrigin();
		return gi.m_Group.GetOrigin();
	}

	protected static bool IsMounted(F44_AIGroupInfo gi)
	{
		return gi.m_Vehicle && gi.m_Vehicle.m_Entity && !gi.m_bDismounted;
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearWaypoints(F44_AIGroupInfo gi)
	{
		foreach (AIWaypoint w : gi.m_aWaypoints)
		{
			if (!w)
				continue;
			if (gi.m_Group)
				gi.m_Group.RemoveWaypoint(w);
			SCR_EntityHelper.DeleteEntityAndChildren(w);
		}
		gi.m_aWaypoints.Clear();
		gi.m_Waypoint = null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool AddWp(F44_AIGroupInfo gi, ResourceName prefab, vector pos, float radius)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			res = Resource.Load(WP_MOVE);
		if (!res || !res.IsValid())
			return false;
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		params.Transform[3] = pos;
		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
		if (!wp)
			return false;
		wp.SetCompletionRadius(radius);
		gi.m_Group.AddWaypoint(wp);
		gi.m_aWaypoints.Insert(wp);
		gi.m_Waypoint = wp;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! chain of road-snapped move waypoints from -> to (vehicles)
	protected int AddRoute(F44_AIGroupInfo gi, vector from, vector to, ResourceName prefab, float lastRadius)
	{
		array<vector> route = F44_Roads.BuildRoute(from, to);
		foreach (int i, vector pt : route)
		{
			float r = 20;
			if (i == route.Count() - 1)
				r = lastRadius;
			AddWp(gi, prefab, pt, r);
		}
		return route.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! armor support position: sector edge facing the attackers
	protected vector EdgePoint(F44_SectorComponent sector, vector from)
	{
		vector c = sector.GetOwner().GetOrigin();
		vector refPos = m_vAttackerRef;
		if (vector.DistanceXZ(refPos, c) < 50)
			refPos = from;
		vector dir = refPos - c;
		dir[1] = 0;
		if (dir.Length() < 1)
			dir = "1 0 0";
		dir.Normalize();
		vector p = c + dir * (sector.m_fRadius + ARMOR_EDGE);
		vector rp, rdir;
		if (F44_Roads.Snap(p, 40, rp, rdir))
			p = rp;
		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! transport unload point: on the approach line, UNLOAD_DIST before the sector center (road if close)
	protected vector UnloadPoint(F44_SectorComponent sector, vector from)
	{
		vector c = sector.GetOwner().GetOrigin();
		vector dir = from - c;
		dir[1] = 0;
		if (dir.Length() < UNLOAD_DIST + 20)
			return from;
		dir.Normalize();
		vector p = c + dir * UNLOAD_DIST;
		vector rp, rdir;
		if (F44_Roads.Snap(p, 50, rp, rdir))
			p = rp;
		return p;
	}

	//------------------------------------------------------------------------------------------------
	protected void Retarget(F44_AIGroupInfo gi, F44_EBattleMode mode, array<int> owners, bool onlyIfChanged)
	{
		if (!gi.m_Group || gi.m_Group.GetAgentsCount() == 0 || gi.m_bReturning)
			return;
		vector from = GroupPos(gi);

		int target = ChooseSector(gi.m_iSide, mode, owners, from);
		if (target < 0)
			return;
		if (onlyIfChanged && target == gi.m_iTargetSector && gi.m_Waypoint)
			return;

		F44_SectorComponent sector = F44_SectorManager.GetSector(target);
		if (!sector)
			return;

		// attackers seize, defenders hold what they own; a defender sent to a lost sector counter-attacks
		bool ownSector = (gi.m_iSide == F44_ESectorOwner.DEFENDER && owners[target] == F44_ESectorOwner.DEFENDER)
			|| (gi.m_iSide == F44_ESectorOwner.ATTACKER && owners[target] == F44_ESectorOwner.ATTACKER);
		vector center = sector.GetOwner().GetOrigin();
		ClearWaypoints(gi);
		if (target != gi.m_iTargetSector)
		{
			gi.m_iStuck = 0;
			gi.m_fBestDest = float.MAX;
		}
		gi.m_iTargetSector = target;
		string wpKind;
		if (IsMounted(gi))
		{
			int n;
			gi.m_bInPosition = false;
			if (gi.m_bTank)
			{
				gi.m_vDest = EdgePoint(sector, from);
				n = AddRoute(gi, from, gi.m_vDest, WP_MOVE, 25);
				wpKind = "armor to edge of";
			}
			else
			{
				gi.m_vDest = UnloadPoint(sector, from);
				n = AddRoute(gi, from, gi.m_vDest, WP_FORCEDMOVE, 25);
				wpKind = "transport to unload point of";
			}
			PrintFormat("[F44] AI: side %1 group (team %2) -> %3 sector %4 (%5 road waypoints, dest %6 m from center)", gi.m_iSide, gi.m_sTeamId, wpKind, sector.m_sSectorName, n, Math.Round(vector.DistanceXZ(gi.m_vDest, center)));
			return;
		}

		ResourceName wpPrefab = WP_ATTACK;
		wpKind = "attack";
		if (ownSector)
		{
			wpPrefab = WP_DEFEND;
			wpKind = "defend";
		}
		AddWp(gi, wpPrefab, center, sector.m_fRadius * 0.8);
		PrintFormat("[F44] AI: side %1 group (team %2) -> %3 sector %4", gi.m_iSide, gi.m_sTeamId, wpKind, sector.m_sSectorName);
	}

	//------------------------------------------------------------------------------------------------
	protected int ChooseSector(int side, F44_EBattleMode mode, array<int> owners, vector from)
	{
		int n = F44_SectorManager.Count();
		if (n == 0)
			return -1;

		// ASSAULT: everybody fights for the active sector(s): the first active one in order
		if (mode == F44_EBattleMode.ASSAULT)
		{
			for (int i = 0; i < n; i++)
			{
				if (F44_SectorManager.IsActive(i, mode, owners))
					return i;
			}
			return -1;
		}

		// SKIRMISH: attackers and defenders alike go to the nearest sector they do not own,
		// otherwise defend the nearest owned one
		int best = -1;
		float bestDist = float.MAX;
		for (int i = 0; i < n; i++)
		{
			if (owners[i] == side)
				continue;
			F44_SectorComponent s = F44_SectorManager.GetSector(i);
			float d = vector.DistanceSq(from, s.GetOwner().GetOrigin());
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}
		if (best >= 0)
			return best;
		for (int i = 0; i < n; i++)
		{
			F44_SectorComponent s2 = F44_SectorManager.GetSector(i);
			float d2 = vector.DistanceSq(from, s2.GetOwner().GetOrigin());
			if (d2 < bestDist)
			{
				bestDist = d2;
				best = i;
			}
		}
		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Vehicles: a group near its spawn takes a free pool vehicle of its side parked there
	//! (empty, unclaimed, parked for 30 s so players get first pick). Armor teams crew tanks, the others ride transports.
	protected bool TryAssignVehicle(F44_AIGroupInfo gi, F44_Team team, vector pos)
	{
		bool wantTank = team.m_sType == "ARM";
		return TryAssignVehicleType(gi, wantTank, pos);
	}

	protected bool TryAssignVehicleType(F44_AIGroupInfo gi, bool wantTank, vector pos)
	{
		if (gi.m_Vehicle || gi.m_Group.GetAgentsCount() < 2)
			return false;	// a lone bot does not take a vehicle
		// no transport ride when the spawn is already within walking distance of a sector
		if (!wantTank)
		{
			for (int si = 0; si < F44_SectorManager.Count(); si++)
			{
				if (vector.DistanceXZ(pos, F44_SectorManager.GetSector(si).GetOwner().GetOrigin()) < UNLOAD_DIST + 80)
					return false;
			}
		}
		float now = GetGame().GetWorld().GetWorldTime() * 0.001;
		int sidePlayers = m_iPlayersDef;
		if (gi.m_iSide == F44_ESectorOwner.ATTACKER)
			sidePlayers = m_iPlayersAtt;
		F44_Catalog cat = F44_Catalog.Get();
		foreach (F44_TrackedVehicle tv : m_GameMode.GetTrackedVehicles())
		{
			if (!tv.m_Entity || tv.m_AIGroup || tv.m_iSide != gi.m_iSide)
				continue;
			// players of the side get a 30 s first pick; bots-only sides take vehicles at once
			if (sidePlayers > 0 && now - tv.m_fSpawnTime < 30)
				continue;
			F44_CatalogEntry e = cat.GetEntry(tv.m_sKey);
			if (!e || (wantTank && !e.IsTank()) || (!wantTank && !e.IsTransport()))
				continue;
			if (vector.Distance(tv.m_Entity.GetOrigin(), pos) > 250 || !IsVehicleEmpty(tv.m_Entity))
				continue;
			DamageManagerComponent dmg = DamageManagerComponent.Cast(tv.m_Entity.FindComponent(DamageManagerComponent));
			if (dmg && dmg.GetState() == EDamageState.DESTROYED)
				continue;
			tv.m_AIGroup = gi.m_Group;
			gi.m_Vehicle = tv;
			gi.m_bTank = e.IsTank();
			// let the characters finish their init before moving them in
			GetGame().GetCallqueue().CallLater(Board, 2000, false, gi);
			return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsVehicleEmpty(IEntity veh)
	{
		BaseCompartmentManagerComponent mgr = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		if (!mgr)
			return false;
		array<BaseCompartmentSlot> slots = {};
		mgr.GetCompartments(slots);
		foreach (BaseCompartmentSlot s : slots)
		{
			if (s.GetOccupant())
				return false;
		}
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static IEntity FindDriver(IEntity veh)
	{
		BaseCompartmentManagerComponent mgr = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		if (!mgr)
			return null;
		array<BaseCompartmentSlot> slots = {};
		mgr.GetCompartments(slots);
		foreach (BaseCompartmentSlot s : slots)
		{
			if (PilotCompartmentSlot.Cast(s) && s.GetOccupant())
				return s.GetOccupant();
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	static array<IEntity> GroupMembers(SCR_AIGroup g)
	{
		array<IEntity> res = {};
		array<AIAgent> agents = {};
		g.GetAgents(agents);
		foreach (AIAgent a : agents)
		{
			if (a && a.GetControlledEntity())
				res.Insert(a.GetControlledEntity());
		}
		return res;
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetUsable(SCR_AIGroup g, IEntity veh, bool usable)
	{
		SCR_AIGroupUtilityComponent util = SCR_AIGroupUtilityComponent.Cast(g.FindComponent(SCR_AIGroupUtilityComponent));
		SCR_AIVehicleUsageComponent usage = SCR_AIVehicleUsageComponent.FindOnNearestParent(veh, veh);
		if (!util || !usage)
			return;
		if (usable)
			util.AddUsableVehicle(usage);
		else
			util.RemoveUsableVehicle(usage);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCruise(IEntity veh, bool tank)
	{
		AICarMovementComponent car = AICarMovementComponent.Cast(veh.FindComponent(AICarMovementComponent));
		if (!car)
			return;
		if (tank)
			car.SetCruiseSpeed(CRUISE_TANK);
		else
			car.SetCruiseSpeed(CRUISE_TRANSPORT);
	}

	//------------------------------------------------------------------------------------------------
	protected void Board(F44_AIGroupInfo gi)
	{
		if (!gi || !gi.m_Group || !gi.m_Vehicle || !gi.m_Vehicle.m_Entity)
			return;
		IEntity veh = gi.m_Vehicle.m_Entity;
		array<IEntity> members = GroupMembers(gi.m_Group);
		int seated;
		foreach (int i, IEntity ch : members)
		{
			SCR_CompartmentAccessComponent ca = SCR_CompartmentAccessComponent.Cast(ch.FindComponent(SCR_CompartmentAccessComponent));
			if (!ca)
				continue;
			ECompartmentType seat = ECompartmentType.CARGO;
			if (i == 0)
				seat = ECompartmentType.PILOT;
			else if (gi.m_bTank || i == 1)
				seat = ECompartmentType.TURRET;
			bool ok = ca.MoveInVehicle(veh, seat, true);
			if (!ok)
				ok = ca.MoveInVehicleAny(veh);
			if (ok)
				seated++;
		}
		SetUsable(gi.m_Group, veh, true);
		bool cruise = veh.FindComponent(AICarMovementComponent) != null;
		SetCruise(veh, gi.m_bTank);
		gi.m_vLastPos = veh.GetOrigin();
		gi.m_fLogAccu = 0;
		string kind = "transport";
		if (gi.m_bTank)
			kind = "crew";
		PrintFormat("[F44] AI: side %1 group (team %2) boards %3 as %4 (%5/%6 seated, cruise speed set %7)", gi.m_iSide, gi.m_sTeamId, gi.m_Vehicle.m_sKey, kind, seated, members.Count(), cruise);
		if (seated == 0)
		{
			gi.m_Vehicle.m_AIGroup = null;
			gi.m_Vehicle = null;
			return;
		}
		gi.m_bBoarded = true;
		// re-issue the sector order now that the group is mounted (road route)
		ClearWaypoints(gi);
		Retarget(gi, m_eMode, m_GameMode.GetSectorOwners(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! per tick: telemetry, stuck detection, unload / support position / return to base
	protected void UpdateVehicle(F44_AIGroupInfo gi, float dt)
	{
		if (!gi.m_Vehicle || !gi.m_Group || !gi.m_bBoarded || (gi.m_bDismounted && !gi.m_bReturning))
			return;
		IEntity veh = gi.m_Vehicle.m_Entity;
		if (!veh)
		{
			gi.m_Vehicle = null;
			gi.m_bReturning = false;
			gi.m_bDismounted = true;
			return;
		}
		vector p = veh.GetOrigin();
		F44_SectorComponent sector = F44_SectorManager.GetSector(gi.m_iTargetSector);
		float toSector = -1;
		if (sector)
			toSector = vector.DistanceXZ(p, sector.GetOwner().GetOrigin());
		vector dest = gi.m_vDest;
		if (gi.m_bReturning)
			dest = gi.m_vHome;
		float toDest = vector.DistanceXZ(p, dest);

		gi.m_fLogAccu += dt;
		if (gi.m_fLogAccu >= VEH_LOG_PERIOD)
		{
			float spd = vector.DistanceXZ(p, gi.m_vLastPos) / gi.m_fLogAccu;
			string state = "transport";
			if (gi.m_bTank)
			{
				state = "armor";
				if (gi.m_bInPosition)
					state = "armor-support";
			}
			if (gi.m_bReturning)
				state = "return";
			PrintFormat("[F44] AIVEH %1 team=%2 side=%3 state=%4 speed=%5 m/s toDest=%6 m toSector=%7 m", gi.m_Vehicle.m_sKey, gi.m_sTeamId, gi.m_iSide, state, Math.Round(spd * 10) / 10, Math.Round(toDest), Math.Round(toSector));
			bool parked = gi.m_bInPosition || toDest < 40;
			gi.m_vLastPos = p;
			gi.m_fLogAccu = 0;
			// no progress toward the destination for 30 s (slow creeping counts as stuck too)
			if (toDest < gi.m_fBestDest - 15)
			{
				gi.m_fBestDest = toDest;
				gi.m_fStillTime = 0;
			}
			else if (!parked)
				gi.m_fStillTime += VEH_LOG_PERIOD;
			if (gi.m_fStillTime >= 30)
			{
				gi.m_fStillTime = 0;
				gi.m_fBestDest = toDest;
				gi.m_iStuck++;
				if (gi.m_iStuck >= 3 && !gi.m_bReturning && sector)
				{
					// give up driving: infantry goes on foot from here, armor supports from where it is
					PrintFormat("[F44] AI: %1 (team %2) still stuck after %3 re-routes, %4 m from sector %5", gi.m_Vehicle.m_sKey, gi.m_sTeamId, gi.m_iStuck - 1, Math.Round(toSector), sector.m_sSectorName);
					if (gi.m_bTank)
					{
						gi.m_vDest = p;
						gi.m_bInPosition = true;
						ClearWaypoints(gi);
						AddWp(gi, WP_DEFEND, p, 50);
					}
					else
						Unload(gi, sector, toSector);
					return;
				}
				PrintFormat("[F44] AI: %1 (team %2) stuck, re-routing", gi.m_Vehicle.m_sKey, gi.m_sTeamId);
				if (gi.m_bReturning)
					SendHome(gi);
				else
					Retarget(gi, m_eMode, m_GameMode.GetSectorOwners(), false);
				return;
			}
		}

		if (gi.m_bReturning)
		{
			if (toDest < 35)
				FreeAtBase(gi);
			return;
		}
		if (!sector)
			return;
		if (gi.m_bTank)
		{
			if (!gi.m_bInPosition && toDest < 35)
			{
				gi.m_bInPosition = true;
				ClearWaypoints(gi);
				AddWp(gi, WP_DEFEND, gi.m_vDest, 50);
				PrintFormat("[F44] AI: %1 (team %2) in support position %3 m from sector %4", gi.m_Vehicle.m_sKey, gi.m_sTeamId, Math.Round(toSector), sector.m_sSectorName);
			}
			return;
		}
		if (toSector <= UNLOAD_DIST + 25 || toDest < 30)
			Unload(gi, sector, toSector);
	}

	//------------------------------------------------------------------------------------------------
	protected void Unload(F44_AIGroupInfo gi, F44_SectorComponent sector, float dist)
	{
		F44_TrackedVehicle tv = gi.m_Vehicle;
		IEntity veh = tv.m_Entity;
		IEntity driver = FindDriver(veh);
		array<IEntity> members = GroupMembers(gi.m_Group);
		bool split = driver && members.Count() > 1;
		int nOut;
		foreach (IEntity ch : members)
		{
			if (split && ch == driver)
				continue;
			CompartmentAccessComponent ca = CompartmentAccessComponent.Cast(ch.FindComponent(CompartmentAccessComponent));
			if (!ca || !ca.IsInCompartment())
				continue;
			// animated get-out can fail (vehicle still moving / just boarded): teleport out then
			if (ca.GetOutVehicle(EGetOutType.ANIMATED, -1, ECloseDoorAfterActions.INVALID, false) || ca.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false))
				nOut++;
		}
		SetUsable(gi.m_Group, veh, false);
		gi.m_bDismounted = true;
		gi.m_Vehicle = null;
		tv.m_AIGroup = null;
		PrintFormat("[F44] AI: side %1 group (team %2) unloads from %3 %4 m from sector %5 (%6 out)", gi.m_iSide, gi.m_sTeamId, tv.m_sKey, Math.Round(dist), sector.m_sSectorName, nOut);
		if (split)
			SplitDriver(gi, tv, driver);
		ClearWaypoints(gi);
		Retarget(gi, m_eMode, m_GameMode.GetSectorOwners(), false);	// fight on foot
	}

	//------------------------------------------------------------------------------------------------
	//! the driver leaves the squad and brings the transport back to base
	protected void SplitDriver(F44_AIGroupInfo gi, F44_TrackedVehicle tv, IEntity driver)
	{
		F44_CatalogEntry ge = F44_Catalog.Get().GetEntry(gi.m_sSideKey + "_AIGROUP");
		if (!ge)
			return;
		Resource gres = Resource.Load(ge.m_sPrefab);
		if (!gres || !gres.IsValid())
			return;
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = driver.GetOrigin();
		SCR_AIGroup ng = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(gres, GetGame().GetWorld(), params));
		if (!ng)
			return;
		ng.SetFaction(gi.m_Group.GetFaction());
		gi.m_Group.RemoveAIEntityFromGroup(driver);
		ng.AddAIEntityToGroup(driver);
		F44_AIGroupInfo d = new F44_AIGroupInfo();
		d.m_Group = ng;
		d.m_iSide = gi.m_iSide;
		d.m_sTeamId = gi.m_sTeamId;
		d.m_sSideKey = gi.m_sSideKey;
		d.m_iTargetSector = gi.m_iTargetSector;
		d.m_Vehicle = tv;
		d.m_bReturning = true;
		d.m_bBoarded = true;
		d.m_vHome = gi.m_vHome;
		d.m_vLastPos = tv.m_Entity.GetOrigin();
		tv.m_AIGroup = ng;
		SetUsable(ng, tv.m_Entity, true);
		m_aPending.Insert(d);
		SendHome(d);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHome(F44_AIGroupInfo gi)
	{
		if (!gi.m_Vehicle || !gi.m_Vehicle.m_Entity)
			return;
		ClearWaypoints(gi);
		int n = AddRoute(gi, gi.m_Vehicle.m_Entity.GetOrigin(), gi.m_vHome, WP_FORCEDMOVE, 20);
		PrintFormat("[F44] AI: driver takes %1 (team %2) back to base (%3 road waypoints, %4 m)", gi.m_Vehicle.m_sKey, gi.m_sTeamId, n, Math.Round(vector.DistanceXZ(gi.m_Vehicle.m_Entity.GetOrigin(), gi.m_vHome)));
	}

	//------------------------------------------------------------------------------------------------
	protected void FreeAtBase(F44_AIGroupInfo gi)
	{
		F44_TrackedVehicle tv = gi.m_Vehicle;
		IEntity veh = tv.m_Entity;
		foreach (IEntity ch : GroupMembers(gi.m_Group))
		{
			CompartmentAccessComponent ca = CompartmentAccessComponent.Cast(ch.FindComponent(CompartmentAccessComponent));
			if (ca && ca.IsInCompartment() && !ca.GetOutVehicle(EGetOutType.ANIMATED, -1, ECloseDoorAfterActions.INVALID, false))
				ca.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);
		}
		SetUsable(gi.m_Group, veh, false);
		tv.m_AIGroup = null;
		gi.m_Vehicle = null;
		gi.m_bReturning = false;
		gi.m_bDismounted = true;
		PrintFormat("[F44] AI: %1 (team %2) back at base, free for the next group", tv.m_sKey, gi.m_sTeamId);
		ClearWaypoints(gi);
		Retarget(gi, m_eMode, m_GameMode.GetSectorOwners(), false);
	}

	//------------------------------------------------------------------------------------------------
	void DeleteAll()
	{
		foreach (F44_AIGroupInfo gi : m_aGroups)
			ClearWaypoints(gi);
		m_aGroups.Clear();
	}
}
