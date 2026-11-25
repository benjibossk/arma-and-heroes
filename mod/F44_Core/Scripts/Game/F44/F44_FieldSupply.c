// Arma & Heroes - free field supply (H&G style): AT crates, ammo boxes, med boxes.
// Server side manager, ticked by F44_GameModeComponent. Settings: battle JSON "fieldSupply" object,
// optional points in the battle JSON ("fieldSupply.supplyCrates") or in layouts.json
// (layouts.<id>.supplyCrates / civSpawns), else fallback around spawns and sectors.

//------------------------------------------------------------------------------------------------
class F44_SupplyPoint
{
	string m_sType;	// AT | AMMO | MED | BIKE | CAR
	string m_sSide;	// side key (US/GER) or empty = both
	float m_fX;
	float m_fZ;
	float m_fYaw;

	static void ReadArray(LoadContext n, string key, array<ref F44_SupplyPoint> outPts)
	{
		int c;
		if (!n.StartArray(key, c))
			return;
		for (int i = 0; i < c; i++)
		{
			if (!n.StartObject())
				break;
			F44_SupplyPoint p = new F44_SupplyPoint();
			n.ReadValue("type", p.m_sType);
			n.ReadValue("side", p.m_sSide);
			n.ReadValue("x", p.m_fX);
			n.ReadValue("z", p.m_fZ);
			n.ReadValue("yaw", p.m_fYaw);
			p.m_sType.ToUpper();
			outPts.Insert(p);
			n.EndObject();
		}
		n.EndArray();
	}
}

//------------------------------------------------------------------------------------------------
class F44_FieldSupplyConfig
{
	bool m_bEnabled = true;
	int m_iCratesPerSector = 1;	// AT crates next to each sector (both factions' launchers)
	int m_iCratesPerSpawn = 1;	// AT crates next to each side spawn (side launcher)
	int m_iLaunchersPerCrate = 4;
	int m_iGrenadesPerCrate = 4;
	float m_fCrateRefillMin = 5;
	int m_iAmmoBoxesPerSpawn = 1;
	int m_iMedBoxesPerSpawn = 1;
	int m_iAmmoBoxCharges = 40;	// 1 charge = 1 magazine
	int m_iMedBoxCharges = 20;	// 1 charge = 1 heal step
	float m_fBoxRegenPerMin = 4;	// charges regained per minute
	float m_fHealStep = 0.25;	// scaled health per heal step
	float m_fLooseLauncherSec = 120;	// free launcher dropped on the ground: removed after this delay

	bool m_bCivVehicles = true;
	int m_iBikesPerSide = 4;
	int m_iCarsPerSide = 2;
	float m_fVehRespawnSec = 120;	// delay before a destroyed/abandoned slot respawns
	float m_fVehAbandonDist = 250;	// abandoned = empty and farther than this from its slot ...
	float m_fVehAbandonSec = 180;	// ... for this long

	ref array<ref F44_SupplyPoint> m_aPoints = {};

	//------------------------------------------------------------------------------------------------
	static F44_FieldSupplyConfig Read(LoadContext root)
	{
		F44_FieldSupplyConfig c = new F44_FieldSupplyConfig();
		if (!root || !root.StartObject("fieldSupply"))
			return c;
		root.ReadValue("enabled", c.m_bEnabled);
		root.ReadValue("cratesPerSector", c.m_iCratesPerSector);
		root.ReadValue("cratesPerSpawn", c.m_iCratesPerSpawn);
		root.ReadValue("launchersPerCrate", c.m_iLaunchersPerCrate);
		root.ReadValue("grenadesPerCrate", c.m_iGrenadesPerCrate);
		root.ReadValue("crateRefillMin", c.m_fCrateRefillMin);
		root.ReadValue("ammoBoxesPerSpawn", c.m_iAmmoBoxesPerSpawn);
		root.ReadValue("medBoxesPerSpawn", c.m_iMedBoxesPerSpawn);
		root.ReadValue("ammoBoxCharges", c.m_iAmmoBoxCharges);
		root.ReadValue("medBoxCharges", c.m_iMedBoxCharges);
		root.ReadValue("boxRegenPerMin", c.m_fBoxRegenPerMin);
		root.ReadValue("healStep", c.m_fHealStep);
		root.ReadValue("looseLauncherSec", c.m_fLooseLauncherSec);
		root.ReadValue("civVehicles", c.m_bCivVehicles);
		root.ReadValue("bikesPerSide", c.m_iBikesPerSide);
		root.ReadValue("carsPerSide", c.m_iCarsPerSide);
		root.ReadValue("vehRespawnSec", c.m_fVehRespawnSec);
		root.ReadValue("vehAbandonDist", c.m_fVehAbandonDist);
		root.ReadValue("vehAbandonSec", c.m_fVehAbandonSec);
		F44_SupplyPoint.ReadArray(root, "supplyCrates", c.m_aPoints);
		F44_SupplyPoint.ReadArray(root, "civSpawns", c.m_aPoints);
		root.EndObject();
		return c;
	}

	//------------------------------------------------------------------------------------------------
	//! Extra points from layouts.json (same resolution as F44_CityLayout.Resolve)
	void ReadLayoutPoints(string layoutId, string cityId)
	{
		string path = F44_CityLayout.PROFILE_FILE;
		if (!FileIO.FileExists(path))
			path = F44_CityLayout.MOD_FILE;
		JsonLoadContext ctx = new JsonLoadContext();
		if (!ctx.LoadFromFile(path))
			return;
		string id = layoutId;
		if (id.IsEmpty() && !cityId.IsEmpty() && ctx.StartObject("cityMap"))
		{
			ctx.ReadValue(cityId, id);
			ctx.EndObject();
		}
		if (id.IsEmpty())
			id = cityId;
		if (id.IsEmpty() || !ctx.StartObject("layouts"))
			return;
		if (!ctx.StartObject(id))
			return;
		int before = m_aPoints.Count();
		F44_SupplyPoint.ReadArray(ctx, "supplyCrates", m_aPoints);
		F44_SupplyPoint.ReadArray(ctx, "civSpawns", m_aPoints);
		ctx.EndObject();
		ctx.EndObject();
		if (m_aPoints.Count() > before)
			PrintFormat("[F44] FieldSupply: %1 points from layout '%2'", m_aPoints.Count() - before, id);
	}
}

//------------------------------------------------------------------------------------------------
enum F44_ESupplyBoxType
{
	AT,
	AMMO,
	MED
}

//------------------------------------------------------------------------------------------------
class F44_SupplyBox
{
	IEntity m_Ent;
	F44_ESupplyBoxType m_eType;
	string m_sSide;	// AT crate: launcher side ("" = both)
	vector m_vPos;
	float m_fAngle;
	// AT crate stock
	int m_iLaunchers;
	int m_iGrenades;
	float m_fNextRefill;
	// ammo / med charges
	int m_iCharges;
	int m_iMaxCharges;
	float m_fRegenAccu;
	int m_iOwnerId;	// player who loaded it on a vehicle (credited); 0 = none
	bool m_bVehicleCrate;	// fitted on a pool vehicle by the web team config: dies with it, never respawned on the ground
}

//------------------------------------------------------------------------------------------------
class F44_FreeLauncher
{
	IEntity m_Ent;
	float m_fLooseSince = -1;
}

//------------------------------------------------------------------------------------------------
class F44_FieldSupply
{
	static const float SEA_MIN_Y = 0.2;	// ground below this = sea / harbour water (Arland sea level 0)
	static const ResourceName PREFAB_AT = "{6F44A7C2E1B30001}Prefabs/F44/Supply/F44_Crate_AT.et";
	static const ResourceName PREFAB_AMMO = "{6F44A7C2E1B30002}Prefabs/F44/Supply/F44_SupplyBox_Ammo.et";
	static const ResourceName PREFAB_MED = "{6F44A7C2E1B30003}Prefabs/F44/Supply/F44_SupplyBox_Med.et";

	static const ResourceName PZF = "{15F0473A1454A801}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust60.et";
	static const ResourceName PZF100 = "{EADCBEFD2705B515}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust100.et";
	static const ResourceName BAZOOKA = "{FBBB0FC51FC85173}Prefabs/Weapons/Launchers/M1A1/Launcher_M1A1_Bazooka.et";
	static const ResourceName GER_AT_GRENADE = "{886DA8E3CFF0C54F}Prefabs/Weapons/Grenades/GeballteLadung/Grenade_GeballteLadung.et";
	static const ResourceName US_AT_GRENADE = "{F832D80F885FD79A}Prefabs/Weapons/Grenades/M7AT/Grenade_M7AT.et";

	protected static F44_FieldSupply s_Instance;

	protected ref F44_FieldSupplyConfig m_Cfg;
	protected ref array<ref F44_SupplyBox> m_aBoxes = {};
	protected ref array<ref F44_FreeLauncher> m_aLaunchers = {};
	protected F44_GameModeComponent m_GameMode;
	protected string m_sAtt;
	protected string m_sDef;
	protected float m_fTime;
	protected float m_fLogAccu;

	//------------------------------------------------------------------------------------------------
	static F44_FieldSupply Get()
	{
		return s_Instance;
	}

	F44_FieldSupplyConfig GetConfig()
	{
		return m_Cfg;
	}

	bool IsReady()
	{
		return m_Cfg != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Web team config: fit a crate (AMMO | MED | AT) on a freshly spawned pool vehicle (logistics point). Server only.
	//! side = faction key of the vehicle's team (AT crate launcher side).
	bool AttachToVehicle(IEntity veh, string crate, string side)
	{
		if (!veh || !m_Cfg)
		{
			PrintFormat("[F44] VEHCRATE crate=%1 side=%2 fail (no vehicle / supply not ready)", crate, side, level: LogLevel.WARNING);
			return false;
		}
		F44_ESupplyBoxType type;
		if (crate == "AMMO")
			type = F44_ESupplyBoxType.AMMO;
		else if (crate == "MED")
			type = F44_ESupplyBoxType.MED;
		else if (crate == "AT")
			type = F44_ESupplyBoxType.AT;
		else
		{
			PrintFormat("[F44] VEHCRATE crate=%1 side=%2 fail (unknown crate type)", crate, side, level: LogLevel.WARNING);
			return false;
		}
		F44_SupplyBox b = SpawnBox(type, veh.GetOrigin() + Vector(0, 3, 0), 0, side);
		if (!b)
		{
			PrintFormat("[F44] VEHCRATE crate=%1 side=%2 veh=%3 fail (box spawn)", crate, side, PrefabOf(veh).GetPath(), level: LogLevel.WARNING);
			return false;
		}
		b.m_bVehicleCrate = true;
		vector mins, maxs;
		veh.GetBounds(mins, maxs);
		vector local[4];
		Math3D.MatrixIdentity4(local);
		local[3] = Vector(0, maxs[1] * 0.5, mins[2] * 0.6);
		veh.AddChild(b.m_Ent, -1, EAddChildFlags.AUTO_TRANSFORM);
		b.m_Ent.SetLocalTransform(local);
		PrintFormat("[F44] VEHCRATE crate=%1 side=%2 veh=%3 pos=%4 ok (attached)", crate, side, PrefabOf(veh).GetPath(), veh.GetOrigin());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	float GetTime()
	{
		return m_fTime;
	}

	//------------------------------------------------------------------------------------------------
	void Init(F44_GameModeComponent gm, F44_BattleConfig cfg)
	{
		s_Instance = this;
		m_GameMode = gm;
		m_Cfg = cfg.m_FieldSupply;
		if (!m_Cfg)
			m_Cfg = new F44_FieldSupplyConfig();
		m_Cfg.ReadLayoutPoints(cfg.m_sLayoutId, cfg.m_sCityId);
		m_sAtt = cfg.m_sAttacker;
		m_sDef = cfg.m_sDefender;
		if (!m_Cfg.m_bEnabled)
		{
			Print("[F44] FieldSupply: disabled by battle config");
			return;
		}
		SpawnAll();
	}

	//------------------------------------------------------------------------------------------------
	static vector Ground(vector p)
	{
		p[1] = GetGame().GetWorld().GetSurfaceY(p[0], p[2]);
		return p;
	}

	//------------------------------------------------------------------------------------------------
	//! Points around a center, on a ring, snapped to ground
	static vector Ring(vector c, int idx, int n, float radius, float phase)
	{
		float a = phase + Math.PI2 * idx / Math.Max(1, n);
		vector p = c + Vector(Math.Cos(a) * radius, 0, Math.Sin(a) * radius);
		return Ground(p);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnAll()
	{
		int explicit;
		foreach (F44_SupplyPoint p : m_Cfg.m_aPoints)
		{
			vector pos = Ground(Vector(p.m_fX, 0, p.m_fZ));
			if (p.m_sType == "AT")
				SpawnBox(F44_ESupplyBoxType.AT, pos, p.m_fYaw, p.m_sSide);
			else if (p.m_sType == "AMMO")
				SpawnBox(F44_ESupplyBoxType.AMMO, pos, p.m_fYaw, p.m_sSide);
			else if (p.m_sType == "MED")
				SpawnBox(F44_ESupplyBoxType.MED, pos, p.m_fYaw, p.m_sSide);
			else
				continue;
			explicit++;
		}

		if (explicit == 0)
		{
			// fallback: around the enabled side spawns ...
			F44_Catalog cat = F44_Catalog.Get();
			string fAtt = cat.GetFactionKey(m_sAtt);
			string fDef = cat.GetFactionKey(m_sDef);
			foreach (SCR_SpawnPoint sp : SCR_SpawnPoint.GetSpawnPoints())
			{
				if (!sp.IsSpawnPointEnabled())
					continue;
				string side;
				if (sp.GetFactionKey() == fAtt)
					side = m_sAtt;
				else if (sp.GetFactionKey() == fDef)
					side = m_sDef;
				else
					continue;
				vector c = sp.GetOrigin();
				int n = m_Cfg.m_iCratesPerSpawn + m_Cfg.m_iAmmoBoxesPerSpawn + m_Cfg.m_iMedBoxesPerSpawn;
				int k;
				for (int i = 0; i < m_Cfg.m_iCratesPerSpawn; i++)
					SpawnBox(F44_ESupplyBoxType.AT, Ring(c, k++, n, 9, 0.4), 0, side);
				for (int i = 0; i < m_Cfg.m_iAmmoBoxesPerSpawn; i++)
					SpawnBox(F44_ESupplyBoxType.AMMO, Ring(c, k++, n, 9, 0.4), 0, side);
				for (int i = 0; i < m_Cfg.m_iMedBoxesPerSpawn; i++)
					SpawnBox(F44_ESupplyBoxType.MED, Ring(c, k++, n, 9, 0.4), 0, side);
			}
			// ... and next to each sector (both launchers), offset out of the capture center
			for (int s = 0; s < F44_SectorManager.Count(); s++)
			{
				F44_SectorComponent sec = F44_SectorManager.GetSector(s);
				if (!sec)
					continue;
				for (int i = 0; i < m_Cfg.m_iCratesPerSector; i++)
					SpawnBox(F44_ESupplyBoxType.AT, Ring(sec.GetOwner().GetOrigin(), i, m_Cfg.m_iCratesPerSector, 18, 1.1 + s), 0, "");
			}
		}

		int at, ammo, med;
		foreach (F44_SupplyBox b : m_aBoxes)
		{
			if (b.m_eType == F44_ESupplyBoxType.AT)
				at++;
			else if (b.m_eType == F44_ESupplyBoxType.AMMO)
				ammo++;
			else
				med++;
		}
		PrintFormat("[F44] FieldSupply: spawned %1 AT crates (%2 launchers + %3 AT grenades each), %4 ammo boxes (%5 charges), %6 med boxes (%7 charges), refill %8 min, source=%9",
			at, m_Cfg.m_iLaunchersPerCrate, m_Cfg.m_iGrenadesPerCrate, ammo, m_Cfg.m_iAmmoBoxCharges, med, m_Cfg.m_iMedBoxCharges, m_Cfg.m_fCrateRefillMin, explicit.ToString() + " explicit points");
	}

	//------------------------------------------------------------------------------------------------
	protected F44_SupplyBox SpawnBox(F44_ESupplyBoxType type, vector pos, float yaw, string side)
	{
		ResourceName rn = PREFAB_AT;
		if (type == F44_ESupplyBoxType.AMMO)
			rn = PREFAB_AMMO;
		else if (type == F44_ESupplyBoxType.MED)
			rn = PREFAB_MED;
		if (pos[1] < SEA_MIN_Y)
		{
			PrintFormat("[F44] CRATE rejected type=%1 side=%2 pos=%3 (below sea level)", typename.EnumToString(F44_ESupplyBoxType, type), side, pos, level: LogLevel.WARNING);
			return null;
		}
		Resource res = Resource.Load(rn);
		if (!res || !res.IsValid())
		{
			Print("[F44] FieldSupply: invalid prefab " + rn, LogLevel.ERROR);
			return null;
		}
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(Vector(yaw, 0, 0), params.Transform);
		params.Transform[3] = pos;
		IEntity e = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		PrintFormat("[F44] CRATE type=%1 side=%2 prefab=%3 pos=%4 -> %5", typename.EnumToString(F44_ESupplyBoxType, type), side, rn.GetPath(), pos, e != null);
		if (!e)
			return null;
		F44_SupplyBox b = new F44_SupplyBox();
		b.m_Ent = e;
		b.m_eType = type;
		b.m_sSide = side;
		b.m_vPos = pos;
		b.m_fAngle = yaw;
		b.m_iLaunchers = m_Cfg.m_iLaunchersPerCrate;
		b.m_iGrenades = m_Cfg.m_iGrenadesPerCrate;
		b.m_fNextRefill = m_fTime + m_Cfg.m_fCrateRefillMin * 60;
		if (type == F44_ESupplyBoxType.AMMO)
			b.m_iMaxCharges = m_Cfg.m_iAmmoBoxCharges;
		else if (type == F44_ESupplyBoxType.MED)
			b.m_iMaxCharges = m_Cfg.m_iMedBoxCharges;
		b.m_iCharges = b.m_iMaxCharges;
		m_aBoxes.Insert(b);
		PrintFormat("[F44] FieldSupply: %1 box at %2 side '%3'", typename.EnumToString(F44_ESupplyBoxType, type), pos, side);
		return b;
	}

	//------------------------------------------------------------------------------------------------
	F44_SupplyBox FindBox(IEntity ent)
	{
		foreach (F44_SupplyBox b : m_aBoxes)
		{
			if (b.m_Ent == ent)
				return b;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	static string SideOf(IEntity user)
	{
		SCR_ChimeraCharacter ch = SCR_ChimeraCharacter.Cast(user);
		if (!ch)
			return "";
		Faction f = ch.GetFaction();
		if (!f)
			return "";
		F44_Catalog cat = F44_Catalog.Get();
		if (s_Instance)
		{
			if (cat.GetFactionKey(s_Instance.m_sAtt) == f.GetFactionKey())
				return s_Instance.m_sAtt;
			if (cat.GetFactionKey(s_Instance.m_sDef) == f.GetFactionKey())
				return s_Instance.m_sDef;
		}
		string fk = f.GetFactionKey();
		fk.ToUpper();
		if (fk.Contains("US"))
			return "US";
		return "GER";
	}

	//------------------------------------------------------------------------------------------------
	static bool IsFreeLauncherPrefab(ResourceName rn)
	{
		return rn == PZF || rn == PZF100 || rn == BAZOOKA;
	}

	//------------------------------------------------------------------------------------------------
	static ResourceName PrefabOf(IEntity e)
	{
		if (!e)
			return ResourceName.Empty;
		EntityPrefabData pd = e.GetPrefabData();
		if (!pd)
			return ResourceName.Empty;
		return pd.GetPrefabName();
	}

	//------------------------------------------------------------------------------------------------
	static bool CarriesLauncher(IEntity user)
	{
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return false;
		array<IEntity> items = {};
		inv.GetItems(items);
		foreach (IEntity it : items)
		{
			if (IsFreeLauncherPrefab(PrefabOf(it)))
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool IsFreeLauncher(IEntity item)
	{
		foreach (F44_FreeLauncher l : m_aLaunchers)
		{
			if (l.m_Ent == item)
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! AT crate: take a launcher (grenade=false) or an AT grenade. Server only. Returns a log/reason string.
	bool TakeFromCrate(IEntity crate, IEntity user, bool grenade, out string reason)
	{
		F44_SupplyBox b = FindBox(crate);
		if (!b || b.m_eType != F44_ESupplyBoxType.AT)
		{
			reason = "not a crate";
			return false;
		}
		int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		string side = SideOf(user);
		if (side.IsEmpty())
			side = b.m_sSide;
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
		{
			reason = "no inventory";
			return false;
		}
		ResourceName rn;
		if (grenade)
		{
			if (b.m_iGrenades <= 0)
			{
				reason = "crate empty (grenades)";
				return false;
			}
			rn = GER_AT_GRENADE;
			if (side == "US")
				rn = US_AT_GRENADE;
		}
		else
		{
			if (b.m_iLaunchers <= 0)
			{
				reason = "crate empty (launchers)";
				return false;
			}
			if (CarriesLauncher(user))
			{
				reason = "already carries a launcher (max 1)";
				PrintFormat("[F44] FieldSupply: REFUSED launcher for player %1: %2", pid, reason);
				return false;
			}
			rn = PZF;
			if (side == "US")
				rn = BAZOOKA;
		}
		// spawn next to the user then insert (we keep the entity to track it)
		Resource res = Resource.Load(rn);
		if (!res || !res.IsValid())
		{
			reason = "invalid prefab " + rn;
			return false;
		}
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		params.Transform[3] = user.GetOrigin() + "0 1 0";
		IEntity item = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		if (!item)
		{
			reason = "spawn failed";
			return false;
		}
		bool stored = inv.TryInsertItem(item, EStoragePurpose.PURPOSE_WEAPON_PROXY);
		if (!stored)
			stored = inv.TryInsertItem(item);
		if (grenade)
			b.m_iGrenades--;
		else
		{
			b.m_iLaunchers--;
			F44_FreeLauncher fl = new F44_FreeLauncher();
			fl.m_Ent = item;
			m_aLaunchers.Insert(fl);
		}
		PrintFormat("[F44] FieldSupply: player %1 took %2 (stored=%3) from AT crate, left %4 launchers / %5 grenades",
			pid, rn.GetPath(), stored, b.m_iLaunchers, b.m_iGrenades);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Ammo box: one magazine for the current weapon (or its first rifle). Server only.
	bool Resupply(IEntity boxEnt, IEntity user)
	{
		F44_SupplyBox b = FindBox(boxEnt);
		if (!b || b.m_iCharges <= 0)
		{
			PrintFormat("[F44] FieldSupply: resupply refused (box empty)");
			return false;
		}
		ResourceName mag;
		BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(user.FindComponent(BaseWeaponManagerComponent));
		if (wm)
		{
			array<IEntity> weapons = {};
			wm.GetWeaponsList(weapons);
			BaseWeaponComponent cur = wm.GetCurrentWeapon();
			if (cur)
				weapons.InsertAt(cur.GetOwner(), 0);
			foreach (IEntity w : weapons)
			{
				if (!w)
					continue;
				BaseWeaponComponent wc = BaseWeaponComponent.Cast(w.FindComponent(BaseWeaponComponent));
				if (!wc || IsFreeLauncherPrefab(PrefabOf(w)))
					continue;
				BaseMagazineComponent cm = wc.GetCurrentMagazine();
				if (cm)
					mag = PrefabOf(cm.GetOwner());
				if (mag.IsEmpty())
				{
					BaseMuzzleComponent mz = wc.GetCurrentMuzzle();
					if (mz)
						mag = mz.GetDefaultMagazineOrProjectileName();
				}
				if (!mag.IsEmpty())
					break;
			}
		}
		if (mag.IsEmpty())
			return false;
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(user.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || !inv.TrySpawnPrefabToStorage(mag))
		{
			PrintFormat("[F44] FieldSupply: resupply refused (inventory full) %1", mag.GetPath());
			return false;
		}
		b.m_iCharges--;
		int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		Credit(b, pid, true);
		PrintFormat("[F44] FieldSupply: player %1 resupplied %2 (box %3/%4, owner %5)", pid, mag.GetPath(), b.m_iCharges, b.m_iMaxCharges, b.m_iOwnerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Med box: one heal step (stop bleeding + m_fHealStep scaled health). Server only.
	bool Heal(IEntity boxEnt, IEntity user)
	{
		F44_SupplyBox b = FindBox(boxEnt);
		if (!b || b.m_iCharges <= 0)
		{
			Print("[F44] FieldSupply: heal refused (box empty)");
			return false;
		}
		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(user.FindComponent(SCR_CharacterDamageManagerComponent));
		if (!dmg)
			return false;
		HitZone hz = dmg.GetDefaultHitZone();
		float before = 1;
		if (hz)
			before = hz.GetHealthScaled();
		bool bleeding = dmg.IsBleeding();
		if (before >= 0.999 && !bleeding)
			return false;
		dmg.RemoveAllBleedings();
		if (hz)
			hz.SetHealthScaled(Math.Min(1, before + m_Cfg.m_fHealStep));
		if (hz && hz.GetHealthScaled() >= 0.999)
			dmg.FullHeal();
		b.m_iCharges--;
		int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		Credit(b, pid, false);
		PrintFormat("[F44] FieldSupply: player %1 healed %2 -> %3 (box %4/%5, owner %6)", pid, before, Math.Min(1, before + m_Cfg.m_fHealStep), b.m_iCharges, b.m_iMaxCharges, b.m_iOwnerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Credit(F44_SupplyBox b, int userId, bool ammo)
	{
		if (!m_GameMode || b.m_iOwnerId <= 0 || b.m_iOwnerId == userId)
			return;
		if (ammo)
			m_GameMode.F44_AddLogistics(b.m_iOwnerId, 1, 0);
		else
			m_GameMode.F44_AddLogistics(b.m_iOwnerId, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	static Vehicle FindVehicleNear(vector pos, float radius)
	{
		Vehicle best;
		float bestD = radius;
		array<IEntity> found = {};
		s_aQuery = found;
		GetGame().GetWorld().QueryEntitiesBySphere(pos, radius, QueryAddVehicle, null, EQueryEntitiesFlags.DYNAMIC);
		foreach (IEntity e : found)
		{
			Vehicle v = Vehicle.Cast(e);
			if (!v)
				continue;
			float d = vector.Distance(v.GetOrigin(), pos);
			if (d <= bestD)
			{
				bestD = d;
				best = v;
			}
		}
		s_aQuery = null;
		return best;
	}

	protected static array<IEntity> s_aQuery;
	protected static bool QueryAddVehicle(IEntity e)
	{
		if (s_aQuery && Vehicle.Cast(e) && !s_aQuery.Contains(e))
			s_aQuery.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Load an ammo/med box on the nearest vehicle (becomes a mobile logistics point). Server only.
	bool LoadOnVehicle(IEntity boxEnt, IEntity user)
	{
		F44_SupplyBox b = FindBox(boxEnt);
		if (!b || b.m_eType == F44_ESupplyBoxType.AT || boxEnt.GetParent())
			return false;
		Vehicle v = FindVehicleNear(boxEnt.GetOrigin(), 10);
		if (!v)
		{
			Print("[F44] FieldSupply: load refused (no vehicle within 10 m)");
			return false;
		}
		// on the vehicle, near its rear, slightly above
		vector mins, maxs;
		v.GetBounds(mins, maxs);
		vector local[4];
		Math3D.MatrixIdentity4(local);
		local[3] = Vector(0, maxs[1] * 0.5, mins[2] * 0.6);
		v.AddChild(boxEnt, -1, EAddChildFlags.AUTO_TRANSFORM);
		boxEnt.SetLocalTransform(local);
		b.m_iOwnerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		PrintFormat("[F44] FieldSupply: player %1 loaded %2 box on %3", b.m_iOwnerId, typename.EnumToString(F44_ESupplyBoxType, b.m_eType), PrefabOf(v).GetPath());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (!m_Cfg || !m_Cfg.m_bEnabled)
			return;
		m_fTime += dt;
		float regen = m_Cfg.m_fBoxRegenPerMin * dt / 60;
		for (int i = m_aBoxes.Count() - 1; i >= 0; i--)
		{
			F44_SupplyBox b = m_aBoxes[i];
			if (!b.m_Ent && b.m_bVehicleCrate)
			{
				// lost with its vehicle: the respawned vehicle gets a new one
				m_aBoxes.Remove(i);
				continue;
			}
			if (!b.m_Ent)
			{
				// destroyed: respawn at its original place on next refill
				if (m_fTime >= b.m_fNextRefill)
				{
					PrintFormat("[F44] FieldSupply: respawning destroyed %1 box", typename.EnumToString(F44_ESupplyBoxType, b.m_eType));
					m_aBoxes.Remove(i);
					SpawnBox(b.m_eType, b.m_vPos, b.m_fAngle, b.m_sSide);
				}
				continue;
			}
			if (b.m_eType == F44_ESupplyBoxType.AT)
			{
				if (m_fTime >= b.m_fNextRefill)
				{
					b.m_fNextRefill = m_fTime + m_Cfg.m_fCrateRefillMin * 60;
					if (b.m_iLaunchers < m_Cfg.m_iLaunchersPerCrate || b.m_iGrenades < m_Cfg.m_iGrenadesPerCrate)
					{
						PrintFormat("[F44] FieldSupply: AT crate refilled %1/%2 launchers, %3/%4 grenades -> full",
							b.m_iLaunchers, m_Cfg.m_iLaunchersPerCrate, b.m_iGrenades, m_Cfg.m_iGrenadesPerCrate);
						b.m_iLaunchers = m_Cfg.m_iLaunchersPerCrate;
						b.m_iGrenades = m_Cfg.m_iGrenadesPerCrate;
					}
				}
			}
			else if (b.m_iCharges < b.m_iMaxCharges)
			{
				b.m_fRegenAccu += regen;
				if (b.m_fRegenAccu >= 1)
				{
					int add = Math.Floor(b.m_fRegenAccu);
					b.m_fRegenAccu -= add;
					b.m_iCharges = Math.Min(b.m_iMaxCharges, b.m_iCharges + add);
					PrintFormat("[F44] FieldSupply: %1 box regen -> %2/%3", typename.EnumToString(F44_ESupplyBoxType, b.m_eType), b.m_iCharges, b.m_iMaxCharges);
				}
			}
		}
		CheckLaunchers(dt);
	}

	//------------------------------------------------------------------------------------------------
	//! Free launchers must stay on a character: removed when stored in any container or left on the ground.
	protected void CheckLaunchers(float dt)
	{
		for (int i = m_aLaunchers.Count() - 1; i >= 0; i--)
		{
			F44_FreeLauncher l = m_aLaunchers[i];
			if (!l.m_Ent)
			{
				m_aLaunchers.Remove(i);	// fired (single use) or deleted
				continue;
			}
			IEntity parent = l.m_Ent.GetParent();
			if (parent && ChimeraCharacter.Cast(parent))
			{
				l.m_fLooseSince = -1;
				continue;
			}
			if (parent)
			{
				PrintFormat("[F44] FieldSupply: free launcher found in container %1: removed", PrefabOf(parent.GetRootParent()).GetPath());
				RplComponent.DeleteRplEntity(l.m_Ent, false);
				m_aLaunchers.Remove(i);
				continue;
			}
			if (l.m_fLooseSince < 0)
				l.m_fLooseSince = m_fTime;
			else if (m_fTime - l.m_fLooseSince > m_Cfg.m_fLooseLauncherSec)
			{
				Print("[F44] FieldSupply: free launcher left on the ground: removed");
				RplComponent.DeleteRplEntity(l.m_Ent, false);
				m_aLaunchers.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Bots-only self test, only when $profile:F44/fieldsupply_test.txt exists (dev servers).
	protected float m_fTestT;
	protected int m_iTestStep;
	void SelfTest(float dt, array<IEntity> botsAtt, array<IEntity> botsDef, F44_CivVehicleSpawner civ)
	{
		if (m_iTestStep < 0 || m_aBoxes.IsEmpty())
			return;
		if (m_iTestStep == 0 && !FileIO.FileExists("$profile:F44/fieldsupply_test.txt"))
		{
			m_iTestStep = -1;
			return;
		}
		m_fTestT += dt;
		IEntity bot;
		foreach (IEntity e : botsAtt)
		{
			if (e && !bot)
				bot = e;
		}
		if (!bot)
			return;
		F44_SupplyBox at, ammo, med;
		foreach (F44_SupplyBox b : m_aBoxes)
		{
			if (!b.m_Ent)
				continue;
			if (b.m_eType == F44_ESupplyBoxType.AT && !at)
				at = b;
			if (b.m_eType == F44_ESupplyBoxType.AMMO && !ammo)
				ammo = b;
			if (b.m_eType == F44_ESupplyBoxType.MED && !med)
				med = b;
		}
		string reason;
		if (m_iTestStep == 0 && m_fTestT > 30 && at)
		{
			m_iTestStep = 1;
			Print("[F44] TEST FieldSupply: step 1 take/refuse launcher");
			bool ok1 = TakeFromCrate(at.m_Ent, bot, false, reason);
			bool ok2 = TakeFromCrate(at.m_Ent, bot, false, reason);
			PrintFormat("[F44] TEST FieldSupply: first take=%1, second take=%2 (%3)", ok1, ok2, reason);
			bool g = TakeFromCrate(at.m_Ent, bot, true, reason);
			PrintFormat("[F44] TEST FieldSupply: grenade take=%1, carries launcher=%2", g, CarriesLauncher(bot));
			if (ammo)
				PrintFormat("[F44] TEST FieldSupply: resupply=%1", Resupply(ammo.m_Ent, bot));
			if (med)
			{
				SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(bot.FindComponent(SCR_CharacterDamageManagerComponent));
				if (dmg && dmg.GetDefaultHitZone())
					dmg.GetDefaultHitZone().SetHealthScaled(0.5);
				PrintFormat("[F44] TEST FieldSupply: heal=%1", Heal(med.m_Ent, bot));
			}
			return;
		}
		if (m_iTestStep == 1 && m_fTestT > 45)
		{
			m_iTestStep = 2;
			Print("[F44] TEST FieldSupply: step 2 load box on vehicle + storage lock");
			Vehicle v = FindVehicleNear(bot.GetOrigin(), 400);
			if (!v)
				v = FindVehicleNear(med.m_vPos, 400);
			if (v && med)
			{
				med.m_Ent.SetOrigin(v.GetOrigin() + "0 0 4");
				PrintFormat("[F44] TEST FieldSupply: load=%1", LoadOnVehicle(med.m_Ent, bot));
				med.m_iOwnerId = 999;	// fake owner to exercise the logistics credit
				IEntity bot2 = bot;
				if (botsAtt.Count() > 1 && botsAtt[1])
					bot2 = botsAtt[1];
				SCR_CharacterDamageManagerComponent dmg2 = SCR_CharacterDamageManagerComponent.Cast(bot2.FindComponent(SCR_CharacterDamageManagerComponent));
				if (dmg2 && dmg2.GetDefaultHitZone())
					dmg2.GetDefaultHitZone().SetHealthScaled(0.4);
				PrintFormat("[F44] TEST FieldSupply: heal from vehicle box=%1 (box parent %2)", Heal(med.m_Ent, bot2), PrefabOf(med.m_Ent.GetParent()).GetPath());
				// storage lock: try to put the free launcher in the vehicle storage
				InventoryStorageManagerComponent vm = InventoryStorageManagerComponent.Cast(v.FindComponent(InventoryStorageManagerComponent));
				IEntity launcher;
				foreach (F44_FreeLauncher fl : m_aLaunchers)
				{
					if (fl.m_Ent)
						launcher = fl.m_Ent;
				}
				if (vm && launcher)
					PrintFormat("[F44] TEST FieldSupply: insert free launcher into vehicle storage=%1", vm.TryInsertItem(launcher));
				if (vm)
				{
					Resource r = Resource.Load(GER_AT_GRENADE);
					EntitySpawnParams sp = new EntitySpawnParams();
					sp.TransformMode = ETransformMode.WORLD;
					sp.Transform[3] = v.GetOrigin() + "0 3 0";
					IEntity g2 = GetGame().SpawnEntityPrefab(r, GetGame().GetWorld(), sp);
					PrintFormat("[F44] TEST FieldSupply: insert grenade into vehicle storage=%1", vm.TryInsertItem(g2));
				}
			}
			else
				Print("[F44] TEST FieldSupply: no vehicle found for load test");
			return;
		}
		if (m_iTestStep == 2 && m_fTestT > 60)
		{
			m_iTestStep = 3;
			if (civ)
				civ.DebugKillHalf();
			if (at)
			{
				string r2;
				if (!botsDef.IsEmpty() && botsDef[0])
					TakeFromCrate(at.m_Ent, botsDef[0], true, r2);
				PrintFormat("[F44] TEST FieldSupply: AT crate now %1 launchers / %2 grenades, waiting refill", at.m_iLaunchers, at.m_iGrenades);
			}
			return;
		}
		if (m_iTestStep == 3 && m_fTestT > 200)
		{
			m_iTestStep = -1;
			Print("[F44] TEST FieldSupply: done");
		}
	}
}
