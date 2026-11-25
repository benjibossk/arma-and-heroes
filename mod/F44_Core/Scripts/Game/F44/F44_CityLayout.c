// Arma & Heroes - city battle layouts (1.1.0).
// A layout = sectors (name, x, z, radius) + infantry spawns per side + vehicle road anchors per side.
// Source: $profile:F44/layouts.json (server override) else Configs/F44/layouts.json (mod).
// Selection: battle "layout" id, else cityMap[battle.cityId], else a layout named like the cityId;
// an inline "layoutData" object in the battle payload wins over all of them.
// No layout -> the sectors / spawns placed in the world are used (legacy).
// Clients rebuild the same sectors from a compact replicated string (F44_GameModeComponent.m_sLayoutSectors).

//------------------------------------------------------------------------------------------------
class F44_Roads
{
	static RoadNetworkManager Get()
	{
		SCR_AIWorld aiw = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiw)
			return null;
		return aiw.GetRoadNetworkManager();
	}

	//! closest point on the road network within maxDist (XZ), with the road direction there
	static bool Snap(vector p, float maxDist, out vector res, out vector dir)
	{
		RoadNetworkManager rnm = Get();
		if (!rnm)
			return false;
		BaseRoad road;
		float d;
		rnm.GetClosestRoad(p, road, d);
		if (!road || d > maxDist)
			return false;
		array<vector> pts = {};
		road.GetPoints(pts);
		if (pts.Count() < 2)
			return false;
		float best = float.MAX;
		for (int i = 0; i < pts.Count() - 1; i++)
		{
			vector a = pts[i];
			vector b = pts[i + 1];
			vector ab = b - a;
			ab[1] = 0;
			float len2 = ab.LengthSq();
			if (len2 < 0.01)
				continue;
			vector ap = p - a;
			ap[1] = 0;
			float t = Math.Clamp(vector.Dot(ap, ab) / len2, 0, 1);
			vector q = a + (b - a) * t;
			float dq = vector.DistanceSqXZ(q, p);
			if (dq < best)
			{
				best = dq;
				res = q;
				dir = ab.Normalized();
			}
		}
		if (best == float.MAX || best > maxDist * maxDist)
			return false;
		res[1] = GetGame().GetWorld().GetSurfaceY(res[0], res[2]);
		return true;
	}

	//! vehicle route: points every ~150 m on the line from -> to, each snapped to a nearby road; ends at 'to'
	static array<vector> BuildRoute(vector from, vector to)
	{
		array<vector> route = {};
		float total = vector.DistanceXZ(from, to);
		int steps = Math.Floor(total / 150);
		vector last = from;
		for (int k = 1; k <= steps; k++)
		{
			float f = (k * 150) / total;
			if (f >= 0.95)
				break;
			vector p = from + (to - from) * f;
			vector rp, rdir;
			if (Snap(p, 90, rp, rdir))
				p = rp;
			if (vector.DistanceXZ(p, last) < 60 || vector.DistanceXZ(p, to) < 60)
				continue;
			route.Insert(p);
			last = p;
		}
		route.Insert(to);
		return route;
	}
}

//------------------------------------------------------------------------------------------------
class F44_LayoutPoint
{
	string m_sName;
	float m_fX;
	float m_fZ;
	float m_fRadius = 40;

	vector Pos()
	{
		vector p = Vector(m_fX, 0, m_fZ);
		p[1] = GetGame().GetWorld().GetSurfaceY(m_fX, m_fZ);
		return p;
	}
}

//------------------------------------------------------------------------------------------------
class F44_CityLayout
{
	static const string MOD_FILE = "Configs/F44/layouts.json";
	static const string PROFILE_FILE = "$profile:F44/layouts.json";
	static const ResourceName SECTOR_PREFAB = "{82EE26039EE138EB}Prefabs/F44/F44_Sector.et";
	static const ResourceName VEHSLOT_PREFAB = "{9CBA04DA3882CC02}Prefabs/F44/F44_VehicleSpawnPoint.et";

	string m_sId;
	string m_sName;
	string m_sWorld;	// optional world key (docs/MAPS.md): "arland", "ww2arland", "belleauwood", "normandy", "utahbeach"
	ref array<ref F44_LayoutPoint> m_aSectors = {};
	ref array<ref F44_LayoutPoint> m_aSpawnAtt = {};
	ref array<ref F44_LayoutPoint> m_aSpawnDef = {};
	ref array<ref F44_LayoutPoint> m_aVehAtt = {};
	ref array<ref F44_LayoutPoint> m_aVehDef = {};

	//------------------------------------------------------------------------------------------------
	protected static void ReadPoints(LoadContext n, string key, array<ref F44_LayoutPoint> outPts)
	{
		int c;
		if (!n.StartArray(key, c))
			return;
		for (int i = 0; i < c; i++)
		{
			if (!n.StartObject())
				break;
			F44_LayoutPoint p = new F44_LayoutPoint();
			n.ReadValue("name", p.m_sName);
			n.ReadValue("x", p.m_fX);
			n.ReadValue("z", p.m_fZ);
			n.ReadValue("radius", p.m_fRadius);
			if (p.m_fRadius <= 0)
				p.m_fRadius = 40;
			outPts.Insert(p);
			n.EndObject();
		}
		n.EndArray();
	}

	void FromJson(LoadContext n)
	{
		n.ReadValue("name", m_sName);
		n.ReadValue("world", m_sWorld);
		ReadPoints(n, "sectors", m_aSectors);
		ReadPoints(n, "attackerSpawns", m_aSpawnAtt);
		ReadPoints(n, "defenderSpawns", m_aSpawnDef);
		ReadPoints(n, "attackerVehicles", m_aVehAtt);
		ReadPoints(n, "defenderVehicles", m_aVehDef);
	}

	//------------------------------------------------------------------------------------------------
	static F44_CityLayout Resolve(string layoutId, string cityId)
	{
		string path = PROFILE_FILE;
		if (!FileIO.FileExists(path))
			path = MOD_FILE;
		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(path))
		{
			Print("[F44] Layout: cannot read " + path, LogLevel.WARNING);
			return null;
		}
		string id = layoutId;
		if (id.IsEmpty() && !cityId.IsEmpty() && ctx.StartObject("cityMap"))
		{
			ctx.ReadValue(cityId, id);
			ctx.EndObject();
		}
		if (id.IsEmpty())
			id = cityId;
		if (id.IsEmpty() || !ctx.StartObject("layouts"))
			return null;
		if (!ctx.StartObject(id))
		{
			PrintFormat("[F44] Layout: none for city '%1' (layout '%2') in %3, using the world sectors", cityId, id, path);
			return null;
		}
		F44_CityLayout l = new F44_CityLayout();
		l.m_sId = id;
		l.FromJson(ctx);
		ctx.EndObject();
		ctx.EndObject();
		if (l.m_aSectors.IsEmpty())
			return null;
		if (!IsWorldLoaded(l.m_sWorld))
		{
			PrintFormat("[F44] Layout: '%1' is for world '%2' but the loaded world is %3, using the world sectors", id, l.m_sWorld, GetGame().GetWorldFile(), level: LogLevel.WARNING);
			return null;
		}
		PrintFormat("[F44] Layout: '%1' (%2) for city '%3' from %4", id, l.m_sName, cityId, path);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! World key -> world file of the F44 mission (Missions/F44_<Map>.conf). Empty = any world (legacy).
	static string WorldFileKey(string world)
	{
		world.ToLower();
		if (world == "arland")
			return "F44_Test";
		if (world == "ww2arland")
			return "F44_WW2Arland";
		if (world == "belleauwood")
			return "F44_BelleauWood";
		if (world == "normandy")
			return "F44_Normandy";
		if (world == "utahbeach")
			return "F44_UtahBeach";
		return "";
	}

	static bool IsWorldLoaded(string world)
	{
		string key = WorldFileKey(world);
		if (key.IsEmpty())
			return true;
		return GetGame().GetWorldFile().Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! "name~x~z~r|..." (replicated to clients)
	string SerializeSectors()
	{
		string s;
		foreach (int i, F44_LayoutPoint p : m_aSectors)
		{
			if (i > 0)
				s += "|";
			s += string.Format("%1~%2~%3~%4", p.m_sName, Math.Round(p.m_fX), Math.Round(p.m_fZ), Math.Round(p.m_fRadius));
		}
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! replace the world sectors by the layout ones (server and clients)
	static void ApplySectors(string compact)
	{
		if (compact.IsEmpty())
			return;
		array<F44_SectorComponent> old = {};
		for (int i = 0; i < F44_SectorManager.Count(); i++)
			old.Insert(F44_SectorManager.GetSector(i));
		foreach (F44_SectorComponent sc : old)
		{
			F44_SectorManager.Unregister(sc);
			SCR_EntityHelper.DeleteEntityAndChildren(sc.GetOwner());
		}
		Resource res = Resource.Load(SECTOR_PREFAB);
		if (!res || !res.IsValid())
			return;
		array<string> items = {};
		compact.Split("|", items, true);
		foreach (int idx, string it : items)
		{
			array<string> f = {};
			it.Split("~", f, false);
			if (f.Count() < 4)
				continue;
			vector p = Vector(f[1].ToFloat(), 0, f[2].ToFloat());
			p[1] = GetGame().GetWorld().GetSurfaceY(p[0], p[2]);
			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = p;
			IEntity e = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
			if (!e)
				continue;
			F44_SectorComponent c = F44_SectorComponent.Cast(e.FindComponent(F44_SectorComponent));
			if (!c)
				continue;
			c.m_sSectorName = f[0];
			c.m_iOrder = idx;
			c.m_fRadius = f[3].ToFloat();
			if (Replication.IsServer())
				PrintFormat("[F44] Layout sector %1 '%2' at %3 %4 radius %5", idx, c.m_sSectorName, Math.Round(p[0]), Math.Round(p[2]), c.m_fRadius);
		}
		F44_SectorManager.SortByOrder();
	}

	//------------------------------------------------------------------------------------------------
	//! server: replace the battle factions' spawn points by the layout ones (same prefab as the world one)
	void ApplySpawns(string attFaction, string defFaction)
	{
		array<SCR_SpawnPoint> points = {};
		points.Copy(SCR_SpawnPoint.GetSpawnPoints());
		ResourceName attPrefab, defPrefab;
		foreach (SCR_SpawnPoint sp : points)
		{
			string fk = sp.GetFactionKey();
			if (fk != attFaction && fk != defFaction)
				continue;
			EntityPrefabData pd = sp.GetPrefabData();
			if (!pd)
				continue;
			if (fk == attFaction && !m_aSpawnAtt.IsEmpty())
				attPrefab = pd.GetPrefabName();
			else if (fk == defFaction && !m_aSpawnDef.IsEmpty())
				defPrefab = pd.GetPrefabName();
			else
				continue;
			SCR_EntityHelper.DeleteEntityAndChildren(sp);
		}
		SpawnPoints(attPrefab, m_aSpawnAtt, "attacker");
		SpawnPoints(defPrefab, m_aSpawnDef, "defender");
	}

	protected void SpawnPoints(ResourceName prefab, array<ref F44_LayoutPoint> pts, string label)
	{
		if (prefab.IsEmpty())
			return;
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
			return;
		foreach (F44_LayoutPoint lp : pts)
		{
			EntitySpawnParams params = new EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = lp.Pos();
			SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params));
			if (sp)
				PrintFormat("[F44] Layout %1 spawn at %2 %3", label, Math.Round(lp.m_fX), Math.Round(lp.m_fZ));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! server: n vehicle slots per side lined up along the road nearest to each anchor
	static void BuildVehicleSlots(array<vector> anchors, F44_ESectorOwner side, int n, vector facing)
	{
		if (anchors.IsEmpty() || n <= 0)
			return;
		Resource res = Resource.Load(VEHSLOT_PREFAB);
		if (!res || !res.IsValid())
			return;
		int perAnchor = Math.Ceil(n / (float)anchors.Count());
		int made;
		foreach (vector a : anchors)
		{
			vector start = a;
			vector dir = facing - a;
			dir[1] = 0;
			dir.Normalize();
			vector rp, rdir;
			bool onRoad = F44_Roads.Snap(a, 80, rp, rdir);
			if (onRoad)
			{
				start = rp;
				dir = rdir;
				// park facing the battle
				if (vector.Dot(dir, facing - start) < 0)
					dir = -dir;
			}
			vector side90 = Vector(dir[2], 0, -dir[0]);
			// two rows on both roadsides, 14 m apart, so every vehicle can pull out on the road
			for (int j = 0; j < perAnchor && made < n; j++)
			{
				float lat = 7;
				if (j % 2 == 1)
					lat = -7;
				vector p = start - dir * ((j / 2) * 14) + side90 * lat;
				vector free;
				if (SCR_WorldTools.FindEmptyTerrainPosition(free, p, 10, 3.5, 3))
					p = free;
				p[1] = GetGame().GetWorld().GetSurfaceY(p[0], p[2]) + 0.3;
				vector mat[4];
				Math3D.AnglesToMatrix(dir.VectorToAngles(), mat);
				mat[3] = p;
				EntitySpawnParams params = new EntitySpawnParams();
				params.TransformMode = ETransformMode.WORLD;
				params.Transform = mat;
				IEntity e = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
				if (!e)
					continue;
				F44_VehicleSpawnPointComponent c = F44_VehicleSpawnPointComponent.Cast(e.FindComponent(F44_VehicleSpawnPointComponent));
				if (c)
					c.m_eSide = side;
				made++;
				PrintFormat("[F44] Vehicle slot side %1 at %2 %3 (road %4)", side, Math.Round(p[0]), Math.Round(p[2]), onRoad);
			}
		}
	}
}
