// Arma & Heroes - P3 in-battle seat enforcement for the account classes (GAP_PLAN.md), server side.
// Checked every tick on the pool vehicles (and air layer planes):
//  - tanks (catalog role in armorRoles, default "tank" / "tank_destroyer"): driver, gunner and commander seats are for the
//    CREW class family only; anybody else is ejected (cargo / passenger seats stay free);
//  - aircraft (catalog type "AIR"): the pilot seat is for the PILOT family; when no team of the side allows a pilot
//    class (no pilot class in the catalog yet) the CREW family may fly instead. Gunner seats stay free.
// Battle JSON (optional): "seatRules": {"enabled": true, "armorRoles": ["tank", "tank_destroyer"]}

//------------------------------------------------------------------------------------------------
class F44_SeatRulesConfig
{
	bool m_bEnabled = true;
	ref array<string> m_aArmorRoles = {"tank", "tank_destroyer"};

	void Read(LoadContext root)
	{
		if (!root || !root.StartObject("seatRules"))
			return;
		root.ReadValue("enabled", m_bEnabled);
		root.ReadValue("armorRoles", m_aArmorRoles);
		root.EndObject();
	}
}

//------------------------------------------------------------------------------------------------
class F44_SeatRules
{
	static ref F44_SeatRulesConfig s_Cfg = new F44_SeatRulesConfig();
	protected static ref map<int, string> s_mFamily = new map<int, string>();	// player -> class family

	//------------------------------------------------------------------------------------------------
	static void Reset()
	{
		s_mFamily.Clear();
	}

	//------------------------------------------------------------------------------------------------
	static void OnDeployed(int pid, string classKey)
	{
		s_mFamily.Set(pid, F44_Maintenance.ClassFamily(classKey));
	}

	//------------------------------------------------------------------------------------------------
	static void Watch(IEntity veh, string key)
	{
		if (!s_Cfg.m_bEnabled || !veh)
			return;
		F44_Catalog cat = F44_Catalog.Get();
		bool air = cat.GetVehicleType(key) == "AIR";
		bool armor = !air && s_Cfg.m_aArmorRoles.Contains(cat.GetVehicleRole(key));
		if (!air && !armor)
			return;
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(veh.FindComponent(BaseCompartmentManagerComponent));
		if (!cm)
			return;
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		PlayerManager pm = GetGame().GetPlayerManager();
		foreach (BaseCompartmentSlot sl : slots)
		{
			IEntity occ = sl.GetOccupant();
			if (!occ || sl.GetType() == ECompartmentType.CARGO)
				continue;
			if (air && sl.GetType() != ECompartmentType.PILOT)
				continue;
			int pid = pm.GetPlayerIdFromControlledEntity(occ);
			if (pid <= 0)
				continue;	// AI crews are not restricted
			string fam;
			s_mFamily.Find(pid, fam);
			if (armor && fam != "CREW")
				F44_Rules.Eject(pid, "Tank seats are for tank crew soldiers. Take a passenger seat or deploy as crew.");
			else if (air && fam != "PILOT" && !(fam == "CREW" && !HasPilotClass()))
				F44_Rules.Eject(pid, "Only pilots can fly this aircraft.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! true when the catalog knows a pilot class
	protected static bool HasPilotClass()
	{
		F44_Catalog cat = F44_Catalog.Get();
		return !cat.GetClassPrefab("US_PILOT").IsEmpty() || !cat.GetClassPrefab("GER_PILOT").IsEmpty();
	}
}
