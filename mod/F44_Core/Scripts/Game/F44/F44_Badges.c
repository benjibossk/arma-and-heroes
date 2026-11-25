// Arma & Heroes - P2-G combat badges (GAP_PLAN.md), server side effects.
// The web sends playerLoadouts[id].badge / badgeLevel (1..3, web/badges.go); one badge per soldier:
//  - FIRST_AID    : +level field dressings at each deploy (after the medical kit);
//  - COMBAT_MEDIC : medical items used 15 % faster per level (F44_Medical.c usage speed);
//  - HEAVY_SET    : 5 % less damage taken per level.

//------------------------------------------------------------------------------------------------
class F44_Badges
{
	protected static ref map<int, string> s_mBadge = new map<int, string>();
	protected static ref map<int, int> s_mLevel = new map<int, int>();

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mBadge.Clear();
		s_mLevel.Clear();
	}

	//------------------------------------------------------------------------------------------------
	static void OnDeployed(int pid, IEntity character, F44_PlayerLoadout lo)
	{
		s_mBadge.Remove(pid);
		s_mLevel.Remove(pid);
		if (!lo || lo.m_sBadge.IsEmpty() || lo.m_iBadgeLevel <= 0)
			return;
		int lv = Math.Min(lo.m_iBadgeLevel, 3);
		s_mBadge.Set(pid, lo.m_sBadge);
		s_mLevel.Set(pid, lv);
		PrintFormat("[F44] BADGE player=%1 %2 level %3", pid, lo.m_sBadge, lv);
		if (lo.m_sBadge == "FIRST_AID")
			GetGame().GetCallqueue().CallLater(GiveDressings, 3000, false, character, lv);
	}

	//------------------------------------------------------------------------------------------------
	//! badge level worn by the player controlling this character (0 = none)
	static int LevelOf(IEntity character, string badge)
	{
		if (!character)
			return 0;
		int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(character);
		string b;
		if (pid <= 0 || !s_mBadge.Find(pid, b) || b != badge)
			return 0;
		int lv;
		s_mLevel.Find(pid, lv);
		return lv;
	}

	//------------------------------------------------------------------------------------------------
	static float UseSpeedFactor(IEntity character)
	{
		return 1 + 0.15 * LevelOf(character, "COMBAT_MEDIC");
	}

	//------------------------------------------------------------------------------------------------
	static float DamageFactor(IEntity character)
	{
		return 1 - 0.05 * LevelOf(character, "HEAVY_SET");
	}

	//------------------------------------------------------------------------------------------------
	protected static void GiveDressings(IEntity character, int n)
	{
		if (!character)
			return;
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return;
		ResourceName dressing = F44_Medical.DRESSING_US;
		if (F44_Medical.IsGerman(character))
			dressing = F44_Medical.DRESSING_GER;
		int ok;
		for (int i = 0; i < n; i++)
		{
			if (inv.TrySpawnPrefabToStorage(dressing))
				ok++;
		}
		PrintFormat("[F44] BADGE first aid: +%1 dressings", ok);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_CharacterDamageManagerComponent
{
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		if (Replication.IsServer() && damageContext.damageValue > 0)
		{
			float f = F44_Badges.DamageFactor(GetOwner());
			if (f < 1)
				damageContext.damageValue = damageContext.damageValue * f;
		}
		return super.HijackDamageHandling(damageContext);
	}
}
