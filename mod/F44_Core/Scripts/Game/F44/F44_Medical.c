// Arma & Heroes - medical model (see docs/MEDICAL.md).
// Arcade-ish on top of the vanilla Reforger medical system:
//  - every soldier is topped up to N field dressings at spawn (players and AI), medics get more + morphine + saline
//  - bandage = stop bleeding (vanilla) + small heal; a medic treating someone else heals him to full, faster
//  - unconsciousness enabled; only a medic can revive (any of his items on an unconscious teammate); bleed-out timer
//  - per-player counters for the BattleResult (healGiven, revives, stats.bandagesApplied, stats.healHp)
// Config: defaults below, optional override $profile:F44/medical.json (same keys).

//------------------------------------------------------------------------------------------------
class F44_MedicalConfig
{
	int m_iBandages = 2;			// field dressings for everybody
	int m_iMedicBandages = 6;		// field dressings for medics
	int m_iMedicMorphine = 2;
	int m_iMedicSaline = 1;
	float m_fBandageHeal = 20;		// health points healed by any bandage (on top of stopping the bleeding)
	bool m_bMedicFullHeal = true;	// a medic treating another soldier heals him to full
	float m_fMedicSpeed = 2.0;		// medic usage speed factor (vanilla divider, 1 = normal)
	bool m_bUnconscious = true;		// game mode health setting
	bool m_bMedicRevive = true;		// only medics can revive
	int m_iBleedOutSec = 60;		// unconscious longer than this -> dead (0 = vanilla)

	void Load()
	{
		string path = "$profile:F44/medical.json";
		if (!FileIO.FileExists(path))
			return;
		SCR_JsonLoadContext ctx = new SCR_JsonLoadContext();
		if (!ctx.LoadFromFile(path))
			return;
		ctx.ReadValue("bandages", m_iBandages);
		ctx.ReadValue("medicBandages", m_iMedicBandages);
		ctx.ReadValue("medicMorphine", m_iMedicMorphine);
		ctx.ReadValue("medicSaline", m_iMedicSaline);
		ctx.ReadValue("bandageHeal", m_fBandageHeal);
		ctx.ReadValue("medicFullHeal", m_bMedicFullHeal);
		ctx.ReadValue("medicSpeed", m_fMedicSpeed);
		ctx.ReadValue("unconscious", m_bUnconscious);
		ctx.ReadValue("medicRevive", m_bMedicRevive);
		ctx.ReadValue("bleedOutSec", m_iBleedOutSec);
		Print("[F44][MED] config override loaded from " + path);
	}
}

//------------------------------------------------------------------------------------------------
class F44_MedStats
{
	int m_iHealGiven;		// treatments on other soldiers (ribbon contract: healGiven)
	int m_iRevives;
	int m_iBandagesApplied;	// bandages on other soldiers
	int m_iHealHp;			// health points restored on others
}

//------------------------------------------------------------------------------------------------
class F44_Medical
{
	static const ResourceName DRESSING_US = "{9070F5AB358FE739}Prefabs/Items/Medicine/FieldDressing_01/CarlisleFieldDressing_US_WW2.et";
	static const ResourceName DRESSING_GER = "{4DC939F9FA9D5349}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_HEER_WW2.et";
	static const ResourceName MORPHINE = "{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et";
	static const ResourceName SALINE = "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et";

	protected static ref F44_MedicalConfig s_Cfg;
	protected static ref map<int, ref F44_MedStats> s_mStats = new map<int, ref F44_MedStats>();
	protected static ref map<int, string> s_mPlayerClass = new map<int, string>();
	protected static bool s_bGameModeDone;

	//------------------------------------------------------------------------------------------------
	static F44_MedicalConfig Cfg()
	{
		if (!s_Cfg)
		{
			s_Cfg = new F44_MedicalConfig();
			s_Cfg.Load();
			PrintFormat("[F44][MED] rules: bandages=%1 medic(bandages=%2 morphine=%3 saline=%4 speed=%5 fullHeal=%6) bandageHeal=%7 unconscious=%8 medicRevive=%9",
				s_Cfg.m_iBandages, s_Cfg.m_iMedicBandages, s_Cfg.m_iMedicMorphine, s_Cfg.m_iMedicSaline, s_Cfg.m_fMedicSpeed, s_Cfg.m_bMedicFullHeal,
				s_Cfg.m_fBandageHeal, s_Cfg.m_bUnconscious, s_Cfg.m_bMedicRevive);
			PrintFormat("[F44][MED] bleedOutSec=%1", s_Cfg.m_iBleedOutSec);
		}
		return s_Cfg;
	}

	static bool IsActive()
	{
		return F44_GameModeComponent.GetInstance() != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Hook from the spawn handler: remembers the catalog class of the player (US_MEDIC, GER_MEDIC...)
	static void SetPlayerClass(int playerId, string classKey)
	{
		s_mPlayerClass.Set(playerId, classKey);
	}

	static int PlayerIdOf(IEntity e)
	{
		if (!e)
			return 0;
		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e);
	}

	static bool IsMedic(IEntity e)
	{
		if (!e)
			return false;
		int pid = PlayerIdOf(e);
		string cls;
		if (pid > 0 && s_mPlayerClass.Find(pid, cls))
			return cls.Contains("MEDIC");
		EntityPrefabData pd = e.GetPrefabData();
		if (!pd)
			return false;
		string p = pd.GetPrefabName();
		p.ToLower();
		return p.Contains("medic");
	}

	static bool IsGerman(IEntity e)
	{
		EntityPrefabData pd = e.GetPrefabData();
		if (!pd)
			return false;
		string p = pd.GetPrefabName();
		return p.Contains("HEER") || p.Contains("GER") || p.Contains("OPFOR");
	}

	//------------------------------------------------------------------------------------------------
	//! Game mode health settings (once)
	static void ApplyGameModeSettings()
	{
		if (s_bGameModeDone)
			return;
		BaseGameMode gm = GetGame().GetGameMode();
		if (!gm)
			return;
		s_bGameModeDone = true;
		SCR_GameModeHealthSettings hs = SCR_GameModeHealthSettings.Cast(gm.FindComponent(SCR_GameModeHealthSettings));
		if (hs)
		{
			hs.SetUnconsciousnessPermitted(Cfg().m_bUnconscious);
			PrintFormat("[F44][MED] game mode health settings: unconscious=%1 bleedingScale=%2 regenScale=%3", hs.IsUnconsciousnessPermitted(), hs.GetBleedingScale(), hs.GetRegenScale());
		}
		else
			Print("[F44][MED] no SCR_GameModeHealthSettings on the game mode, unconsciousness set per character");
	}

	//------------------------------------------------------------------------------------------------
	//! Server: top the soldier up to his medical kit (idempotent, also fine after the persistent loadout)
	static void GiveKit(IEntity character)
	{
		if (!character || !IsActive())
			return;
		ApplyGameModeSettings();
		SCR_CharacterDamageManagerComponent dm = SCR_CharacterDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(character));
		if (dm && Cfg().m_bUnconscious)
			dm.SetPermitUnconsciousness(true, false);

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;
		bool medic = IsMedic(character);
		int wantB = Cfg().m_iBandages;
		int wantM = 0;
		int wantS = 0;
		if (medic)
		{
			wantB = Cfg().m_iMedicBandages;
			wantM = Cfg().m_iMedicMorphine;
			wantS = Cfg().m_iMedicSaline;
		}
		int hasB = Count(inv, SCR_EConsumableType.BANDAGE);
		int hasM = Count(inv, SCR_EConsumableType.MORPHINE);
		int hasS = Count(inv, SCR_EConsumableType.SALINE);
		ResourceName dressing = DRESSING_US;
		if (IsGerman(character))
			dressing = DRESSING_GER;
		int addB = Spawn(inv, dressing, wantB - hasB);
		int addM = Spawn(inv, MORPHINE, wantM - hasM);
		int addS = Spawn(inv, SALINE, wantS - hasS);
		PrintFormat("[F44][MED] kit %1 pid=%2 medic=%3: bandages %4+%5 morphine %6+%7 saline %8+%9",
			FilePath.StripExtension(FilePath.StripPath(character.GetPrefabData().GetPrefabName())), PlayerIdOf(character), medic, hasB, addB, hasM, addM, hasS, addS);
	}

	protected static int Count(SCR_InventoryStorageManagerComponent inv, SCR_EConsumableType t)
	{
		array<IEntity> items = {};
		inv.GetItems(items);
		int n;
		foreach (IEntity it : items)
		{
			SCR_ConsumableItemComponent c = SCR_ConsumableItemComponent.Cast(it.FindComponent(SCR_ConsumableItemComponent));
			if (c && c.GetConsumableType() == t)
				n++;
		}
		return n;
	}

	protected static int Spawn(SCR_InventoryStorageManagerComponent inv, ResourceName prefab, int n)
	{
		int ok;
		for (int i = 0; i < n; i++)
		{
			if (inv.TrySpawnPrefabToStorage(prefab))
				ok++;
		}
		return ok;
	}

	//------------------------------------------------------------------------------------------------
	static F44_MedStats Stats(int pid)
	{
		F44_MedStats s;
		if (!s_mStats.Find(pid, s))
		{
			s = new F44_MedStats();
			s_mStats.Set(pid, s);
		}
		return s;
	}

	//! Fields appended to the player object of the BattleResult (docs/RIBBONS.md)
	static string ResultJson(int pid)
	{
		F44_MedStats s;
		if (!s_mStats.Find(pid, s))
			return "";
		string q = "\"";
		string j = "," + q + "healGiven" + q + ":" + s.m_iHealGiven.ToString();
		j += "," + q + "revives" + q + ":" + s.m_iRevives.ToString();
		j += "," + q + "stats" + q + ":{";
		j += q + "bandagesApplied" + q + ":" + s.m_iBandagesApplied.ToString();
		j += "," + q + "healHp" + q + ":" + s.m_iHealHp.ToString() + "}";
		return j;
	}

	//------------------------------------------------------------------------------------------------
	//! Called (server) after a medical item effect was applied
	static void OnTreated(IEntity target, IEntity user, SCR_EConsumableType type, float hpBefore, bool wasUnconscious)
	{
		SCR_CharacterDamageManagerComponent dm = SCR_CharacterDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(target));
		if (!dm)
			return;
		bool medic = IsMedic(user);
		bool other = target != user;

		if (type == SCR_EConsumableType.BANDAGE && Cfg().m_fBandageHeal > 0)
			dm.HealHitZones(Cfg().m_fBandageHeal);
		if (medic && other && Cfg().m_bMedicFullHeal)
			dm.FullHeal();

		bool revived;
		if (wasUnconscious && medic && other && Cfg().m_bMedicRevive)
		{
			dm.FullHeal();
			dm.UpdateConsciousness();
			ChimeraCharacter ch = ChimeraCharacter.Cast(target);
			if (ch && ch.GetCharacterController())
				ch.GetCharacterController().SetUnconscious(false);
			revived = true;
		}

		float hpAfter = dm.GetHealth();
		int gained = (int)Math.Max(0, hpAfter - hpBefore);
		int upid = PlayerIdOf(user);
		PrintFormat("[F44][MED] treat type=%1 user=%2(pid %3 medic=%4) target=%5 other=%6 hp %7->%8 revived=%9",
			typename.EnumToString(SCR_EConsumableType, type), user, upid, medic, target, other, (int)hpBefore, (int)hpAfter, revived);
		if (!other || upid <= 0)
			return;
		F44_MedStats s = Stats(upid);
		s.m_iHealGiven++;
		s.m_iHealHp += gained;
		if (type == SCR_EConsumableType.BANDAGE)
			s.m_iBandagesApplied++;
		if (revived)
			s.m_iRevives++;
	}

	//------------------------------------------------------------------------------------------------
	static void BleedOutCheck(IEntity target)
	{
		ChimeraCharacter ch = ChimeraCharacter.Cast(target);
		if (!ch || !ch.GetCharacterController())
			return;
		if (ch.GetCharacterController().GetLifeState() != ECharacterLifeState.INCAPACITATED)
			return;
		SCR_CharacterDamageManagerComponent dm = SCR_CharacterDamageManagerComponent.Cast(ch.GetDamageManager());
		if (!dm)
			return;
		PrintFormat("[F44][MED] bleed-out after %1 s: %2", Cfg().m_iBleedOutSec, target);
		dm.Kill(dm.GetInstigator());
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_ConsumableEffectHealthItems
{
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		if (!Replication.IsServer() || !F44_Medical.IsActive())
		{
			super.ApplyEffect(target, user, item, animParams);
			return;
		}
		float hp;
		bool uncon;
		SCR_CharacterDamageManagerComponent dm = SCR_CharacterDamageManagerComponent.Cast(SCR_DamageManagerComponent.GetDamageManager(target));
		if (dm)
			hp = dm.GetHealth();
		ChimeraCharacter ch = ChimeraCharacter.Cast(target);
		if (ch && ch.GetCharacterController())
			uncon = ch.GetCharacterController().GetLifeState() == ECharacterLifeState.INCAPACITATED;
		super.ApplyEffect(target, user, item, animParams);
		F44_Medical.OnTreated(target, user, m_eConsumableType, hp, uncon);
	}

	override float GetUsageSpeedFactor(notnull SCR_ChimeraCharacter character)
	{
		float f = super.GetUsageSpeedFactor(character);
		if (F44_Medical.IsActive() && F44_Medical.IsMedic(character))
			return Math.Max(f, F44_Medical.Cfg().m_fMedicSpeed);
		return f;
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_CharacterDamageManagerComponent
{
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (Replication.IsServer() && ChimeraCharacter.Cast(owner) && GetGame().InPlayMode())
			GetGame().GetCallqueue().CallLater(F44_Medical.GiveKit, 1500, false, owner);
	}

	override void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP)
	{
		super.OnLifeStateChanged(previousLifeState, newLifeState, isJIP);
		if (!Replication.IsServer() || !F44_Medical.IsActive())
			return;
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
		{
			PrintFormat("[F44][MED] unconscious: %1 (bleed-out in %2 s)", GetOwner(), F44_Medical.Cfg().m_iBleedOutSec);
			if (F44_Medical.Cfg().m_iBleedOutSec > 0)
				GetGame().GetCallqueue().CallLater(F44_Medical.BleedOutCheck, F44_Medical.Cfg().m_iBleedOutSec * 1000, false, GetOwner());
		}
		else if (previousLifeState == ECharacterLifeState.INCAPACITATED && newLifeState == ECharacterLifeState.ALIVE)
			PrintFormat("[F44][MED] woke up: %1", GetOwner());
	}
}
