// Arma & Heroes - forward spawn points on captured sectors (SERVER authoritative).
// A sector owned by a side becomes a spawn point for that side when it touches the front line
// (its neighbour in assault order is not owned by the same side). The point is a copy of the
// side's HQ spawn point prefab (vanilla SCR_SpawnPoint, so the vanilla deploy menu / map lists it).
// Contested sector (capture progress not full) = spawn point disabled. Forward points carry a
// deploy delay penalty (SCR_SpawnPoint respawn time, checked by the vanilla server respawn timer).
//! Optional battle JSON "spawnTimes" object (aligned on H&G, see inventory/HG_MECHANICS.md spawn time)
class F44_SpawnTimesConfig
{
	float m_fBaseSec = 10;	// vanilla SCR_RespawnTimerComponent base timer (H&G soldier base 10 s)
	float m_fForwardPenaltySec = 10;	// added on a forward sector point
	// P1-C respawn model: per-player delay = base + equipment term - defender bonus (clamped),
	// plus the spawn point term = forward penalty + per-link distance from the side's HQ.
	float m_fPerPointSec = 0.5;	// s per loadout equipment point (playerLoadouts.points)
	float m_fMaxEquipSec = 20;	// cap of the equipment term
	float m_fPerLinkSec = 3;	// s per sector link between the side's HQ end and the forward point
	float m_fDefenderBonusSec = 3;	// s removed from the defender's delay
	float m_fMinSec = 3;	// floor of the per-player delay

	static ref F44_SpawnTimesConfig s_Current;	// last config read (server), used by the respawn timer

	static F44_SpawnTimesConfig Read(LoadContext root)
	{
		F44_SpawnTimesConfig c = new F44_SpawnTimesConfig();
		s_Current = c;
		if (!root || !root.StartObject("spawnTimes"))
			return c;
		root.ReadValue("baseSec", c.m_fBaseSec);
		root.ReadValue("forwardPenaltySec", c.m_fForwardPenaltySec);
		root.ReadValue("perPointSec", c.m_fPerPointSec);
		root.ReadValue("maxEquipSec", c.m_fMaxEquipSec);
		root.ReadValue("perLinkSec", c.m_fPerLinkSec);
		root.ReadValue("defenderBonusSec", c.m_fDefenderBonusSec);
		root.ReadValue("minSec", c.m_fMinSec);
		root.EndObject();
		return c;
	}

	//! per-player part of the delay (s): base + equipment points term - defender bonus
	float PlayerDelay(int points, bool defender)
	{
		float d = m_fBaseSec + Math.Min(Math.Max(points, 0) * m_fPerPointSec, m_fMaxEquipSec);
		if (defender)
			d -= m_fDefenderBonusSec;
		return Math.Max(d, m_fMinSec);
	}

	//! spawn point part (s): forward penalty + links from the side's HQ end of the assault order
	float PointDelay(int idx, int side, int sectorCount)
	{
		int links = idx + 1;	// attacker HQ sits before sector 0
		if (side == F44_ESectorOwner.DEFENDER)
			links = sectorCount - idx;	// defender HQ sits after the last sector
		return m_fForwardPenaltySec + Math.Max(links, 0) * m_fPerLinkSec;
	}
}

//------------------------------------------------------------------------------------------------
//! Per-player respawn delay (P1-C): set on death from the web loadout points (server, replicated)
modded class SCR_RespawnTimerComponent
{
	override void OnPlayerKilled(notnull SCR_InstigatorContextData instigatorContextData)
	{
		super.OnPlayerKilled(instigatorContextData);
		F44_SpawnTimesConfig cfg = F44_SpawnTimesConfig.s_Current;
		int pid = instigatorContextData.GetVictimPlayerID();
		if (!cfg || pid <= 0 || !IsMaster())
			return;
		int points;
		F44_LoadoutService ls = F44_LoadoutService.s_Instance;
		if (ls)
		{
			F44_PlayerLoadout l = ls.Get(SCR_PlayerIdentityUtils.GetPlayerIdentityId(pid));
			if (l)
				points = l.m_iPoints;
		}
		bool defender;
		F44_GameModeComponent gm = F44_GameModeComponent.GetInstance();
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(instigatorContextData.GetVictimEntity());
		if (gm && ch && F44_Catalog.Get())
			defender = F44_Catalog.Get().GetSideFromFaction(ch.GetFactionKey()) == gm.GetDefender();
		float d = cfg.PlayerDelay(points, defender);
		SetRespawnTime(pid, d);
		PrintFormat("[F44] Respawn delay player %1: %2 s (points %3, defender %4)", pid, d, points, defender);
	}

	//! per-player timer duration (client and server), -1 if unknown
	float F44_GetPlayerDuration(int playerId)
	{
		SCR_RespawnTimer t = m_mRespawnTimers.Get(playerId);
		if (!t)
			return -1;
		return t.GetDuration();
	}
}

//------------------------------------------------------------------------------------------------
class F44_SectorSpawns
{
	float m_fForwardPenalty = 10;	// s added to the deploy timer on a forward point (spawnTimes.forwardPenaltySec)
	static const string NAME_PREFIX = "SECTOR ";

	protected ref map<int, SCR_SpawnPoint> m_mPoints = new map<int, SCR_SpawnPoint>();	// sector idx -> point
	protected ref map<int, int> m_mPointSide = new map<int, int>();
	protected float m_fAccu;

	//------------------------------------------------------------------------------------------------
	static bool IsFrontSector(int idx, int side, array<int> owners)
	{
		if (idx < 0 || idx >= owners.Count() || owners[idx] != side || side == F44_ESectorOwner.NONE)
			return false;
		if (idx > 0 && owners[idx - 1] != side)
			return true;
		if (idx + 1 < owners.Count() && owners[idx + 1] != side)
			return true;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsContested(int idx, array<int> owners, array<int> progress)
	{
		if (idx < 0 || idx >= progress.Count())
			return false;
		int p = progress[idx];
		if (owners[idx] == F44_ESectorOwner.ATTACKER)
			return p < 100;
		if (owners[idx] == F44_ESectorOwner.DEFENDER)
			return p > -100;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static ResourceName FindHqPrefab(string factionKey)
	{
		foreach (SCR_SpawnPoint sp : SCR_SpawnPoint.GetSpawnPoints())
		{
			if (sp.GetFactionKey() != factionKey || sp.GetSpawnPointName().StartsWith(NAME_PREFIX) || sp.GetSpawnPointName().StartsWith(F44_MobileSpawns.NAME_PREFIX))
				continue;
			EntityPrefabData pd = sp.GetPrefabData();
			if (pd)
				return pd.GetPrefabName();
		}
		return ResourceName.Empty;
	}

	//! HQ spawn point prefab of a faction (used by F44_MobileSpawns)
	static ResourceName FindHqPrefabPublic(string factionKey)
	{
		return FindHqPrefab(factionKey);
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> owners, array<int> progress, string attFaction, string defFaction)
	{
		m_fAccu += dt;
		if (m_fAccu < 2)
			return;
		m_fAccu = 0;

		for (int i = 0; i < owners.Count(); i++)
		{
			int side = owners[i];
			bool want = IsFrontSector(i, side, owners);
			SCR_SpawnPoint cur = m_mPoints.Get(i);
			if (cur && (!want || m_mPointSide.Get(i) != side))
			{
				SCR_EntityHelper.DeleteEntityAndChildren(cur);
				m_mPoints.Remove(i);
				m_mPointSide.Remove(i);
				cur = null;
			}
			if (!want)
				continue;

			if (!cur)
				cur = CreatePoint(i, side, attFaction, defFaction);
			if (cur)
			{
				bool enabled = !IsContested(i, owners, progress);
				if (cur.IsSpawnPointEnabled() != enabled)
					cur.SetSpawnPointEnabled_S(enabled);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SpawnPoint CreatePoint(int idx, int side, string attFaction, string defFaction)
	{
		F44_SectorComponent sc = F44_SectorManager.GetSector(idx);
		if (!sc)
			return null;
		string fk = defFaction;
		if (side == F44_ESectorOwner.ATTACKER)
			fk = attFaction;
		ResourceName prefab = FindHqPrefab(fk);
		if (prefab.IsEmpty())
			return null;
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return null;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		vector pos = sc.GetCenter();
		pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);
		params.Transform[3] = pos;
		SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
		if (!sp)
			return null;
		sp.SetFactionKey(fk);
		sp.SetSpawnPointName(NAME_PREFIX + sc.m_sSectorName);
		float pen = m_fForwardPenalty;
		if (F44_SpawnTimesConfig.s_Current)
			pen = F44_SpawnTimesConfig.s_Current.PointDelay(idx, side, F44_SectorManager.Count());
		sp.SetRespawnTime(pen);
		m_mPoints.Set(idx, sp);
		m_mPointSide.Set(idx, side);
		PrintFormat("[F44] Forward spawn point on sector %1 for %2 (+%3 s)", sc.m_sSectorName, fk, pen);
		return sp;
	}
}
