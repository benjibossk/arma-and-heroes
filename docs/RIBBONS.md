# Ribbons — Arma & Heroes

Our take on the "ribbon" grind (research summary of the original in `inventory/HG_RIBBONS.md`).
The original used 12 levels per ribbon, fed by XP. We keep it simpler: **3 tiers (Bronze / Silver / Gold)**,
each tier is a **threshold on a per-soldier counter**, and pays a **one-time XP + credits reward**.
Some tiers **gate items** in the store (on top of the rank gate and the price).

Tuning: `web/ribbons.json` (embedded; a `./ribbons.json` next to the binary overrides it, restart to reload).

## Ribbon list (defaults)

| id | name | category | counter (`stat`) | Bronze / Silver / Gold | gates |
|---|---|---|---|---|---|
| rifle | Rifleman | weapon | `kills.rifle` | 25 / 50 / 200 | B: Thompson, MP40, G43 · S: BAR, MG42, scoped Springfield, scoped Kar98k |
| smg | Close Quarters | weapon | `kills.smg` | 25 / 100 / 300 | |
| mg | Suppressor | weapon | `kills.mg` | 25 / 100 / 300 | |
| sniper | Marksman | weapon | `kills.sniper` | 15 / 60 / 200 | |
| pistol | Sidearm | weapon | `kills.pistol` | 10 / 40 / 120 | |
| at | Tank Hunter | weapon | `kills.at` | 5 / 25 / 80 | B: Panzerschreck |
| grenade | Grenadier | weapon | `kills.grenade` | 10 / 40 / 120 | |
| explosive | Sapper | weapon | `kills.explosive` | 5 / 25 / 80 | |
| melee | Cold Steel | weapon | `kills.melee` | 3 / 15 / 50 | |
| vehiclegun | Mounted Gunner | weapon | `kills.vehicle` | 25 / 100 / 300 | |
| headshot | Steady Aim | skill | `headshots` | 10 / 50 / 200 | |
| multikill | Rampage | skill | `multiKills` | 3 / 15 / 50 | |
| capture | Point Taker | objective | `captures` | 10 / 50 / 150 | |
| defend | Holdfast | objective | `defends` | 10 / 50 / 150 | |
| resupply | Supply Line | support | `resupplyGiven` | 20 / 100 / 400 | |
| heal | Field Medic | support | `healGiven` | 20 / 100 / 400 | |
| revive | Lifeline | support | `revives` | 10 / 50 / 200 | |
| repair | Wrench | support | `repairs` | 10 / 50 / 200 | |
| spot | Eyes Forward | support | `spots` | 25 / 100 / 400 | |
| assist | Wingman | support | `assists` | 25 / 100 / 400 | |
| armor | Armor Breaker | vehicle | `vehicleKills` | 5 / 25 / 100 | |
| transport | Driver | vehicle | `transportPassengers` | 25 / 100 / 400 | |
| veteran | Veteran | service | `battles` | 10 / 50 / 200 | |

Rewards per tier are roughly 150/400/900 XP and half that in credits (see JSON). Ribbon rewards are added **outside**
the per-battle XP/credit cap (`progression.json antiFarm`), like the first-win bonus.

## Backend rules

- Tables: `soldier_stats(soldier_id, stat, value)` (unique per soldier+stat) and `soldier_ribbons(soldier_id, ribbon_id, tier, battle_id)`
  (unique per soldier+ribbon+tier → **an award can only happen once**). Battle results are already refused once a battle is finished, so counters are not double-counted either.
- On each `POST /battles/:id/result`, for every linked player: counters += battle deltas, then every tier whose threshold is reached and
  not yet in `soldier_ribbons` is inserted and its XP/credits are given to the soldier. An event `ribbon` is logged ("X earns Rifleman Bronze").
- Item gate: an item listed in some ribbon's `unlocks` needs that ribbon at that tier (`requires ribbon: Rifleman Bronze (25)`).
  Compat: items not listed in `ribbons.json` still honour the old catalog field `ribbon: {category, kills}` (checked against `kills.<category>`).
  The legacy `soldiers.cat_kills` JSON is still updated and merged into `kills.*` (max of both), so old soldiers keep their progress.
- `aiKillFactor` (default 1.0): if < 1 and the mod sends `playerKillsByCategory`, AI kills count only partially toward weapon ribbons.
- `GET /api/v1/soldiers/:id/ribbons` (public) → `{soldierId, tierNames, stats:{counter:value}, ribbons:[{id,name,category,stat,value,tier,tierName,next,tiers:[{tier,name,at,xp,credits,awardedAt?}],unlocks:[{item,label,tier,tierName,unlocked}]}]}`.
- Web: barracks page, "Ribbons" panel (progress bar to next tier, tier badges, gated items).

## Mod contract (F44_Core → `BattleResult.players[]`)

All fields are **per battle, per player**, optional (absent = 0). Existing fields are unchanged.

```json
{
  "identityId": "abcdef1234567890",
  "name": "Pvt Smith",
  "side": "US",
  "teamId": "at_1",
  "soldierId": 12,
  "kills": 14, "deaths": 3, "captures": 2,
  "playerKills": 6, "aiKills": 8, "assists": 4, "vehicleKills": 1, "defends": 1,
  "objectiveSec": 420, "presenceSec": 1500, "squadSec": 1200,

  "killsByCategory": {"rifle": 9, "smg": 0, "mg": 0, "pistol": 1, "sniper": 0, "at": 1,
                      "grenade": 2, "explosive": 0, "melee": 1, "vehicle": 0},
  "playerKillsByCategory": {"rifle": 4, "at": 1},

  "headshots": 5,
  "multiKills": 1,
  "resupplyGiven": 12,
  "healGiven": 3,
  "revives": 0,
  "repairs": 0,
  "spots": 7,
  "transportPassengers": 4,

  "stats": {"anyFutureCounter": 1}
}
```

Counter definitions (what the mod should count):

| field | meaning |
|---|---|
| `killsByCategory.<cat>` | enemy kills (players + AI) by the killer's weapon class. Classes: `rifle` (bolt + semi-auto), `smg`, `mg`, `pistol`, `sniper` (scoped), `at` (launchers/rifle grenades AT), `grenade` (hand grenades), `explosive` (TNT, mines, satchels), `melee` (bayonet, knife, buttstroke), `vehicle` (mounted/vehicle guns). `weaponStats{cat:{kills}}` is still accepted and wins if present. |
| `playerKillsByCategory` | same, only kills on human players (optional, used by `aiKillFactor`) |
| `headshots` | kills by head hit |
| `multiKills` | number of times the player got 3+ kills within 10 s |
| `resupplyGiven` | ammo resupplies given to **other** players (one per magazine pack / interaction) |
| `healGiven` | heals applied to **other** players (bandage/medkit uses) |
| `revives` | unconscious teammates brought back |
| `stats.bandagesApplied` | bandages applied to **other** soldiers (F44_Medical, docs/MEDICAL.md) |
| `stats.healHp` | health points restored on **other** soldiers |
| `repairs` | repair actions on friendly vehicles (one per completed repair) |
| `spots` | enemies marked that were later damaged/killed by the team |
| `transportPassengers` | distinct teammates transported ≥ 300 m as driver |
| `vehicleKills` | enemy vehicles destroyed |
| `captures` / `defends` / `assists` | as in docs/PROGRESSION.md |
| `stats` | free map for new counters (key ≤ 64 chars); add a ribbon in `ribbons.json` with `"stat": "<key>"` to use it without code change |

Anti-farm: count only actions on teammates/enemies other than self, ignore friendly kills, and the backend refuses a second result for the same battle.
