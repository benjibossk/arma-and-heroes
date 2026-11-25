# GAP_PLAN — archived mechanics vs Arma & Heroes (2026-11-26)

Source: the private wiki archive (109 pages, 8 categories), read page by page; mechanics paraphrased, never copied.
Status was checked by grepping `web/` (Go + static), `mod/F44_Core/Scripts`, `orchestrator/`, `docs/`, `inventory/`.
Abbreviations: **GM** = `mod/F44_Core/Scripts/Game/F44/F44_GameModeComponent.c`; **api.go** = `web/api.go`; **game.go** = `web/game.go`.
Pure lore, controls and vanilla Reforger handling (sway, flight keys) are left out. Duplicates across pages are merged.

**Totals: 108 mechanics — 18 done, 53 partial, 37 missing.**

## 1. Mechanics table

### 01 Battle mechanics / 02 War map strategy

| Mechanic (paraphrase) | Source page | Status | Where in our code | Missing |
|---|---|---|---|---|
| Two layers: FPS battles are spawned by the strategic map; also an unranked practice mode | Action Game, Action Rules | partial | `Battle` model, `buildConfig` (game.go) | Practice mode with bots on both sides |
| Teams travel along roads, travel time grows with distance | Battle System, Strategy Game | partial | `travelDuration`, `moveTeam` (api.go), `arrive` (game.go) | Per-type speed, air movement |
| Enemy contact starts a battle: in a town = assault, on a road = skirmish | Battle System, Battle, Skirmish | partial | `arrive` (game.go) assault only | Road skirmishes, mid-road meetings |
| Extra teams can join a running battle | Battle | done | `addTeamToBattle` (game.go) | — |
| Short preparation window before a battle opens | Battle System, Battle Director | missing | `Battle.Status` "pending", no timer | Prep timer + "starting" state |
| A director cancels battles too lopsided / too small to be fun, with a penalty | Battle Director | missing | — | Size check, retreat + resource/morale penalty |
| Idle battles are auto-resolved after a long timeout (stronger side wins) | Auto-Resolver | missing | — | Timeout + resolver in `doTick` |
| Assault: attackers must take every objective, defenders win on time | Assault, Action Rules | done | GM `CheckEnd` | Timer is 45 min (wiki-like 30) |
| Objectives unlock one after another | Capture Points | done | `m_iOrder`, `SortByOrder` (F44_SectorComponent.c) | — |
| Capture goes through neutral, faster with more attackers | Capture Points | partial | F44_SectorComponent.c progress ticking | Per-type capture times, +time bonus per capture |
| Several parallel lines of access points chained to objectives; being pushed off all of them loses | Access Point, Capture Points | missing | single linear order only | Lines/chains, "pushed off" defeat |
| Skirmish: few points in any order, a score bar fills faster with more points held | Skirmish | partial | `F44_EBattleMode.SKIRMISH` in GM | Bar-to-100 victory, long timeout |
| Spawn only on owned, connected, non-contested points | Action Rules | partial | `IsFrontSector`, `IsContested` (F44_SectorSpawn.c) | Nearby-death lockout, secondary spawns |
| Respawn delay = base + per-link + per-vehicle + underdog help | Action Rules | partial | `F44_SpawnTimesConfig` (10 s / +10 s) | Per-link scaling, underdog pool |
| Mobile spawn in vehicles with caps (1 halftrack, 3 tanks) | Action Rules | done | F44_MobileSpawn.c | Stolen-vehicle rule |
| One transport plane for para spawns at a time | Action Rules | partial | F44_ParaDrop.c | Plane pool fallback |
| Fighters spawn on the runway or airborne | Action Rules | partial | F44_ParaDrop.c (closest) | Fighter spawn, air team type |
| Limited lives per team; side with no lives loses | Action Rules | done | tickets `RemoveTickets`, `EndBattle("TICKETS")` (GM) | — |
| Squads with a leader, orders, spawn on leader, squad XP | Squads | missing | — | Whole system |
| Squad size/slot limits per class | Squads | missing | — | Slot rules |
| Max 3 active mines per player | Action Rules | missing | — | Mine cap |
| Hit-zone damage multipliers | Action Rules | partial | vanilla hit zones, docs/MEDICAL.md | Explicit tuning |
| Score/XP formulas shared among contributors | Action Rules | partial | `computeReward`, `creditSoldier` (game.go, progression.go) | Assist split, spot/savior XP |
| Empty server closes without result | Action Rules | partial | `heartbeat` (api.go), orchestrator | Exact rule unverified |
| Faction stockpile produced over time, spent to deploy teams | Limited Supply, Strategy Game | partial | `applyIncome`, `Faction.resources`, `createTeam` | Per-unit production, queue |
| Refunds and bonuses at war end | Strategy Game | partial | `applyWarFunds`, `dissolveTeam` half refund | End-of-war refund, winner bonus |
| War victory by holding most capitals | Victory conditions, Campaign map | partial | `checkVictory` (game.go: 70 % cities / all capitals) | Capital-count rule |
| Capitals start neutral, few starting capitals per side | Campaign map | partial | `City.IsCapital`, seed.go | Neutral capitals |
| Road colours show friendly/contested/enemy supply | Supply lines | partial | web/static/map.js | Unverified colouring by supply |
| Team morale: drops per battle, rises on win/time, rout when low | Morale | missing | — | Whole system |
| Defeated team retreats to adjacent friendly town or dies | Battle System | done | `retreatCity`, `applyResult` (game.go) | Morale penalty |
| Team destroyed at zero manpower | Morale | done | `applyResult` | — |
| Command points budget gated by rank | Command Points, Officers, General tips | missing | `requireGeneral` (auth.go) only | CP budget, rank gating |
| Generals vs officers (generals only command) | Generals, Officers | partial | role general/admin, `IsGeneral()` | Promotion rule, FPS lockout |
| Command hierarchy (supreme / strategic / team) | Command Structure | missing | single general role | Hierarchy |
| Teams are raised at a capital or factory | General tips | partial | `createTeam` (api.go) | CP requirement |
| Player locked to a faction for a war | Factions | partial | `Player` side, `GetLinkedSide` | Explicit war-long lock |
| Airfields: air teams move only airfield to airfield | Airfield, Forward Airfield | missing | — | Air layer, airfield nodes |
| Battle map chosen by team composition (no-armour encounter maps) | Samrée Depot, List of Maps | missing | `Mode`/`ScenarioID` per city | Composition-based choice |
| Map list per mode | List of Maps | partial | catalog.json, web/static/maps | More maps |
| Spotting enemies by aiming at them | Action Game Controls | partial | `Spots` counter (ribbons.go) | Spot mechanic in mod |

### 03 Assault teams / 04 Economy progression / 05 Ribbons badges

| Mechanic (paraphrase) | Source page | Status | Where in our code | Missing |
|---|---|---|---|---|
| A team is a pool of soldiers + vehicles commanded on the map | Assault Team | done | `atSpecs`, `AssaultTeam` (models.go), teams.go | Per-class pools |
| Team type families (guard, inf, motorised, mech, recon, armour tiers, TD, air, para) | 14 unit pages | partial | `atSpecs`: INF, PARA, ARM, REC, MOT | Mech, TD, armour tiers, air, guard |
| Upgrade tree: higher types are paid upgrades along one branch | Assault Team Progression | missing | `EquipLevel` only (game.go) | Upgrade endpoint, prices, branches |
| Buy and resupply costs | Assault Team Costs, Warfunds | partial | buy in api.go, `resupplyTeam` (teams.go) | Per-resource cost |
| Commander earns funds equal to the value of what his team destroys | Warfunds | missing | `applyWarFunds` flat per battle | Per-kill attribution |
| Funds for objective actions (capture/defend) | Warfunds | partial | `applyWarFunds` `PerCapture` | Defend/neutralise |
| Team XP per battle (win/loss) with participation threshold, team levels | Assault Team Progression | missing | — | XP column, levels |
| Commander gains XP from his teams | Ranks | missing | — | Link |
| Team map speed and vision radius by type | Assault Team | partial | `travelMinPerKm` global | Per-type speed, fog |
| Spawns are deducted from the team pool | Assault Team Slots | done | manpower deduction (game.go) | — |
| Max teams per commander | Assault Team Slots | partial | none explicit | Cap at purchase |
| Disband with partial refund | Assault Team Costs | done | api.go disband | — |
| Mech team brings supply-crate halftracks | Mechanized Infantry | partial | `configureVehicle`, crates (teams.go), F44_MobileSpawn.c | Mech type |
| Para tiers (pathfinder → para), planes as a resource | Pathfinders, Paratrooper | partial | `PARA`, F44_ParaDrop.c | Tier, plane count |
| Credits = rank salary × time alive, bonus for staying to the end | Credits, Soldier salary | partial | `computeReward` flat table (progression.go) | Salary × time formula |
| Credits not purchasable, scale with score | Credits | partial | progression.json `RewardTable` | — |
| Premium currency | Gold | missing | — | Intentionally skipped |
| Bond currency (removed upstream) | WarBonds | missing | — | Skip |
| Ranks from XP; command from a set rank; optional irreversible general promotion | Ranks | partial | `rankFor` (models.go), `rankIdxFor` (soldiers.go) | Per-class tables, promotion flow |
| XP weighted by action type | Ranks | done | `computeReward`, docs/PROGRESSION.md | — |
| Several characters, one class each, customisable | Characters | partial | `createSoldier`, `maxSoldiers` (soldiers.go) | Tank/pilot classes |
| Soldier slot count | Soldier Slots | partial | `maxSoldiers` 2–6 | Design choice |
| Equipment-point budget per class | Equipment Points | done | `budgetFor`, `validateLoadout` (soldiers.go) | Per-class tuning |
| Death adds a maintenance timer to items/vehicles; deploy blocked over a cap | Field Maintenance Load | missing | respawn timer in GM | Load timers, lockout |
| Daily login reward cycle with streak | Daily Reward | partial | first-win-of-day bonus (progression.go) | Cycle, streak |
| Premium membership bonuses | Warfunds, Combat Badge | missing | — | Skip |
| Seasonal reward track | Battle Pass Season 0 | missing | — | Season tiers |
| Ribbons level up from actions and unlock gear | Combat/Specialist Ribbons | partial | ribbons.go/json (3 tiers) | Per-weapon specialist ribbons |
| Ribbon kinds (capture, rifle, AT, driving, passengers, medic) | Tactical/Infantry/Tank/Driver/Chauffeur/Medic Ribbon | done | ribbons.json, `battleCounters` | Driving-time counter |
| Badge slot: equip one perk earned from ribbon tiers | Combat Badge | missing | badge = icon only (barracks.html) | Perk effects, slot UI |
| Medic/toughness perks (faster bandage, more kits, less injury) | First Aid / Combat Medic / Hand of God / Heavy Set CB | missing | F44_Medical.c (no perk hook) | Perk scaling |

### 06 Classes / 07 Equipment weapons / 08 Vehicles aircraft

| Mechanic (paraphrase) | Source page | Status | Where in our code | Missing |
|---|---|---|---|---|
| Classes (infantry, recon, tank crew, pilot, para), soldier locked to one | Class pages | partial | models.go roles, soldiers.go | Tank/pilot/recon account classes |
| Spawn delay driven by loadout equipment points | Spawn Time | missing | flat 10 s (docs/SPAWN_UI.md) | EP-scaled delay |
| Spawn delay grows with distance from the front point | Spawn Time | missing | fixed +10 s forward (F44_DeployMenu.c) | Distance term, defender bonus |
| Penalty for spawning in a vehicle | Spawn Time | done | F44_MobileSpawn.c `penaltySec` | — |
| Suicide and first-join penalties | Spawn Time | missing | — | Both penalties |
| Per-vehicle spawn delay and cooldown | Spawn Time | partial | `mobileSpawn.cooldownSec` | Fighter delay |
| Vehicle spawn temporarily disabled when damaged/enemy close | Infantry Support Vehicles | partial | F44_MobileSpawn.c (lost/full/destroyed) | 15 s temporary lock |
| Vehicles cost team resources; loss costs tickets | Vehicles, tank pages | partial | `GetVehicleCost` (F44_Catalog.c), GM `RemoveTickets` | Per-deploy price |
| Vehicle unlock by licence | Vehicles | partial | `LIC_<vehicle>` (docs/PROGRESSION.md) | In-battle enforcement |
| Repair/rearm timer after vehicle loss | Terrain, tank pages, Supply Crates | missing | — | Timer |
| Seats and gunner positions | vehicle pages | partial | F44_VehicleSeats, prefabs | No tuning (vanilla OK) |
| Free light vehicles at spawns | Terrain, bike/moto pages | done | F44_CivVehicleSpawner.c, F44_FieldSupply.c | — |
| Neutral civilian vehicles | Civilian Vehicles | done | F44_CivVehicleSpawner.c | — |
| Tank tiers and shell types | Light/Heavy Tanks | partial | catalog.json | AP/HE split |
| Gunner/mechanic crew perks | Tank Crew | missing | — | Badge effects |
| Repair tool for crews | Tank Crew, Close Combat | partial | vanilla repair | Class gating |
| Melee weapons with stats | Close Combat Weapons | partial | catalog.json melee | Vanilla stats |
| Medkits cost points and credits | Medkits | partial | F44_Medical.c, docs/MEDICAL.md | Cost tiers |
| Supply crates as a bought vehicle upgrade | Supply Crates | partial | F44_FieldSupply.c free crates | Purchase per vehicle |
| Free launcher crates on the map | Weapon crates | done | F44_FieldSupply.c | — |
| AA guns on points | Anti-Air Guns | partial | catalog.json flak entries | Per-point placement |
| Explosives (grenades, mines, charges) | Explosives | partial | PROGRESSION unlock tiers | Placement rules |
| Picking up enemy weapons | Captured Weapons | partial | lootable bodies (F44_VehicleStorageLock.c) | Account-level captured unlocks |
| Foreign-faction gear costs more to unlock | Captured Weapons, class pages | missing | — | Surcharge rule |
| Per-weapon ribbons gate weapon unlocks | weapon pages | partial | ribbons.json | Per-weapon ribbons |
| Weapon upgrades (scope, barrel, mags) | weapon pages | missing | "out of scope" in PROGRESSION | Upgrade slots |
| Camouflage choice per class/faction | Customisation | missing | inventory/UNIFORMS.md | Unlock + selection |
| Face/skin presets | Customisation | missing | names.go only | Presets |
| Event cosmetic helmets | Customisation | missing | — | Low priority |
| Career lock after purchase | Infantryman | missing | — | Rule |
| Aircraft component damage model | Fighter Planes | missing | vanilla | Tuning |
| Plane classes (light/medium/heavy/recon) | Fighter Planes | partial | catalog.json, AIRCRAFT_PARA.md | Balance |
| Plane ammo choice (HE vs AP) | Flight Manual | missing | — | Selection |
| Land to repair at airfield | Flight Manual | partial | HG_MECHANICS.md note | Repair zone |
| Plane ticket cap per battle | Fighter Pilot | missing | — | Cap |
| Paras jump from a plane over the sector | Spawn Time, Infantryman | done | F44_ParaDrop.c | — |

## 2. Work packages

Conflict hotspots: **[GM]** = touches `F44_GameModeComponent.c`, **[API]** = touches `web/api.go`. At most one package per hotspot should run at a time.

### P1 — tonight, no human tester, parallel-safe (none touches GM or api.go)

**P1-A Battle director (web)** — prep window, minimum-size "fun" check with retreat penalty, auto-resolve of idle battles.
- Files: new `web/director.go` (+ `web/director_test.go`); one call added in `doTick` (game.go). Constants in `web/progression.json` or director.go.
- Effort: M (3 h). Test: `go test ./web/...` with fake clock: pending battle → starting after prep; undersized battle cancelled, teams retreat; idle battle resolved after timeout, stronger side wins. `go vet`.
- Deps: none. Shared file: game.go (only `doTick`).

**P1-B Team morale + team XP (web)** — DONE 2026-11-26 (web/morale.go, lazy regen 5/h, rout <25 = 10% desertion, XP only if battle ≥300 s; deploy pending). — morale field (start 100, −per battle, +win, +regen per tick, rout under threshold), team XP/levels on win/loss with participation threshold, shown on the map card.
- Files: new `web/morale.go` (+ test); `web/models.go` (`AssaultTeam` fields + migration); hook in `applyResult` (game.go); display in `web/static/map.js`.
- Effort: M (3 h). Test: unit tests on applyResult outcomes; DB migration on a copy of the SQLite file; map card renders morale.
- Deps: none; P1-A may later apply morale penalties (call `adjustMorale`, read-only dependency). Shared file: game.go (only `applyResult`).

**P1-C Spawn-time model (mod)** — respawn delay = base + equipment-point term + per-link distance term; defender bonus; values read from the battle config.
- Files: `F44_SectorSpawn.c`, `F44_SpawnTimesConfig` (its .c), `F44_DeployMenu.c` (display); web side only adds fields in `buildConfig` (game.go) and loadout points from `loadoutPoints` (soldiers.go).
- Effort: M (3 h). Test: Workbench compile (wb_compile) with zero errors; config JSON round-trip unit test in Go; local headless server log shows computed delays.
- Deps: none. Shared file: game.go (only `buildConfig`).

### P2 — next, mostly one hotspot each (run serially by hotspot)

- **P2-A Team upgrade tree + command points [API]** — branching upgrades, CP budget by rank, team cap per commander. Files: `web/models.go` atSpecs, new `web/upgrades.go`, api.go routes, `web/static` HQ page. Effort M. Test: go tests on branch rules + curl. Deps: P1-B (team XP/levels).
- **P2-B Road skirmishes [API]** — teams meeting on a road fight a skirmish battle. Files: `moveTeam` (api.go), `arrive` (game.go). Effort M. Deps: P1-A.
- **P2-C Strategic economy (web)** — per-unit production stockpile, capital-count victory, neutral capitals, war-end refund/bonus. Files: `applyIncome`, `checkVictory` (game.go), seed.go, progression.go. Effort M. Test: go tests. Deps: none (conflicts with P1-A/B in game.go only by function).
- **P2-D Salary credits (web)** — credits = rank salary × time alive × stay-to-end bonus. Files: progression.go/json. Effort S. Deps: battle results already carry play time (verify).
- **P2-E Per-kill war funds [GM][API]** — mod reports kills by victim type/team; web pays the commander. Effort M. Deps: none.
- **P2-F Field maintenance load [GM]** — per-item/vehicle timers after death, deploy lockout over cap. Also F44_DeployMenu.c. Effort L. Needs in-game test.
- **P2-G Combat badges** — perk slot in barracks (web/soldiers.go, barracks.html), effects in F44_Medical.c / F44_Loadout.c (medic speed, extra kits, injury reduction). Effort M. Mod part needs in-game test.

### P3 — later, needs design or human playtest

- Squads with leader/orders/spawn [GM] (L).
- Multi-line access-point layouts + "pushed off the map" defeat [GM] + F44_SectorComponent.c (L).
- Air layer: airfields, air teams, fighter spawn, plane tickets (web + F44_ParaDrop.c) (L).
- Vehicle temporary spawn lock, mine cap, suicide/join penalties [GM] (S each).
- Account classes tank/pilot/recon + career lock (web/models.go, soldiers.go) and in-battle seat enforcement [GM].
- Cosmetics (camo, face presets), weapon upgrades, cross-faction unlock surcharge (web/progression.go, soldiers.go).
- Daily reward cycle + seasonal track (web/progression.go, [API] routes).
- Skip: premium currency, membership, bonds.
