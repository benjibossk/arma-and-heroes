// Arma & Heroes - capture sectors.
// F44_SectorComponent: placed on any world entity (see Prefabs/F44/F44_Sector.et) = one sector.
// F44_SectorManager: static registry + capture simulation (server, driven by F44_GameModeComponent).
// Replicated state (owner / progress) lives in F44_GameModeComponent.

enum F44_ESectorOwner
{
	NONE,
	ATTACKER,
	DEFENDER
}

[ComponentEditorProps(category: "F44", description: "Arma & Heroes capture sector")]
class F44_SectorComponentClass : ScriptComponentClass
{
}

class F44_SectorComponent : ScriptComponent
{
	[Attribute("A", desc: "Sector display name")]
	string m_sSectorName;

	[Attribute("0", desc: "Order in ASSAULT mode (0 = first sector the attacker must take)")]
	int m_iOrder;

	[Attribute("40", desc: "Capture radius (m)")]
	float m_fRadius;

	[Attribute("30", desc: "Capture time (s) for one unit alone")]
	float m_fCaptureTime;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		F44_SectorManager.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		F44_SectorManager.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	vector GetCenter()
	{
		return GetOwner().GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	bool IsInside(vector pos)
	{
		return vector.DistanceSqXZ(pos, GetCenter()) <= m_fRadius * m_fRadius;
	}
}

//------------------------------------------------------------------------------------------------
class F44_SectorManager
{
	protected static ref array<F44_SectorComponent> s_aSectors = {};

	//------------------------------------------------------------------------------------------------
	static void Register(F44_SectorComponent sector)
	{
		if (!s_aSectors.Contains(sector))
			s_aSectors.Insert(sector);
		SortByOrder();
	}

	static void Unregister(F44_SectorComponent sector)
	{
		s_aSectors.RemoveItem(sector);
	}

	//------------------------------------------------------------------------------------------------
	//! Sectors sorted by m_iOrder (simple insertion sort, few items)
	static void SortByOrder()
	{
		for (int i = 1; i < s_aSectors.Count(); i++)
		{
			int j = i;
			while (j > 0 && s_aSectors[j - 1].m_iOrder > s_aSectors[j].m_iOrder)
			{
				s_aSectors.SwapItems(j - 1, j);
				j--;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static int Count()
	{
		return s_aSectors.Count();
	}

	static F44_SectorComponent GetSector(int idx)
	{
		if (idx < 0 || idx >= s_aSectors.Count())
			return null;
		return s_aSectors[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! Is the sector capturable right now
	//! ASSAULT: only the first sector (by order) not yet owned by the attacker.
	//! SKIRMISH: all sectors.
	// P3 multi-line layouts (F44_AccessPoints.c): each line is its own assault chain; access point sectors stay contestable
	static ref array<ref array<int>> s_aLines = {};
	static ref array<int> s_aAlwaysActive = {};

	static bool IsActive(int idx, F44_EBattleMode mode, array<int> owners)
	{
		if (mode == F44_EBattleMode.SKIRMISH)
			return true;
		if (s_aAlwaysActive.Contains(idx))
			return true;
		foreach (array<int> line : s_aLines)
		{
			if (!line.Contains(idx))
				continue;
			foreach (int li : line)
			{
				if (li < owners.Count() && owners[li] != F44_ESectorOwner.ATTACKER && !s_aAlwaysActive.Contains(li))
					return li == idx;
			}
			return false;
		}
		for (int i = 0; i < owners.Count(); i++)
		{
			if (owners[i] != F44_ESectorOwner.ATTACKER)
				return i == idx;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Advance capture of every sector by dt seconds.
	//! progress: -100 (defender) .. +100 (attacker). Returns true if something changed.
	//! capturers receives the playerIds credited with a capture this tick.
	static bool Simulate(float dt, F44_EBattleMode mode, notnull array<int> owners, notnull array<int> progress,
		notnull map<int, int> playerSide, notnull array<int> capturers,
		array<IEntity> aiAttackers = null, array<IEntity> aiDefenders = null)
	{
		bool changed;
		PlayerManager pm = GetGame().GetPlayerManager();
		array<int> players = {};
		pm.GetPlayers(players);

		for (int s = 0; s < s_aSectors.Count(); s++)
		{
			if (!IsActive(s, mode, owners))
				continue;

			F44_SectorComponent sector = s_aSectors[s];
			int att, def;
			array<int> attInside = {};
			array<int> defInside = {};
			foreach (int pid : players)
			{
				int side;
				if (!playerSide.Find(pid, side))
					continue;
				IEntity ent = pm.GetPlayerControlledEntity(pid);
				if (!ent || !IsAlive(ent) || !sector.IsInside(ent.GetOrigin()))
					continue;
				if (side == F44_ESectorOwner.ATTACKER)
				{
					att++;
					attInside.Insert(pid);
				}
				else if (side == F44_ESectorOwner.DEFENDER)
				{
					def++;
					defInside.Insert(pid);
				}
			}

			// bots capture like players (no individual credit)
			if (aiAttackers)
			{
				foreach (IEntity a : aiAttackers)
				{
					if (a && IsAlive(a) && sector.IsInside(a.GetOrigin()))
						att++;
				}
			}
			if (aiDefenders)
			{
				foreach (IEntity d : aiDefenders)
				{
					if (d && IsAlive(d) && sector.IsInside(d.GetOrigin()))
						def++;
				}
			}

			if (att == def)
				continue;

			// speed: 100 points over captureTime, scaled by numeric advantage (capped at 3)
			int advantage = Math.AbsInt(att - def);
			if (advantage > 3)
				advantage = 3;
			float captureTime = sector.m_fCaptureTime;
			if (captureTime < 1)
				captureTime = 1;
			int delta = Math.Round(100.0 * dt * advantage / captureTime);
			if (delta < 1)
				delta = 1;

			int p = progress[s];
			int oldOwner = owners[s];
			if (att > def)
			{
				// attacker cannot capture a sector already his own
				if (oldOwner == F44_ESectorOwner.ATTACKER)
					continue;
				p = Math.ClampInt(p + delta, -100, 100);
				if (p >= 100)
				{
					owners[s] = F44_ESectorOwner.ATTACKER;
					capturers.InsertAll(attInside);
				}
				else if (p >= 0 && oldOwner == F44_ESectorOwner.DEFENDER)
				{
					owners[s] = F44_ESectorOwner.NONE;
				}
			}
			else
			{
				if (oldOwner == F44_ESectorOwner.DEFENDER && p <= -100)
					continue;
				p = Math.ClampInt(p - delta, -100, 100);
				if (p <= -100)
				{
					if (oldOwner != F44_ESectorOwner.DEFENDER)
						capturers.InsertAll(defInside);
					owners[s] = F44_ESectorOwner.DEFENDER;
				}
				else if (p <= 0 && oldOwner == F44_ESectorOwner.ATTACKER)
				{
					owners[s] = F44_ESectorOwner.NONE;
				}
			}

			if (p != progress[s] || owners[s] != oldOwner)
			{
				progress[s] = p;
				changed = true;
				if (owners[s] != oldOwner)
					PrintFormat("[F44] Sector %1 owner %2 -> %3", sector.m_sSectorName, oldOwner, owners[s]);
			}
		}
		return changed;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsAlive(IEntity ent)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(ent);
		if (!character)
			return false;
		CharacterControllerComponent ctrl = character.GetCharacterController();
		if (!ctrl)
			return false;
		return !ctrl.IsDead();
	}

	//------------------------------------------------------------------------------------------------
	static bool AllOwnedBy(array<int> owners, int who)
	{
		if (owners.IsEmpty())
			return false;
		foreach (int o : owners)
		{
			if (o != who)
				return false;
		}
		return true;
	}
}
