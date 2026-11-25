// Arma & Heroes - persistent per-player equipment (server only).
// Source: BattleConfig.playerLoadouts[identityId], or for late joiners
//   GET {apiUrl}/api/v1/battles/{battleId}/loadout/{identityId}
// The character is built from catalog keys only: never from client data.
// Battle arsenals are disabled (players keep their loadout); picking up weapons on the ground stays allowed.

//------------------------------------------------------------------------------------------------
class F44_LoadoutRequest
{
	string m_sIdentityId;
	ref RestCallback m_Callback;
	F44_LoadoutService m_Service;

	void OnSuccess(RestCallback cb)
	{
		if (m_Service)
			m_Service.OnFetched(this, cb.GetData());
	}

	void OnError(RestCallback cb)
	{
		if (m_Service)
			m_Service.OnFetched(this, string.Empty);
	}
}

//------------------------------------------------------------------------------------------------
class F44_LoadoutService
{
	protected ref map<string, ref F44_PlayerLoadout> m_mLoadouts = new map<string, ref F44_PlayerLoadout>();
	protected ref array<ref F44_LoadoutRequest> m_aPending = {};
	protected string m_sApiUrl;
	protected string m_sToken;
	protected string m_sBattleId;

	//------------------------------------------------------------------------------------------------
	void Init(F44_BattleConfig cfg, string apiUrl, string token, string battleId)
	{
		m_sApiUrl = apiUrl;
		m_sToken = token;
		m_sBattleId = battleId;
		foreach (string id, F44_PlayerLoadout l : cfg.m_mLoadouts)
		{
			m_mLoadouts.Set(id, l);
			Print("[F44] Loadout " + id.Substring(0, Math.Min(8, id.Length())) + "..: " + Describe(l));
		}
		PrintFormat("[F44] Loadouts: %1 player loadouts from BattleConfig", m_mLoadouts.Count());
		// self-test without players: resolve the sample loadouts against the catalog (log only)
		if (m_mLoadouts.IsEmpty())
		{
			F44_BattleConfig sample = F44_BattleConfig.FromFile("Configs/F44/battle_sample.json");
			if (sample)
			{
				foreach (string sid, F44_PlayerLoadout sl : sample.m_mLoadouts)
					Print("[F44] Loadout selftest " + sid + ": " + Describe(sl));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "class RIFLEMAN, US_M1_GARAND=Rifle_M1.et, GER_X=UNRESOLVED" (unit-level check without a player)
	static string Describe(F44_PlayerLoadout l)
	{
		string s = "class " + l.m_sClass;
		F44_Catalog cat = F44_Catalog.Get();
		foreach (string key : l.m_aItems)
		{
			F44_CatalogEntry e = cat.GetEntry(key);
			string res = "UNRESOLVED";
			if (e && !e.m_bIsClass)
			{
				res = e.m_sPrefab;
				int slash = res.LastIndexOf("/");
				if (slash >= 0)
					res = res.Substring(slash + 1, res.Length() - slash - 1);
			}
			s += ", " + key + "=" + res;
		}
		return s;
	}

	//------------------------------------------------------------------------------------------------
	F44_PlayerLoadout Get(string identityId)
	{
		F44_PlayerLoadout l;
		m_mLoadouts.Find(identityId, l);
		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! Late joiner: ask the site if the battle config did not contain this player
	void Fetch(string identityId)
	{
		if (identityId.IsEmpty() || m_mLoadouts.Contains(identityId) || m_sApiUrl.IsEmpty() || m_sBattleId.IsEmpty())
			return;
		RestContext ctx = F44_BattleLoader.GetRestContext(m_sApiUrl, m_sToken);
		if (!ctx)
			return;
		F44_LoadoutRequest r = new F44_LoadoutRequest();
		r.m_sIdentityId = identityId;
		r.m_Service = this;
		r.m_Callback = new RestCallback();
		r.m_Callback.SetOnSuccess(r.OnSuccess);
		r.m_Callback.SetOnError(r.OnError);
		m_aPending.Insert(r);
		ctx.GET(r.m_Callback, "/api/v1/battles/" + m_sBattleId + "/loadout/" + identityId);
	}

	//------------------------------------------------------------------------------------------------
	void OnFetched(F44_LoadoutRequest r, string data)
	{
		F44_PlayerLoadout l = F44_PlayerLoadout.FromString(r.m_sIdentityId, data);
		if (l)
		{
			m_mLoadouts.Set(r.m_sIdentityId, l);
			Print("[F44] Loadout fetched for a late joiner (class " + l.m_sClass + ")");
		}
		m_aPending.RemoveItem(r);
	}

	//------------------------------------------------------------------------------------------------
	//! Base = class prefab already spawned; then catalog items replace / complete the equipment
	static void Apply(IEntity character, F44_PlayerLoadout l)
	{
		if (!character || !l)
			return;
		Print("[F44] Loadout applied at spawn: " + Describe(l));
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;
		F44_Catalog cat = F44_Catalog.Get();

		// replace the class primary weapon when the loadout brings its own one
		bool hasWeapon;
		foreach (string key : l.m_aItems)
		{
			F44_CatalogEntry e = cat.GetEntry(key);
			if (e && e.m_sPrefab.Contains("Prefabs/Weapons/") && !e.m_sPrefab.Contains("Magazines") && !e.m_sPrefab.Contains("Grenades"))
				hasWeapon = true;
		}
		if (hasWeapon)
		{
			BaseWeaponManagerComponent wm = BaseWeaponManagerComponent.Cast(character.FindComponent(BaseWeaponManagerComponent));
			if (wm)
			{
				array<IEntity> weapons = {};
				wm.GetWeaponsList(weapons);
				foreach (IEntity w : weapons)
				{
					if (w)
						inv.TryDeleteItem(w);
				}
			}
		}

		foreach (string key2 : l.m_aItems)
		{
			F44_CatalogEntry e2 = cat.GetEntry(key2);
			if (!e2 || e2.m_bIsClass)
			{
				Print("[F44] Loadout: item " + key2 + " unknown or unavailable in catalog", LogLevel.WARNING);
				continue;
			}
			if (!inv.TrySpawnPrefabToStorage(e2.m_sPrefab))
				Print("[F44] Loadout: no room for " + key2, LogLevel.WARNING);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Base arsenals/crates are disabled during an Arma & Heroes battle: players keep their own loadout.
//! (Ground pickup / looting is untouched.)
modded class SCR_ArsenalComponent
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer())
			GetGame().GetCallqueue().CallLater(F44_DisableIfBattle, 2000, false);
	}

	protected void F44_DisableIfBattle()
	{
		if (F44_GameModeComponent.GetInstance())
			SetArsenalEnabled(false);
	}
}
