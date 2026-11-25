// Arma & Heroes - free light vehicles (bicycles, civilian cars) near each side spawn and along roads.
// Free for everyone, capped per side, respawned when destroyed or abandoned far away (timers).

class F44_CivVehicleSlot
{
	string m_sSide;
	string m_sKind;	// BIKE | CAR
	vector m_vPos;
	float m_fYaw;
	IEntity m_Veh;
	float m_fNextSpawn;	// world time; 0 = now
	float m_fAwaySince = -1;
	int m_iSpawns;
	bool m_bAlive;
}

class F44_CivVehicleSpawner
{
	static const ResourceName BIKE = "{3FDBFE7F1A1EB8F2}Prefabs/Vehicles/Wheeled/WW2_Bike/WW2_Bike_base.et";	// WW2_Bicycle (Modest23)
	static const ref array<ResourceName> CARS = {
		"{23B8C20ACAEC164B}Prefabs/Vehicles/Wheeled/S1203/S1203_transport_white.et",
		"{321016E0F9361A22}Prefabs/Vehicles/Wheeled/S105/S105_lightgreen.et",
		"{1D96D899FA313A08}Prefabs/Vehicles/Wheeled/UAZ452/UAZ452_cargo_CIV_blue.et"
	};

	protected ref array<ref F44_CivVehicleSlot> m_aSlots = {};
	protected ref F44_FieldSupplyConfig m_Cfg;
	protected float m_fTime;

	//------------------------------------------------------------------------------------------------
	void Init(F44_BattleConfig cfg, F44_FieldSupplyConfig fs)
	{
		m_Cfg = fs;
		if (!m_Cfg || !m_Cfg.m_bCivVehicles)
		{
			Print("[F44] CivVehicles: disabled by battle config");
			return;
		}
		int explicit;
		foreach (F44_SupplyPoint p : m_Cfg.m_aPoints)
		{
			if (p.m_sType != "BIKE" && p.m_sType != "CAR")
				continue;
			AddSlot(p.m_sSide, p.m_sType, F44_FieldSupply.Ground(Vector(p.m_fX, 0, p.m_fZ)), p.m_fYaw);
			explicit++;
		}
		if (explicit == 0)
		{
			F44_Catalog cat = F44_Catalog.Get();
			FallbackSide(cfg.m_sAttacker, cat.GetFactionKey(cfg.m_sAttacker));
			FallbackSide(cfg.m_sDefender, cat.GetFactionKey(cfg.m_sDefender));
		}
		PrintFormat("[F44] CivVehicles: %1 slots (bikes/side %2, cars/side %3, respawn %4 s, abandon %5 m / %6 s), source=%7 explicit",
			m_aSlots.Count(), m_Cfg.m_iBikesPerSide, m_Cfg.m_iCarsPerSide, m_Cfg.m_fVehRespawnSec, m_Cfg.m_fVehAbandonDist, m_Cfg.m_fVehAbandonSec, explicit);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddSlot(string side, string kind, vector pos, float yaw)
	{
		F44_CivVehicleSlot s = new F44_CivVehicleSlot();
		s.m_sSide = side;
		s.m_sKind = kind;
		s.m_vPos = pos;
		s.m_fYaw = yaw;
		m_aSlots.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	//! Slots spread over the side spawns: on the nearest road when there is one, else on a ring.
	protected void FallbackSide(string side, string factionKey)
	{
		array<vector> anchors = {};
		foreach (SCR_SpawnPoint sp : SCR_SpawnPoint.GetSpawnPoints())
		{
			if (sp.IsSpawnPointEnabled() && sp.GetFactionKey() == factionKey)
				anchors.Insert(sp.GetOrigin());
		}
		if (anchors.IsEmpty())
		{
			PrintFormat("[F44] CivVehicles: no spawn point for %1", side);
			return;
		}
		int total = m_Cfg.m_iBikesPerSide + m_Cfg.m_iCarsPerSide;
		for (int i = 0; i < total; i++)
		{
			vector a = anchors[i % anchors.Count()];
			string kind = "BIKE";
			if (i >= m_Cfg.m_iBikesPerSide)
				kind = "CAR";
			float radius = 16 + 5 * (i / anchors.Count());
			vector p = F44_FieldSupply.Ring(a, i, total, radius, 2.2);
			float yaw = Math.RandomFloat(0, 360);
			vector road, dir;
			if (F44_Roads.Snap(p, 40, road, dir))
			{
				// road side, along the road
				vector side2 = Vector(dir[2], 0, -dir[0]);
				p = F44_FieldSupply.Ground(road + side2 * 3.5 + dir * (i * 1.5));
				yaw = dir.ToYaw();
			}
			AddSlot(side, kind, p, yaw);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity Spawn(F44_CivVehicleSlot s)
	{
		ResourceName rn = BIKE;
		if (s.m_sKind == "CAR")
			rn = CARS[(m_aSlots.Find(s) + s.m_iSpawns) % CARS.Count()];
		Resource res = Resource.Load(rn);
		if (!res || !res.IsValid())
		{
			PrintFormat("[F44] CivVehicles: invalid prefab %1 (mod missing?)", rn, level: LogLevel.ERROR);
			s.m_fNextSpawn = m_fTime + 600;
			return null;
		}
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(Vector(s.m_fYaw, 0, 0), params.Transform);
		params.Transform[3] = s.m_vPos + "0 0.3 0";
		IEntity v = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params);
		if (v)
		{
			s.m_iSpawns++;
			F44_VehicleStorageLock.Apply(v);
			PrintFormat("[F44] CivVehicles: spawned %1 for %2 at %3 (spawn #%4)", rn.GetPath(), s.m_sSide, s.m_vPos, s.m_iSpawns);
		}
		return v;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsDestroyed(IEntity v)
	{
		SCR_DamageManagerComponent dm = SCR_DamageManagerComponent.GetDamageManager(v);
		return dm && dm.GetState() == EDamageState.DESTROYED;
	}

	protected static bool HasOccupants(IEntity v)
	{
		BaseCompartmentManagerComponent cm = BaseCompartmentManagerComponent.Cast(v.FindComponent(BaseCompartmentManagerComponent));
		if (!cm)
			return false;
		array<BaseCompartmentSlot> slots = {};
		cm.GetCompartments(slots);
		foreach (BaseCompartmentSlot slot : slots)
		{
			if (slot && slot.GetOccupant())
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		if (!m_Cfg || !m_Cfg.m_bCivVehicles)
			return;
		m_fTime += dt;
		foreach (F44_CivVehicleSlot s : m_aSlots)
		{
			if (s.m_Veh)
			{
				string why;
				if (IsDestroyed(s.m_Veh))
					why = "destroyed";
				else if (!HasOccupants(s.m_Veh) && vector.Distance(s.m_Veh.GetOrigin(), s.m_vPos) > m_Cfg.m_fVehAbandonDist)
				{
					if (s.m_fAwaySince < 0)
						s.m_fAwaySince = m_fTime;
					else if (m_fTime - s.m_fAwaySince > m_Cfg.m_fVehAbandonSec)
						why = "abandoned";
				}
				else
					s.m_fAwaySince = -1;
				if (why.IsEmpty())
					continue;
				PrintFormat("[F44] CivVehicles: %1 of %2 %3, respawn in %4 s", s.m_sKind, s.m_sSide, why, m_Cfg.m_fVehRespawnSec);
				if (why == "abandoned")
					RplComponent.DeleteRplEntity(s.m_Veh, false);
				// a wreck stays where it is (no delete); the slot just gets a fresh vehicle later
				s.m_Veh = null;
				s.m_bAlive = false;
				s.m_fAwaySince = -1;
				s.m_fNextSpawn = m_fTime + m_Cfg.m_fVehRespawnSec;
				continue;
			}
			if (s.m_bAlive)
			{
				// vehicle deleted from outside (cleanup, admin): same respawn delay
				s.m_bAlive = false;
				s.m_fNextSpawn = m_fTime + m_Cfg.m_fVehRespawnSec;
				PrintFormat("[F44] CivVehicles: %1 of %2 gone, respawn in %3 s", s.m_sKind, s.m_sSide, m_Cfg.m_fVehRespawnSec);
				continue;
			}
			if (m_fTime < s.m_fNextSpawn)
				continue;
			s.m_Veh = Spawn(s);
			s.m_bAlive = s.m_Veh != null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Test hook: destroy every 2nd spawned vehicle to exercise the respawn path (orchestrator test only)
	void DebugKillHalf()
	{
		foreach (int i, F44_CivVehicleSlot s : m_aSlots)
		{
			if (s.m_Veh && i % 2 == 0)
			{
				PrintFormat("[F44] CivVehicles: TEST deleting %1 of %2", s.m_sKind, s.m_sSide);
				RplComponent.DeleteRplEntity(s.m_Veh, false);
			}
		}
	}
}
