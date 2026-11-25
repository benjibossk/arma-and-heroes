# CAMPAIGN — strategic layer design choices (GAP_PLAN P2/P3, web lane)

Design decisions taken autonomously on 2026-11-26. Mechanics are paraphrased from the
archived reference; names, numbers and texts are our own. Tunables live in Go constants
next to the code (one file per package) unless noted.

## P2-A Team doctrines + command points (`web/upgrades.go`)
- Each assault team can adopt **doctrines** bought with war funds *and* command points (CP).
- **CP budget = team level (P1-B) + commander rank / 3.** CP are never refunded.
- Tree: `DRILL` (T1, +1 equip) → `SHOCK` (+15% max MP) → `SPEARHEAD` (+1 equip, +10% MP),
  or `DRILL` → `ENTRENCH` (+20 morale, +10% MP) → `BULWARK` (+20% MP, +20 morale).
  `SHOCK` and `ENTRENCH` exclude each other (branching). Independent line: `LOGISTICS`
  (resupply −25%) → `RAIL_DEPOT` (another −25%, total capped at 75%).
- Max-manpower bonuses raise the resupply target (`teamMaxManpower`, teams.go).
- **Team cap per commander = 2 + rank / 2** live teams (admin uncapped), checked in `createTeam`.
- API: `GET /api/v1/teams/:id/upgrades`, `POST /api/v1/teams/:id/upgrades/:key` (general, team idle).
- UI: map team card → "Doctrines & command points" (`static/campaign.js`).

## P2-B Road skirmishes (`web/skirmish.go`)
- Two hostile teams moving on the same road in opposite directions meet: when the first
  arrives while the other is still en route (they have crossed), `arrive()` starts a
  `SKIRMISH` battle (`Battle.RoadID` set, 30 min, no garrison, no capture, no city-capture war funds).
- Scenario = the city the arriving team was heading to (the foe's origin). If that city is
  already under attack, no skirmish (the arrival proceeds normally).
- Winner resumes its march and arrives on the next tick; loser falls back to its origin idle.
