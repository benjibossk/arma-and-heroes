// Arma & Heroes - user actions of the field supply boxes (prefabs Prefabs/F44/Supply/*.et).
// Timed actions (Duration in the prefab): holding the key repeats them, one step per completion.
// Effects run on the server only (F44_FieldSupply).

//------------------------------------------------------------------------------------------------
class F44_SupplyActionBase : ScriptedUserAction
{
	override bool CanBeShownScript(IEntity user)
	{
		return ChimeraCharacter.Cast(user) != null;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		CharacterControllerComponent cc = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		if (cc && cc.IsDead())
			return false;
		return true;
	}

	//! false: the action is broadcast/performed on the server
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	override bool CanBroadcastScript()
	{
		return false;
	}

	protected F44_FieldSupply Mgr()
	{
		if (!Replication.IsServer())
			return null;
		return F44_FieldSupply.Get();
	}
}

//------------------------------------------------------------------------------------------------
class F44_TakeLauncherAction : F44_SupplyActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		string side;
		PlayerController pc = GetGame().GetPlayerController();
		if (pc)
			side = F44_FieldSupply.SideOf(pc.GetControlledEntity());
		if (side == "US")
			outName = "Take Bazooka";
		else
			outName = "Take Panzerfaust 60";
		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;
		if (F44_FieldSupply.CarriesLauncher(user))
		{
			SetCannotPerformReason("Already carrying one");
			return false;
		}
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		F44_FieldSupply m = Mgr();
		if (!m)
			return;
		string reason;
		if (!m.TakeFromCrate(pOwnerEntity, pUserEntity, false, reason))
			PrintFormat("[F44] FieldSupply: take launcher refused: %1", reason);
	}
}

//------------------------------------------------------------------------------------------------
class F44_TakeATGrenadeAction : F44_SupplyActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Take AT grenade";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		F44_FieldSupply m = Mgr();
		if (!m)
			return;
		string reason;
		if (!m.TakeFromCrate(pOwnerEntity, pUserEntity, true, reason))
			PrintFormat("[F44] FieldSupply: take AT grenade refused: %1", reason);
	}
}

//------------------------------------------------------------------------------------------------
class F44_ResupplyAction : F44_SupplyActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Resupply ammo (hold)";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		F44_FieldSupply m = Mgr();
		if (m)
			m.Resupply(pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
class F44_HealAction : F44_SupplyActionBase
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Treat wounds (hold)";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		F44_FieldSupply m = Mgr();
		if (m)
			m.Heal(pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
class F44_LoadBoxOnVehicleAction : F44_SupplyActionBase
{
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;
		return GetOwner() && !GetOwner().GetParent();
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Load box on nearest vehicle";
		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		F44_FieldSupply m = Mgr();
		if (m)
			m.LoadOnVehicle(pOwnerEntity, pUserEntity);
	}
}
