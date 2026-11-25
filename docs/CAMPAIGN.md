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

## P2-C Strategic economy (`web/economy.go`)
- **Factory stockpile**: the +30 factory bonus of each income tick is banked in
  `City.Stockpile` (cap 600) instead of the national treasury. A team raised in that city is
  paid from the stockpile first, then from faction funds (`payTeamCost`).
- **Capture** seizes 50% of the city's stockpile into the captor's funds; the rest is lost.
- **Neutral cities** (owner empty): the first team marching in rallies them, no battle.
  Map shows them as "Neutral". Seeding neutral cities is left to the maps lane (`seed.go`).
- **Capital-count victory**: a faction holding >= 3/4 of all capitals (rounded up, min 2)
  wins. With only the two faction capitals this equals the old "hold both" rule. The 70%
  city rule is unchanged.
- **War-end payout**: every player who fought gets credits on his main side: winners 400,
  losers 150 (refund), +20 per battle fought (max 20 battles). Paid to player + active soldier.

## P2-D Rank salary (`web/salary.go`, `progression.json` "salary")
- Battle credits += **minutes alive x rank pay** (2,3,4,5,6,7,8,10 per min for ranks 1..8,
  capped at 60 min) **+25% if the player stayed to the end**. Not paid to idle players.
- Contract (optional, backward compatible): `players[].aliveSec`, `players[].stayedToEnd`.
  Fallbacks: `presenceSec` for time alive; stayed = presence >= 90% of `durationSec`.

## P3 Account careers + career lock (`web/careers.go`, web part)
- Careers: `infantry` (INF, PARA), `armor` (INF, CREW), `pilot` (INF, PARA + pilot seat),
  `recon` (INF, REC). Enlisting a soldier of a class outside the career is refused.
- First pick free; then a change costs 500 credits (player credits) and is allowed once per
  7 days. Accounts without a career are grandfathered (no restriction) until they pick.
- Mod hook (seat enforcement [GM], not done): `GET /api/v1/careers/by-identity/:identityId`
  (server key) -> `{linked, career, seats[]}` with seats `tank_driver|tank_gunner|tank_commander|pilot|recon_driver`.
- UI: barracks -> "Service record" panel.

## P3 Cosmetics, weapon upgrades, cross-faction surcharge (`web/cosmetics.go`)
- **Cosmetics** per soldier: camo (`standard` free, `field` 150, `mud` 250 r2, `winter` 400 r3,
  `veteran` 800 r5 — bought once with soldier credits) + face preset 1..8 (free).
  `GET/POST /api/v1/soldiers/:id/cosmetics` (GET is public so the mod can read it).
- **Weapon upgrades**: owned primary/secondary, 3 levels (sights, action, match ammo);
  level n needs rank n+1; price = 100 x n + weapon price x n / 4.
  `GET /api/v1/soldiers/:id/weapon-upgrades`, `POST .../weapon-upgrades/:item`.
  Gameplay effects are mod-side (not done): web only stores and exports the levels.
- **Cross-faction**: enemy primary/secondary weapons become buyable from rank 3 at +50%
  (`itemUsable` / `unlockPrice`; depot, unlock tree and loadout validation use them).
- UI: barracks "Service record" panel (appearance + armourer).
