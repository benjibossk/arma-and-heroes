// Arma & Heroes - catalog: logical key -> Reforger prefab (+ cost).
// Format (D:\Serveur\ww2-heroes\catalog.json):
//   { "factions": { "US": "FH_USAWW2", "GER": "FH_HEERWW2" },
//     "catalog": { "US_RIFLEMAN": { "prefab": "{GUID}Prefabs/...", "cost": 1, ... }, ... } }
// prefab null = unavailable (warning, the class/vehicle cannot be used).
// Entries whose prefab lives under Prefabs/Characters/ are classes, the others vehicles.
// Source: $profile:F44/catalog.json if present, else the path given by the game mode (default Configs/F44/catalog.json).
class F44_CatalogEntry
{
	string m_sKey;
	ResourceName m_sPrefab;
	int m_iCost = 1;
	bool m_bIsClass;
	string m_sCategory; // optional: rifle, smg, lmg, sniper, at, explosive, vehicle...
	string m_sRole;     // vehicles: light_utility, truck, halftrack, tank...
	bool m_bIsItem;     // entry of the "items" section (player loadout)

	bool IsVehicle() { return !m_bIsClass && !m_bIsItem; }
	bool IsTank() { return m_sRole == "tank"; }
	bool IsTransport() { return m_sRole == "truck" || m_sRole == "halftrack" || m_sRole == "light_utility"; }
}

class F44_Catalog
{
	static const string PROFILE_PATH = "$profile:F44/catalog.json";
	static const string MOD_PATH = "Configs/F44/catalog.json";

	protected static ref F44_Catalog s_Instance;

	protected ref map<string, string> m_mFactions = new map<string, string>();
	protected ref map<string, ref F44_CatalogEntry> m_mEntries = new map<string, ref F44_CatalogEntry>();
	protected ref array<string> m_aUnavailable = {};
	protected ref map<string, ref array<string>> m_mAppearance = new map<string, ref array<string>>();	// "GER|Veteran|Recon" -> cloth prefabs

	//! cloth prefabs for a bot of faction/tier/team type (INF/MOT -> Infantry, REC -> Recon, PARA -> Para, ARM -> ArmorCrew)
	array<string> GetAppearance(string faction, string tier, string teamType)
	{
		string ty = "Infantry";
		if (teamType == "REC")
			ty = "Recon";
		else if (teamType == "PARA")
			ty = "Para";
		else if (teamType == "ARM")
			ty = "ArmorCrew";
		array<string> l;
		m_mAppearance.Find(faction + "|" + tier + "|" + ty, l);
		return l;
	}

	protected void ReadAppearance(LoadContext ctx)
	{
		int nf, nt, nk;
		if (!ctx.StartMap("aiAppearance", nf))
			return;
		string f, t, k;
		for (int i = 0; i < nf; i++)
		{
			if (!ctx.ReadMapKey(i, f) || !ctx.StartMap(f, nt))
				continue;
			for (int j = 0; j < nt; j++)
			{
				if (!ctx.ReadMapKey(j, t) || !ctx.StartMap(t, nk))
					continue;
				for (int m = 0; m < nk; m++)
				{
					if (!ctx.ReadMapKey(m, k))
						continue;
					array<string> l = {};
					if (ctx.ReadValue(k, l))
						m_mAppearance.Set(f + "|" + t + "|" + k, l);
				}
				ctx.EndMap();
			}
			ctx.EndMap();
		}
		ctx.EndMap();
	}

	//------------------------------------------------------------------------------------------------
	//! \param modPath catalog path inside the mod (used when no profile override exists)
	static F44_Catalog Get(string modPath = string.Empty)
	{
		if (!s_Instance)
		{
			s_Instance = new F44_Catalog();
			if (modPath.IsEmpty())
				modPath = MOD_PATH;
			s_Instance.Load(modPath);
		}
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	void Load(string modPath)
	{
		string path = modPath;
		if (FileIO.FileExists(PROFILE_PATH))
			path = PROFILE_PATH;

		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(path))
		{
			Print("[F44] Catalog: cannot load " + path, LogLevel.ERROR);
			return;
		}

		ReadFactions(ctx);
		ReadEntries(ctx, "catalog");
		ReadEntries(ctx, "items");
		ReadAppearance(ctx);

		int classes, vehicles, items;
		foreach (string k, F44_CatalogEntry e : m_mEntries)
		{
			if (e.m_bIsClass)
				classes++;
			else if (e.m_bIsItem)
				items++;
			else
				vehicles++;
		}
		PrintFormat("[F44] Catalog loaded from %1: %2 classes, %3 vehicles, %4 items, %5 unavailable", path, classes, vehicles, items, m_aUnavailable.Count());
		foreach (string missing : m_aUnavailable)
			Print("[F44] Catalog: key " + missing + " has no prefab (null) -> unavailable", LogLevel.WARNING);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadFactions(LoadContext ctx)
	{
		int count;
		if (!ctx.StartMap("factions", count))
			return;
		string key, value;
		for (int i = 0; i < count; i++)
		{
			if (!ctx.ReadMapKey(i, key))
				continue;
			if (ctx.ReadValue(key, value))
				m_mFactions.Set(key, value);
		}
		ctx.EndMap();
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadEntries(LoadContext ctx, string section)
	{
		int count;
		if (!ctx.StartMap(section, count))
			return;
		string key;
		for (int i = 0; i < count; i++)
		{
			if (!ctx.ReadMapKey(i, key))
				continue;
			if (!ctx.StartObject(key))
				continue;
			string prefab;
			int cost = 1;
			ctx.ReadValue("prefab", prefab);
			ctx.ReadValue("cost", cost);
			string category;
			ctx.ReadValue("category", category);
			string role;
			ctx.ReadValue("role", role);
			ctx.EndObject();

			if (prefab.IsEmpty() || prefab == "null")
			{
				m_aUnavailable.Insert(key);
				continue;
			}
			F44_CatalogEntry e = new F44_CatalogEntry();
			e.m_sKey = key;
			e.m_sPrefab = prefab;
			e.m_iCost = cost;
			e.m_bIsItem = section == "items";
			e.m_bIsClass = !e.m_bIsItem && prefab.Contains("Prefabs/Characters/");
			e.m_sCategory = category;
			e.m_sRole = role;
			m_mEntries.Set(key, e);
		}
		ctx.EndMap();
	}

	//------------------------------------------------------------------------------------------------
	string GetFactionKey(string sideKey)
	{
		string v;
		if (m_mFactions.Find(sideKey, v))
			return v;
		return sideKey;
	}

	//------------------------------------------------------------------------------------------------
	//! Reverse lookup: Reforger faction key -> side key
	string GetSideFromFaction(string factionKey)
	{
		foreach (string side, string fk : m_mFactions)
		{
			if (fk == factionKey)
				return side;
		}
		return factionKey;
	}

	//------------------------------------------------------------------------------------------------
	//! ("US", "RIFLEMAN") -> "US_RIFLEMAN"
	static string MakeClassKey(string sideKey, string classKey)
	{
		return sideKey + "_" + classKey;
	}

	//------------------------------------------------------------------------------------------------
	bool IsUnavailable(string key)
	{
		return m_aUnavailable.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetClassPrefab(string fullClassKey)
	{
		F44_CatalogEntry e;
		if (m_mEntries.Find(fullClassKey, e) && e.m_bIsClass)
			return e.m_sPrefab;
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetVehiclePrefab(string vehicleKey)
	{
		F44_CatalogEntry e;
		if (m_mEntries.Find(vehicleKey, e) && e.IsVehicle())
			return e.m_sPrefab;
		return ResourceName.Empty;
	}

	//------------------------------------------------------------------------------------------------
	int GetVehicleCost(string vehicleKey)
	{
		F44_CatalogEntry e;
		if (m_mEntries.Find(vehicleKey, e))
			return e.m_iCost;
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Class key (without side prefix) whose prefab matches the given prefab for one side
	string FindClassByPrefab(string sideKey, ResourceName prefab)
	{
		string guid = GuidOf(prefab);
		string prefix = sideKey + "_";
		foreach (string key, F44_CatalogEntry e : m_mEntries)
		{
			if (!e.m_bIsClass || !key.StartsWith(prefix))
				continue;
			if (GuidOf(e.m_sPrefab) == guid)
				return key.Substring(prefix.Length(), key.Length() - prefix.Length());
		}
		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! Any entry (class, vehicle, item, weapon...) by key
	F44_CatalogEntry GetEntry(string key)
	{
		F44_CatalogEntry e;
		m_mEntries.Find(key, e);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Weapon category for ribbons: catalog "category" first, else guessed from the prefab path
	string GetWeaponCategory(ResourceName prefab)
	{
		string guid = GuidOf(prefab);
		foreach (string key, F44_CatalogEntry e : m_mEntries)
		{
			if (!e.m_sCategory.IsEmpty() && GuidOf(e.m_sPrefab) == guid)
				return e.m_sCategory;
		}
		string p = prefab;
		p.ToLower();
		if (p.Contains("sniper") || p.Contains("scoped"))
			return "sniper";
		if (p.Contains("launcher") || p.Contains("bazooka") || p.Contains("panzerschreck") || p.Contains("panzerfaust") || p.Contains("/at/"))
			return "at";
		if (p.Contains("grenade") || p.Contains("explosive") || p.Contains("mine") || p.Contains("satchel"))
			return "explosive";
		if (p.Contains("machinegun") || p.Contains("/mg/") || p.Contains("lmg") || p.Contains("_bar") || p.Contains("mg42") || p.Contains("mg34"))
			return "lmg";
		if (p.Contains("smg") || p.Contains("submachine") || p.Contains("thompson") || p.Contains("mp40") || p.Contains("grease"))
			return "smg";
		if (p.Contains("vehicle"))
			return "vehicle";
		return "rifle";
	}

	//------------------------------------------------------------------------------------------------
	static string GuidOf(string resourceName)
	{
		int end = resourceName.IndexOf("}");
		if (resourceName.StartsWith("{") && end > 0)
			return resourceName.Substring(1, end - 1);
		return resourceName;
	}
}
