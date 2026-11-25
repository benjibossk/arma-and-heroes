# Battle contract — assault-team vehicles, losses, resupply

Tunables: `web/teams.json` (embedded; a `./teams.json` next to the binary overrides it).

## Vehicles allowed per team type (web side)

| Type | Allowed roles |
|---|---|
| INF, PARA | none |
| ARM | tank, light_armored, light_armored_aa |
| REC | light_utility, light_armed, light_armored |
| MOT | truck, light_utility, light_armed, light_armored |

`vehicleRoles` maps each vehicle key to a role; an unknown key is refused. The filter is applied when a team is
raised (`newTeam`), merged (`mergeTeam`), resupplied, and again in `BattleConfig.sides[*].teams[*].vehicles`
sent to the mod, so the mod only ever receives an allowed pool.

## Per-vehicle fit-out (supply crate, armed variant)

The general configures each vehicle of an idle team on the map page (team panel → *Vehicle fit-out*):
`POST /api/teams/:id/vehicle {key, slot, crate: "" | "AMMO" | "MED" | "AT", armed: bool}`; prices from
`GET /api/teams/options` (= `teams.json` `crates` / `variants`). A new crate or the armed upgrade is paid once in
war funds (friendly city not under attack); removing it is not refunded. Stored in `AssaultTeam.VehicleCfg`
(`[]VehicleOpt{key, slot, crate, armed}`); slots beyond the current vehicle count (losses) are dropped, last slot first.

Each `vehicles[]` entry of a team in the BattleConfig now groups vehicles of the same key **and** config:

```json
{ "key": "US_JEEP", "count": 1, "crate": "AMMO", "armed": true, "prefab": "{10448D0351BF0979}Prefabs/Vehicles/ww2 jeep/Willys Jeep_MG.et" }
```

- `crate` (optional): the mod fits that crate on the vehicle each time it spawns from the pool (respawn included),
  via `F44_FieldSupply.AttachToVehicle`, once FieldSupply is up (~8 s after start). The crate is a normal
  AMMO/MED box or AT crate (same charges / stock / regen), dies with the vehicle and is not respawned on the ground.
  Manual in-game "load box on vehicle" is kept.
- `prefab` (optional): overrides the catalog prefab of `key` — `variants[key].base`, or `variants[key].armed` when
  `armed` is true. Absent = catalog prefab. Key, tickets and losses still use `key`.
- old mods ignore the new fields (they read `key`/`count` only).

## Vehicle losses → tickets (mod side, already implemented)

F44_Core already does both, no new field needed:
- a destroyed pool vehicle removes `catalog.json` vehicle `cost` tickets from its side (`RemoveTickets`);
- it is counted in `BattleResult.losses[teamId].vehicles[key]`.

The server subtracts `losses.manpower` and `losses.vehicles` from the team (`applyResult`). Vehicle tickets are
*not* added to manpower (the mod's tickets are battle-only; the persistent cost of a lost vehicle is its resupply price).

## Strength and resupply

- Full team = `atSpecs[type]` (manpower + faction vehicle pool), cost = buy cost.
- `vehicleShare[type]` of the cost is attributed to vehicles (pro-rata per vehicle), the rest to manpower.
- `strengthPct` = 100 × (1 − missing value / cost). `resupplyCost` = ceil(missing value × `resupplyFactor`) (0.5).
- `POST /api/v1/teams/:id/resupply` (owner general): team idle, in a friendly city not under attack;
  refills to full, debits faction war funds.
- `moveTeam` to an enemy city is refused when `strengthPct < minAttackPct` (50).
- `/map` team views of your own teams carry `strengthPct`, `resupplyCost`, `canAttack`.

## Spawn timers and mobile spawn (battle JSON, optional, mod 1.2.0)
- `spawnTimes`: `{ "baseSec": 10, "forwardPenaltySec": 10 }` — base respawn timer and the extra delay on forward sector points.
- `mobileSpawn`: `{ "enabled": true, "usesHalftrack": 8, "usesTruck": 3, "cooldownSec": 20, "penaltySec": 10, "maxSpeedKmh": 2, "stopSec": 3, "enemyRadius": 40, "driverAbsentSec": 15, "backOffset": 3, "requireDriver": true, "halftrackKeys": ["GER_SDKFZ251*", "US_M3A1*"], "truckKeys": ["GER_TRUCK", "GER_TRUCK_OPEN", "US_TRUCK*"] }`.
  Applies to the team pool vehicles whose key matches; rules and defaults in docs/SPAWN_UI.md "Mobile spawn". Absent = defaults.

## Air drop (battle JSON, optional, mod 1.2.0) — `F44_ParaDrop.c`
- `airDrop`: `{ "enabled": true, "altitude": 350, "radius": 1200, "speed": 65, "planeCount": { "US": 1, "GER": 0 }, "classes": ["PARA"] }`.
  - `altitude` (m AGL), `radius` (m from the layout centre) and `speed` (m/s) set the C-47 **spawn** state; then the Aviation-Toolkit AI pilot orbits the layout centre with the prefab's own profile (cruise 350 m AGL, orbit radius 1200 m, ~140 kt).
  - `planeCount`: planes per side key (0/absent = none). Each plane = one `AIR DROP` spawn point for that side's faction, disabled while its 30 cargo seats are full.
  - `classes`: class keys allowed to use it (default `["PARA"]`). Gate is server-side (loadout at reservation + spawned prefab), the client still lists the point for everyone.
  - `testSeatBot` (debug only): seats a PARA bot after each plane spawn and logs `AirDrop TEST/TRACK` lines.
- Server mods required for the feature: AeroplaneHavens C47 Dakota `66A2A0B8531C9D2A` (+ Aviation-Toolkit `69B307004DF5C417`, USAAF Bomber Attire `64F8C1AB4CFD207A`), Grey Mans Chute Framework `69EEAA01F4A11001`. Not dependencies of F44_Core: without them the plane prefab does not load and the air drop is skipped with a warning.
- **Web side (later, not implemented):** a team of type `PARA` should carry "transport plane" resources (H&G ratio 4 soldiers : 1 plane) and the battle payload should then fill `airDrop.planeCount[side]` from them; PARA teams list `classes: ["PARA"]`.
