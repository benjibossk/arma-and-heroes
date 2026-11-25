// Arma & Heroes - P3 squads (GAP_PLAN.md), server side. Squads = vanilla player groups (SCR_GroupsManagerComponent),
// the group leader is the squad leader.
//  - Orders: every 10 s each squad of >= 2 players gets an order on a sector: ATTACK (sector not owned by the side)
//    or DEFEND (owned). ASSAULT = the active sector of the assault order; SKIRMISH = sector nearest to the leader.
//    The leader can pin another order by standing inside a sector for 5 s (that sector becomes the order).
//    Members are told by hint when the order changes; time spent inside the ordered sector is counted (orderSec).
//  - Spawn on leader (squads.spawnOnLeader, default on): a member deploying at a sector / base point is moved next
//    to his leader when the leader is alive, on foot, and no enemy is within leaderSpawnEnemyRadius.
//  - BattleResult players gain "squad": {"size", "leader", "orderSec"} (web may pay a squad bonus).
// Battle JSON (optional): "squads": {"enabled": true, "spawnOnLeader": true, "leaderSpawnEnemyRadius": 60}

//------------------------------------------------------------------------------------------------
class F44_SquadConfig
{
	bool m_bEnabled = true;
	bool m_bSpawnOnLeader = true;
	float m_fEnemyRadius = 60;

	void Read(LoadContext root)
	{
		if (!root || !root.StartObject("squads"))
			return;
		root.ReadValue("enabled", m_bEnabled);
		root.ReadValue("spawnOnLeader", m_bSpawnOnLeader);
		root.ReadValue("leaderSpawnEnemyRadius", m_fEnemyRadius);
		root.EndObject();
	}
}

//------------------------------------------------------------------------------------------------
class F44_SquadStat
{
	int m_iSize;
	bool m_bLeader;
	float m_fOrderSec;
}

//------------------------------------------------------------------------------------------------
class F44_Squads
{
	static ref F44_SquadConfig s_Cfg = new F44_SquadConfig();
	protected static ref map<int, int> s_mOrder = new map<int, int>();	// group id -> ordered sector
	protected static ref map<int, int> s_mPinned = new map<int, int>();	// group id -> sector pinned by the leader
	protected static ref map<int, float> s_mLeaderInSector = new map<int, float>();	// group id -> s inside current sector
	protected static ref map<int, ref F44_SquadStat> s_mStats = new map<int, ref F44_SquadStat>();
	protected static float s_fAccu;

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mOrder.Clear();
		s_mPinned.Clear();
		s_mLeaderInSector.Clear();
		s_mStats.Clear();
		s_fAccu = 0;
	}

	//------------------------------------------------------------------------------------------------
	static SCR_AIGroup GroupOf(int pid)
	{
		SCR_GroupsManagerComponent gmc = SCR_GroupsManagerComponent.GetInstance();
		if (!gmc)
			return null;
		return gmc.GetPlayerGroup(pid);
	}

	//------------------------------------------------------------------------------------------------
	protected static F44_SquadStat Stat(int pid)
	{
		F44_SquadStat s;
		if (!s_mStats.Find(pid, s))
		{
			s = new F44_SquadStat();
			s_mStats.Set(pid, s);
		}
		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected static int SectorAt(vector pos)
	{
		for (int i = 0; i < F44_SectorManager.Count(); i++)
		{
			F44_SectorComponent s = F44_SectorManager.GetSector(i);
			if (s && s.IsInside(pos))
				return i;
		}
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static int NearestSector(vector pos)
	{
		int best = -1;
		float bestD = float.MAX;
		for (int i = 0; i < F44_SectorManager.Count(); i++)
		{
			F44_SectorComponent s = F44_SectorManager.GetSector(i);
			if (!s)
				continue;
			float d = vector.DistanceSqXZ(pos, s.GetCenter());
			if (d < bestD)
			{
				bestD = d;
				best = i;
			}
		}
		return best;
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(float dt, array<int> owners, F44_EBattleMode mode, map<int, int> playerSide)
	{
		if (!s_Cfg.m_bEnabled || F44_SectorManager.Count() == 0)
			return;
		s_fAccu += dt;
		bool refresh = s_fAccu >= 10;
		if (refresh)
			s_fAccu = 0;
		PlayerManager pm = GetGame().GetPlayerManager();
		map<int, bool> seen = new map<int, bool>();
		foreach (int pid, int side : playerSide)
		{
			SCR_AIGroup g = GroupOf(pid);
			if (!g || seen.Contains(g.GetGroupID()))
				continue;
			seen.Set(g.GetGroupID(), true);
			array<int> members = g.GetPlayerIDs();
			if (members.Count() < 2)
				continue;
			int gid = g.GetGroupID();
			int leader = g.GetLeaderID();
			IEntity lent = pm.GetPlayerControlledEntity(leader);

			// leader pins an order by standing 5 s in a sector
			if (lent && F44_SectorManager.IsAlive(lent))
			{
				int here = SectorAt(lent.GetOrigin());
				float t;
				s_mLeaderInSector.Find(gid, t);
				if (here >= 0)
				{
					t += dt;
					if (t >= 5)
						s_mPinned.Set(gid, here);
				}
				else
					t = 0;
				s_mLeaderInSector.Set(gid, t);
			}

			if (refresh)
			{
				int order = -1;
				if (!s_mPinned.Find(gid, order))
				{
					if (mode == F44_EBattleMode.ASSAULT)
					{
						for (int i = 0; i < owners.Count(); i++)
						{
							if (F44_SectorManager.IsActive(i, mode, owners))
							{
								order = i;
								break;
							}
						}
					}
					else if (lent)
						order = NearestSector(lent.GetOrigin());
				}
				int prev = -2;
				s_mOrder.Find(gid, prev);
				if (order >= 0 && order != prev)
				{
					s_mOrder.Set(gid, order);
					string verb = "ATTACK";
					if (order < owners.Count() && owners[order] == side)
						verb = "DEFEND";
					string txt = string.Format("Squad order: %1 %2", verb, F44_SectorManager.GetSector(order).m_sSectorName);
					foreach (int m : members)
						F44_Rules.Notify(m, txt);
					PrintFormat("[F44] SQUAD group=%1 leader=%2 order=%3 %4", gid, leader, verb, order);
				}
			}

			int ord = -1;
			s_mOrder.Find(gid, ord);
			foreach (int m2 : members)
			{
				F44_SquadStat st = Stat(m2);
				st.m_iSize = Math.Max(st.m_iSize, members.Count());
				st.m_bLeader = st.m_bLeader || m2 == leader;
				IEntity me = pm.GetPlayerControlledEntity(m2);
				if (ord >= 0 && me && F44_SectorManager.IsAlive(me) && F44_SectorManager.GetSector(ord).IsInside(me.GetOrigin()))
					st.m_fOrderSec += dt;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! after a deploy: move the member next to his leader (CallLater so the spawn is complete)
	static void OnDeployed(int pid, IEntity character, map<int, int> playerSide)
	{
		if (!s_Cfg.m_bEnabled || !s_Cfg.m_bSpawnOnLeader || !character)
			return;
		SCR_AIGroup g = GroupOf(pid);
		if (!g || g.GetLeaderID() == pid || g.GetLeaderID() <= 0)
			return;
		GetGame().GetCallqueue().CallLater(MoveToLeader, 800, false, character, g.GetLeaderID(), playerSide);
	}

	//------------------------------------------------------------------------------------------------
	protected static void MoveToLeader(IEntity character, int leader, map<int, int> playerSide)
	{
		IEntity lent = GetGame().GetPlayerManager().GetPlayerControlledEntity(leader);
		if (!character || !lent || !F44_SectorManager.IsAlive(lent))
			return;
		CompartmentAccessComponent ca = CompartmentAccessComponent.Cast(lent.FindComponent(CompartmentAccessComponent));
		if (ca && ca.IsInCompartment())
			return;
		ca = CompartmentAccessComponent.Cast(character.FindComponent(CompartmentAccessComponent));
		if (ca && ca.IsInCompartment())
			return;	// spawned in a vehicle (mobile spawn / air drop): keep it
		int mySide;
		playerSide.Find(leader, mySide);
		float r2 = s_Cfg.m_fEnemyRadius * s_Cfg.m_fEnemyRadius;
		PlayerManager pm = GetGame().GetPlayerManager();
		foreach (int p, int s : playerSide)
		{
			if (s == mySide)
				continue;
			IEntity e = pm.GetPlayerControlledEntity(p);
			if (e && F44_SectorManager.IsAlive(e) && vector.DistanceSqXZ(e.GetOrigin(), lent.GetOrigin()) < r2)
				return;	// leader in contact: normal spawn
		}
		vector pos = lent.GetOrigin() - lent.GetTransformAxis(2) * 2.5;
		pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		vector mat[4];
		character.GetWorldTransform(mat);
		mat[3] = pos;
		BaseGameEntity bge = BaseGameEntity.Cast(character);
		if (bge)
			bge.Teleport(mat);
		PrintFormat("[F44] SQUAD spawn on leader %1 at %2", leader, pos);
	}

	//------------------------------------------------------------------------------------------------
	static string PlayerJson(int pid)
	{
		F44_SquadStat s;
		if (!s_mStats.Find(pid, s))
			return string.Empty;
		string lead = "false";
		if (s.m_bLeader)
			lead = "true";
		return string.Format(",\"squad\":{\"size\":%1,\"leader\":%2,\"orderSec\":%3}", s.m_iSize, lead, Math.Round(s.m_fOrderSec));
	}
}
