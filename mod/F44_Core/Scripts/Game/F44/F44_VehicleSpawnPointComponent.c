// Arma & Heroes - vehicle spawn slot. Holds at most one vehicle at a time.
// The game mode component feeds it from the side vehicle pool.
[ComponentEditorProps(category: "F44", description: "Arma & Heroes vehicle spawn slot")]
class F44_VehicleSpawnPointComponentClass : ScriptComponentClass
{
}

class F44_VehicleSpawnPointComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.ComboBox, desc: "Side using this slot", enums: ParamEnumArray.FromEnum(F44_ESectorOwner))]
	F44_ESectorOwner m_eSide;

	[Attribute("", desc: "Optional: only these vehicle keys (empty = any)")]
	ref array<string> m_aAllowedKeys;

	[Attribute("10", desc: "Delay before refilling the slot after the vehicle left/was destroyed (s)")]
	float m_fRespawnDelay;

	protected static ref array<F44_VehicleSpawnPointComponent> s_aAll = {};

	// runtime (server)
	IEntity m_Vehicle;
	string m_sVehicleKey;
	string m_sTeamId;
	float m_fNextSpawnTime;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		s_aAll.Insert(this);
	}

	override void OnDelete(IEntity owner)
	{
		s_aAll.RemoveItem(this);
	}

	//------------------------------------------------------------------------------------------------
	static array<F44_VehicleSpawnPointComponent> GetAll()
	{
		return s_aAll;
	}

	//------------------------------------------------------------------------------------------------
	bool AcceptsKey(string key)
	{
		if (!m_aAllowedKeys || m_aAllowedKeys.IsEmpty())
			return true;
		return m_aAllowedKeys.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! True when the slot is free: no vehicle, or vehicle moved more than 15 m away
	bool IsFree()
	{
		if (!m_Vehicle)
			return true;
		return vector.Distance(m_Vehicle.GetOrigin(), GetOwner().GetOrigin()) > 15;
	}

	//------------------------------------------------------------------------------------------------
	IEntity SpawnVehicle(ResourceName prefab)
	{
		Resource res = Resource.Load(prefab);
		if (!res || !res.IsValid())
		{
			Print("[F44] Vehicle prefab invalid: " + prefab, LogLevel.ERROR);
			return null;
		}
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetOwner().GetWorldTransform(params.Transform);
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
	}
}
