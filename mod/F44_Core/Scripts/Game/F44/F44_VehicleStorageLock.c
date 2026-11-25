// Arma & Heroes - no vehicle storage in battles (anti-hoarding): vehicle inventories/trunks cannot be
// opened nor receive items. Free crate launchers cannot go in any container (backpacks, boxes, vehicles).
// Only active when the world runs the F44 game mode. Bodies stay lootable.

class F44_VehicleStorageLock
{
	protected static int s_iLogged;

	static bool IsActive()
	{
		BaseGameMode gm = GetGame().GetGameMode();
		return gm && gm.FindComponent(F44_GameModeComponent);
	}

	static bool IsVehicleStorage(IEntity storageOwner)
	{
		if (!storageOwner)
			return false;
		// walk up: a character's own gear (even seated in a vehicle) is never vehicle storage
		IEntity e = storageOwner;
		while (e)
		{
			if (ChimeraCharacter.Cast(e))
				return false;
			if (Vehicle.Cast(e))
				return true;
			e = e.GetParent();
		}
		return false;
	}

	//! Called on each F44 vehicle spawn (server): logs the (now inaccessible) vehicle storages.
	static void Apply(IEntity veh)
	{
		if (!veh)
			return;
		InventoryStorageManagerComponent mgr = InventoryStorageManagerComponent.Cast(veh.FindComponent(InventoryStorageManagerComponent));
		if (!mgr)
		{
			PrintFormat("[F44] VehicleStorage: %1 has no storage manager", F44_FieldSupply.PrefabOf(veh).GetPath());
			return;
		}
		array<IEntity> items = {};
		mgr.GetItems(items);
		int removed;
		foreach (IEntity it : items)
		{
			if (!it || ChimeraCharacter.Cast(it))
				continue;
			removed++;	// kept: vehicle ammo racks live there; access is blocked instead
		}
		array<BaseInventoryStorageComponent> storages = {};
		mgr.GetStorages(storages);
		string kinds;
		foreach (BaseInventoryStorageComponent s : storages)
			kinds += s.ClassName() + " ";
		PrintFormat("[F44] VehicleStorage: locked %1 (%2 storages: %3; %4 items inside, access blocked)", F44_FieldSupply.PrefabOf(veh).GetPath(), storages.Count(), kinds, removed);
	}

	static void LogBlock(string what, IEntity owner)
	{
		if (s_iLogged > 40)
			return;
		s_iLogged++;
		PrintFormat("[F44] VehicleStorage: blocked %1 on %2", what, F44_FieldSupply.PrefabOf(owner.GetRootParent()).GetPath());
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_OpenVehicleStorageAction
{
	override bool CanBeShownScript(IEntity user)
	{
		if (F44_VehicleStorageLock.IsActive())
			return false;
		return super.CanBeShownScript(user);
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (F44_VehicleStorageLock.IsActive())
		{
			SetCannotPerformReason("Vehicle storage disabled");
			return false;
		}
		return super.CanBePerformedScript(user);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_OpenStorageAction
{
	override bool CanBeShownScript(IEntity user)
	{
		if (F44_VehicleStorageLock.IsActive() && F44_VehicleStorageLock.IsVehicleStorage(GetOwner()))
			return false;
		return super.CanBeShownScript(user);
	}
}

//------------------------------------------------------------------------------------------------
modded class SCR_UniversalInventoryStorageComponent
{
	override bool CanStoreItem(IEntity item, int slotID)
	{
		if (F44_VehicleStorageLock.IsActive())
		{
			IEntity owner = GetOwner();
			if (F44_VehicleStorageLock.IsVehicleStorage(owner))
			{
				if (Replication.IsServer())
					F44_VehicleStorageLock.LogBlock("item insert", owner);
				return false;
			}
			F44_FieldSupply fs = F44_FieldSupply.Get();
			if (fs && fs.IsFreeLauncher(item))
			{
				PrintFormat("[F44] FieldSupply: REFUSED storing free launcher in %1", F44_FieldSupply.PrefabOf(owner).GetPath());
				return false;
			}
		}
		return super.CanStoreItem(item, slotID);
	}
}
