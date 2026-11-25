// Arma & Heroes - dev tool: dumps map locations, nearby buildings and road junctions of the loaded terrain
// into $profile:F44/locations.txt. Runs once on the server when the file $profile:F44/dump_locations exists.
// Used to author Configs/F44/layouts.json on real towns / buildings / crossroads.
class F44_WorldDump
{
	static const string FLAG = "$profile:F44/dump_locations";
	static const string OUT = "$profile:F44/locations.txt";

	protected static ref F44_WorldDump s_Dump;
	protected ref array<IEntity> m_aLocations = {};
	protected ref array<IEntity> m_aBuildings = {};

	static void RunIfRequested()
	{
		if (!FileIO.FileExists(FLAG))
			return;
		s_Dump = new F44_WorldDump();
		GetGame().GetCallqueue().CallLater(s_Dump.Run, 20000, false);	// road network is ready later
	}

	protected bool OnEntity(IEntity e)
	{
		if (e.FindComponent(MapDescriptorComponent))
		{
			m_aLocations.Insert(e);
			return true;
		}
		EntityPrefabData pd = e.GetPrefabData();
		if (!pd)
			return true;
		string pn = pd.GetPrefabName();
		if ((pn.Contains("/Houses/") || pn.Contains("/Commercial/") || pn.Contains("/Religious/") || pn.Contains("Structures/Buildings")) && !pn.Contains("Ruin") && !pn.Contains("Shed") && !pn.Contains("Toilet"))
			m_aBuildings.Insert(e);
		return true;
	}

	protected static string PrefabShort(IEntity e)
	{
		EntityPrefabData pd = e.GetPrefabData();
		if (!pd)
			return "?";
		string pn = pd.GetPrefabName();
		int slash = pn.LastIndexOf("/");
		if (slash >= 0)
			pn = pn.Substring(slash + 1, pn.Length() - slash - 1);
		return pn;
	}

	protected static string V(vector v)
	{
		return string.Format("%1 %2 %3", Math.Round(v[0]), Math.Round(v[1]), Math.Round(v[2]));
	}

	void Run()
	{
		BaseWorld w = GetGame().GetWorld();
		vector mins, maxs;
		w.GetBoundBox(mins, maxs);
		vector c = (mins + maxs) * 0.5;
		float r = vector.Distance(mins, maxs) * 0.5;
		w.QueryEntitiesBySphere(c, r, OnEntity, null, EQueryEntitiesFlags.ALL);
		PrintFormat("[F44] WorldDump: %1 locations, %2 buildings (bbox %3 .. %4)", m_aLocations.Count(), m_aBuildings.Count(), mins, maxs);

		RoadNetworkManager rnm;
		SCR_AIWorld aiw = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (aiw)
			rnm = aiw.GetRoadNetworkManager();

		PrintFormat("[F44] WorldDump: road network %1", rnm != null);
		FileHandle f = FileIO.OpenFile(OUT, FileMode.WRITE);
		if (!f)
			return;
		foreach (IEntity loc : m_aLocations)
		{
			string name = PrefabShort(loc);
			MapDescriptorComponent md = MapDescriptorComponent.Cast(loc.FindComponent(MapDescriptorComponent));
			if (md && md.Item())
				name = md.Item().GetDisplayName() + " (" + name + ")";
			vector lp = loc.GetOrigin();
			string fullPn;
			if (loc.GetPrefabData())
				fullPn = loc.GetPrefabData().GetPrefabName();
			f.WriteLine("LOC " + V(lp) + " " + name + " " + fullPn);
			int nb;
			foreach (IEntity b : m_aBuildings)
			{
				float d = vector.DistanceXZ(b.GetOrigin(), lp);
				if (d > 350)
					continue;
				nb++;
				if (nb <= 40)
					f.WriteLine("  BLD " + V(b.GetOrigin()) + " d=" + Math.Round(d) + " " + PrefabShort(b));
			}
			f.WriteLine("  buildings within 350 m: " + nb);
			if (!rnm)
				continue;
			array<BaseRoad> roads = {};
			rnm.GetRoadsInAABB(lp - "400 200 400", lp + "400 200 400", roads);
			array<vector> ends = {};
			foreach (BaseRoad rd : roads)
			{
				array<vector> pts = {};
				rd.GetPoints(pts);
				if (pts.Count() < 2)
					continue;
				ends.Insert(pts[0]);
				ends.Insert(pts[pts.Count() - 1]);
			}
			// junction = 3+ road ends within 8 m
			array<vector> junctions = {};
			foreach (vector e1 : ends)
			{
				int n;
				foreach (vector e2 : ends)
				{
					if (vector.DistanceXZ(e1, e2) < 8)
						n++;
				}
				bool dup;
				foreach (vector j : junctions)
				{
					if (vector.DistanceXZ(j, e1) < 20)
						dup = true;
				}
				if (n >= 3 && !dup)
					junctions.Insert(e1);
			}
			foreach (vector jj : junctions)
				f.WriteLine("  JCT " + V(jj) + " d=" + Math.Round(vector.DistanceXZ(jj, lp)));
			f.WriteLine("  roads: " + roads.Count());
		}
		f.Close();
		Print("[F44] WorldDump written to " + OUT);
	}
}
