# Progression — the grind

All numbers live in `web/progression.json` (embedded at build; a `progression.json` next to the binary overrides it at startup).
Code: `web/progression.go`. Progression is **per soldier** (H&G style: each soldier has its own XP, credits, rank and owned items);
the player account also accumulates the same XP/credits as a career total.

## 1. Rewards per battle (computed server-side from BattleResult; the mod's `xp` field is ignored)

| Action | XP | Credits |
|---|---|---|
| Kill — player | 40 | 10 |
| Kill — AI (diminishing, see §2) | 15 | 4 |
| Assist | 15 | 4 |
| Vehicle destroyed | 100 | 40 |
| Sector captured | 80 | 20 |
| Sector defended | 40 | 15 |
| Time on objective, per minute (max 30 min) | 6 | 2 |
| Victory / Defeat | 150 / 50 | 60 / 25 |
| Participation floor (only if active) | 50 | 25 |
| Squad play (≥ 50 % of presence in a squad) | +10 % | +10 % |
| First win of the day (per player, UTC) | +200 | +150 |

BattleResult player fields used: `kills, deaths, captures, killsByCategory` (sent today) and optional
`playerKills, aiKills, assists, vehicleKills, defends, objectiveSec, presenceSec, squadSec`.
While the mod does not split kills, all `kills` count as AI kills (conservative).

Typical battle (6 kills, 1 capture): **~350 XP / ~130 cr** on a win, ~250 XP / ~95 cr on a loss, +200 XP / +150 cr for the first win of the day.

## 2. Anti-farm

| Rule | Value |
|---|---|
| AI kill value | ×1 for kills 1-10, ×0.5 for 11-20, ×0.25 for 21-30, 0 beyond |
| Per-battle cap (first-win bonus excluded) | 1 500 XP, 600 cr |
| Idle | no reward if no kill/assist/capture/defend/death/objective time |
| Presence | no reward under 5 min in battle (when `presenceSec` is sent) |
| Purchases | atomic credit deduction; buying an owned item is a no-op (idempotent) |

## 3. Rank curve (15 ranks)

| # | US | GER | XP | ≈ battles |
|---|---|---|---|---|
| 1 | Private | Schütze | 0 | 0 |
| 2 | Private First Class | Oberschütze | 600 | 2 |
| 3 | Corporal | Gefreiter | 1 300 | 4 |
| 4 | Sergeant | Obergefreiter | 2 200 | 6 |
| 5 | Staff Sergeant | Unteroffizier | 3 300 | 10 |
| 6 | Technical Sergeant | Unterfeldwebel | 4 600 | 13 |
| 7 | Master Sergeant | Feldwebel | 6 000 | 17 |
| 8 | First Sergeant | Oberfeldwebel | 7 600 | 22 |
| 9 | Sergeant Major | Stabsfeldwebel | 9 400 | 27 |
| 10 | Warrant Officer | Hauptfeldwebel | 11 400 | 33 |
| 11 | Second Lieutenant | Leutnant | 13 600 | 39 |
| 12 | First Lieutenant | Oberleutnant | 16 000 | 46 |
| 13 | Captain | Hauptmann | 18 600 | 53 |
| 14 | Major | Major | 21 400 | 61 |
| 15 | Lieutenant Colonel | Oberstleutnant | 24 400 | 70 |

(≈ battles assumes ~350 XP/battle including some daily bonuses.)

## 4. Unlocks

Each item = **required rank + credit price**, bought per soldier on the Unlocks page (`POST /api/v1/unlocks/{key}/buy`).
Class/faction compatibility (`classes` in `catalog.json`) and ribbons (e.g. Thompson: 25 rifle kills) still apply.
Trees (display tabs): rifleman, assault, mg, medic, at, recon, tanker. Free starter kit per class
(Garand/Kar98k, M1911/P38, class uniform) is always owned; existing soldiers are back-filled at startup.

| Tier | Target | Examples (US / GER) | Rank | Price |
|---|---|---|---|---|
| Starter | always | M1 Garand, M1911 / Kar98k, P38, uniforms | 1 | 0 |
| Entry | 2-3 battles | M1 Carbine 300, M3 Grease 350, Panzerfaust 350, grenades 100, medkit 150 | 1-2 | 80-350 |
| Light vehicles | ~5 battles | Jeep / Kübelwagen 500, trucks 600 | 3 | 500-600 |
| Mid | 15-20 battles | Thompson / MP40 1 500, scoped rifles 1 800, Bazooka 2 000, G43 2 000, BAR 2 400, MG42 2 600, half-tracks 2 200 | 5-8 | 1 500-2 600 |
| High | 30-40 battles | TNT / mines 2 800, Panzerschreck 3 800 | 9-11 | 2 800-3 800 |
| Top | 60+ battles | Panzer IV 5 000 (r12), Sherman 6 000 (r13), Tiger I 8 000 (r15) | 12-15 | 5 000-8 000 |

Vehicle **licences** (`LIC_<vehicle>`) are owned like items; in-game enforcement (who may drive/crew) is a later mod step.
Weapon upgrades (scopes, extended mags) are out of scope for now; the same rank+price model will apply.

## 5. War funds (generals)

Faction resources fund assault teams (team cost: INF 300, PARA 450, ARM 800, REC 250, MOT 400) on top of city income.
After each battle each side earns: **win 250 / loss 100**, +150 if the attacker captures the city, +15 per participating player, +10 per sector capture.

## 6. Loadouts

`PUT /soldiers/{id}/loadout` only accepts owned, class-compatible items within the point budget (server-side).
The mod keeps reading `GET /api/v1/battles/{id}/loadout/{identityId}` (unchanged format; rank = US rank name).
