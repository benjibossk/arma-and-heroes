// Arma & Heroes - forward spawn points on captured sectors (SERVER authoritative).
// A sector owned by a side becomes a spawn point for that side when it touches the front line
// (its neighbour in assault order is not owned by the same side). The point is a copy of the
// side's HQ spawn point prefab (vanilla SCR_SpawnPoint, so the vanilla deploy menu / map lists it).
// Contested sector (capture progress not full) = spawn point disabled. Forward points carry a
// deploy delay penalty (SCR_SpawnPoint respawn time, checked by the vanilla server respawn timer).
class F44_SectorSpawns
{
	static const float FORWARD_PENALTY = 10;	// s added to the deploy timer on a forward point
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
			if (sp.GetFactionKey() != factionKey || sp.GetSpawnPointName().StartsWith(NAME_PREFIX))
				continue;
			EntityPrefabData pd = sp.GetPrefabData();
			if (pd)
				return pd.GetPrefabName();
		}
		return ResourceName.Empty;
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
		sp.SetRespawnTime(FORWARD_PENALTY);
		m_mPoints.Set(idx, sp);
		m_mPointSide.Set(idx, side);
		PrintFormat("[F44] Forward spawn point on sector %1 for %2", sc.m_sSectorName, fk);
		return sp;
	}
}
