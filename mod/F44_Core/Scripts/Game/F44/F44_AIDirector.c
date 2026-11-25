// Arma & Heroes - AI reinforcements (server only).
// Each side tops up its strength with AI groups built from its Assault Teams:
//   aliveTarget = max(0, aiMaxAlive - playersOnSide * aiPerPlayerScale), capped by the global aiLimit.
// Groups spawn on the side's spawn points, use the team classes (catalog) and get waypoints
// re-evaluated every 30 s: attackers -> active sector (ASSAULT) or nearest non-owned sector (SKIRMISH),
// defenders -> defend the threatened / owned sectors. Bots capture sectors (F44_SectorManager.Simulate).
// Bot deaths cost 1 ticket and count in the team manpower losses (handled by the game mode).

//------------------------------------------------------------------------------------------------
class F44_AIGroupInfo
{
	SCR_AIGroup m_Group;
	int m_iSide;
	string m_sTeamId;
	int m_iTargetSector = -1;
	AIWaypoint m_Waypoint;
}

//------------------------------------------------------------------------------------------------
class F44_AIDirector
{
	static const ResourceName WP_ATTACK = "{B3E7B8DC2BAB8ACC}Prefabs/AI/Waypoints/AIWaypoint_SearchAndDestroy.et";
	static const ResourceName WP_DEFEND = "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et";
	static const int GROUP_SIZE = 4;

	protected F44_GameModeComponent m_GameMode;
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

		m_fRetargetAccu += dt;
		if (m_fRetargetAccu >= 30)
		{
			m_fRetargetAccu = 0;
			foreach (F44_AIGroupInfo gi : m_aGroups)
				Retarget(gi, cfg.GetMode(), owners, true);
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
		m_aGroups.Insert(gi);
		Retarget(gi, cfg.GetMode(), m_GameMode.GetSectorOwners(), false);
		MapAddI(m_mSkillSum, sideId, SKILLS.Find(GetSkill(sideId)));
		MapAddI(m_mSkillN, sideId, 1);
		PrintFormat("[F44] AI: %1 group of %2 spawned (team %3 \"%4\", skill %5)", side.m_sSideKey, spawned, team.m_sTeamId, team.m_sDisplayName, GetSkill(sideId));
	}

	//------------------------------------------------------------------------------------------------
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
	protected void Retarget(F44_AIGroupInfo gi, F44_EBattleMode mode, array<int> owners, bool onlyIfChanged)
	{
		if (!gi.m_Group || gi.m_Group.GetAgentsCount() == 0)
			return;
		vector from = gi.m_Group.GetOrigin();
		IEntity leader = gi.m_Group.GetLeaderEntity();
		if (leader)
			from = leader.GetOrigin();

		int target = ChooseSector(gi.m_iSide, mode, owners, from);
		if (target < 0)
			return;
		if (onlyIfChanged && target == gi.m_iTargetSector && gi.m_Waypoint)
			return;

		F44_SectorComponent sector = F44_SectorManager.GetSector(target);
		if (!sector)
			return;

		// attackers seize, defenders hold what they own; a defender sent to a lost sector counter-attacks
		ResourceName wpPrefab = WP_ATTACK;
		bool ownSector = (gi.m_iSide == F44_ESectorOwner.DEFENDER && owners[target] == F44_ESectorOwner.DEFENDER)
			|| (gi.m_iSide == F44_ESectorOwner.ATTACKER && owners[target] == F44_ESectorOwner.ATTACKER);
		if (ownSector)
			wpPrefab = WP_DEFEND;

		Resource res = Resource.Load(wpPrefab);
		if (!res || !res.IsValid())
			return;
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = sector.GetOwner().GetOrigin();
		AIWaypoint wp = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
		if (!wp)
			return;
		wp.SetCompletionRadius(sector.m_fRadius * 0.8);

		if (gi.m_Waypoint)
		{
			gi.m_Group.RemoveWaypoint(gi.m_Waypoint);
			SCR_EntityHelper.DeleteEntityAndChildren(gi.m_Waypoint);
		}
		gi.m_Group.AddWaypoint(wp);
		gi.m_Waypoint = wp;
		string wpKind = "attack";
		if (ownSector)
			wpKind = "defend";
		PrintFormat("[F44] AI: side %1 group (team %2) -> %3 sector %4", gi.m_iSide, gi.m_sTeamId, wpKind, sector.m_sSectorName);
		gi.m_iTargetSector = target;
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
	void DeleteAll()
	{
		foreach (F44_AIGroupInfo gi : m_aGroups)
		{
			if (gi.m_Waypoint)
				SCR_EntityHelper.DeleteEntityAndChildren(gi.m_Waypoint);
		}
		m_aGroups.Clear();
	}
}
