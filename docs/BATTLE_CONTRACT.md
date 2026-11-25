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
