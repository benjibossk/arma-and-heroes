# Heroes & Generals - gameplay mechanics reference

Reference for the tribute mode **Arma & Heroes**: mechanics only, no art, names or text copied.
Complements `HG_CONTENT.md` (content list, assault-team cost table), `HG_RIBBONS.md` (ribbons),
`docs/PROGRESSION.md` (our XP/credits/ranks) and `docs/MEDICAL.md` (our medical design, other agent).
Nothing here repeats those files; it links to them.

**Confidence flags:** `[S]` = confirmed by a source listed at the end. `[M]` = community memory / consistent
with several player accounts but not re-verified (fandom wiki numbers were re-checked by hand in a browser on 2026-11-25, tagged `[W:Page]`;
web.archive.org is blocked from this environment). `[?]` = uncertain, changed across patches, or a guess.
H&G was patched heavily from 2016 to 2022 (closed 2022-11), so many values changed over time; treat numbers as orders of magnitude.

---

## 1. Soldier classes and kits

Each player owns several **soldiers** (character slots), each locked to one faction and one class, each with its
own rank, items and XP `[S]`. Classes `[S]`: Infantry, Recon, Paratrooper, Tank Crew, Pilot. Officers / Generals are a
separate role reached by rank (see section 7).

| Class | Role | Equipment budget | Notes |
|---|---|---|---|
| Infantry | All-round, LMG/AT/medic-style items | **10 points** (largest) `[S]` | Only class with LMGs `[S]`. Heavy load = slow; light load = faster than other classes `[S]` |
| Recon | Scoped rifles, binoculars, spotting, mines/TNT | less than infantry `[M]` (~8) | Spots for team; can be Recon-team only `[S]` |
| Paratrooper | SMGs, carbines, airdrop spawn | less than infantry `[M]` (~8) | Spawns by parachute from the plane over the sector `[M]` |
| Tank Crew | Owns tanks/armoured cars, wrench | small `[M]` | Only class that can drive/gun the tank it owns `[M]`; pistol/SMG only |
| Pilot | Owns fighters / ground-attack planes | small `[M]` | Spawns in the air or on an airfield `[M]` |

**Loadout slots `[M]`:** primary weapon, sidearm, melee (knife / shovel / wrench `[S]`), several item slots
(grenades, first-aid kit, explosives, binoculars), plus clothing. **Weight `[M]`**: every item has a weight;
total weight lowers sprint speed and stamina recovery. The design pushes "light and fast" vs "walking fortress" `[S]`.

**Starter kit `[M]`:** each new soldier got the faction's bolt-action or semi rifle, a pistol, a knife and one
bandage/first-aid item for free; everything else bought with credits (or gold) and gated by soldier rank.

**Weapon upgrades `[M]`:** per-weapon modification slots bought with credits: barrel, magazine (extended), sights
(iron / aperture / scope), stock, ammo type (e.g. high-velocity / AP). Upgrades trade one stat for another
(range vs recoil vs ADS speed). Gated by weapon-ribbon level in later patches `[?]`.

## 2. Health, healing, stamina

| Mechanic | H&G behaviour | Flag |
|---|---|---|
| Health pool | ~100 HP (derived: 36-dmg pistol = 3 body hits); multipliers **head x4, body x1, legs x0.8, arms x0.5** [W:Action_Rules] | `[S]` (mult.) / `[M]` (HP) |
| Medkits | First Aid Kit 9 200 cr (Tactical ribbon gr.5); Medic Pouch "heals 3 kits" 30 000 cr (Medic ribbon gr.6) [W:Equipment, W:Medkits]; heal amount/speed not documented | `[S]` |
| Medic badges | First Aid +40/60/80 % health recovery; Combat Medic +1/2/3 first-aid kits; Heavy Set damage taken 92/89/85 % [W:First_Aid_CB, W:Combat_Medic_CB, W:Combat_Badge] | `[S]` |
| Natural regen | Regenerates automatically **up to 1/3 of max health** only | `[S]` |
| Full heal | Medic/supply **crates** restore full health and refill ammo | `[S]` |
| First-aid kits | Consumable item, heal over a few seconds, refilled at medic crates | `[S]` (refill) / `[M]` (timing) |
| Bleeding | Not a core system (no bandage-to-stop-bleed loop like Arma) | `[?]` |
| Revive | **None.** Death = respawn, costs one ticket of the assault team | `[M]` |
| Fall damage | Yes; long falls kill (parachute spawns avoid it) | `[M]` |
| Sprint/stamina | Stamina bar, depletes on sprint/jump, recovery slowed by carried weight | `[M]` |
| Spawn | Choose a spawn point (owned sector, radio truck / vehicle, para drop) | `[M]` |

## 3. Ammo and supply crates

| Crate | Effect | Where | Flag |
|---|---|---|---|
| Ammunition crate | Refills magazines and throwables; hold-to-use interaction | Placed near capture points + vehicle-mounted | `[S]` |
| Medic crate | Full health + refills first-aid kits / medic pouches | Same | `[S]` |
| AT crate | Gives **one single-use Panzerfaust 60** (only 1 carried at a time; armor dmg 650-950, sights 30/60/80 m) [W:Supply_Crates, W:Panzerfaust_60]; regenerates after a delay | Map-placed, near objectives | `[S]` (content) / `[?]` (delay and count per crate: not on wiki) |
| Vehicle crate | A vehicle (e.g. truck, tank) can carry **one** supply crate at a time; it is a paid vehicle upgrade | Bought by the vehicle owner | `[S]` |

Crate upgrade prices quoted by the wiki: ammunition crate **1 300 credits / 15 gold** (spawn cost 12), medic crate **4 400 credits / 52 gold** (spawn cost 44), AT crate **7 500 cr / 90 gold** (spawn cost 68) `[S]` [W:Supply_Crates].
Ammo crates at **every** capture point; medic crates near capture points `[S]`. Refill rate / hold time / crate XP: not documented `[?]`.
Teammates using your vehicle crate gave you XP (logistics reward) `[M]`.

**Pickup from the dead `[M]`:** you could pick up the weapon of a fallen soldier (it replaced yours for that life), and
take ammo from dropped kits. No full loot/inventory looting.

## 4. Vehicles in battle

| Rule | Behaviour | Flag |
|---|---|---|
| Ownership | Vehicles are owned by a soldier (like weapons); tank crew owns tanks, pilot owns planes | `[M]` |
| Team gate | Armour/planes can only spawn if the side has a matching assault team (Armor / Fighter) on that battle | `[S]` (team types) / `[M]` (gate) |
| Free vehicles | Bicycles, motorbikes, light cars and trucks free for anyone, spawn at sector spawns | `[S]` (bikes, terrain vehicles) / `[M]` |
| Tickets | Spawning an owned vehicle consumes a ticket of its assault team; vehicle death = ticket lost | `[M]` |
| Repair | Wrench (melee item) repairs vehicles; crews repair from inside/outside | `[S]` (wrench exists) / `[M]` |
| Crew | Driver, gunner, commander/MG; infantry can ride as passengers, even on tanks | `[S]` |
| Planes | Fighters / ground attack, bombs, strafing; pilot class only | `[S]` |

## 5. Battles

| Mode | Shape | Flag |
|---|---|---|
| Assault | Attackers push a chain of sectors; defenders hold. Driven by the war map | `[M]` |
| Capture & Hold | Several control points; holding drains enemy tickets | `[S]` (name) / `[M]` |
| Capture & Hold Tank | Tank vs tank with **AI infantry** (2019) | `[S]` |
| Skirmish | Quick small battles off the war map (matchmaking) | `[M]` |

- **Capture**: control points are often a **radio inside a building**; infantry must enter to capture/defend, tanks and planes
  cannot easily do it `[S]`. Contested state shown when both sides are present `[S]`.
- **Tickets**: a battle's tickets come from the assault teams the generals send; each team brings a number of
  lives (team-type dependent) `[S]`. One community post: a battle started once both sides had at least
  ~220 infantry spawns `[S, low confidence]`. A side at 0 tickets loses; ticket difference decides close battles `[S]`.
- **Reinforcement**: generals can send more teams to an ongoing battle to add tickets `[M]`.
- **Team size**: no cap documented `[?]`; Battle Director "fun" threshold = **18 players/faction** (2 factions) or 12 (3 factions / Encounter) `[S]` [W:Battle_Director].
- **Ticket sizing `[S]`** [W:Battle_Director]: spawns counted per player = infantry/para 12, tank/plane/recon 10; >=50 % of players need infantry tickets; "fun" Assault = **216 infantry spawns** (18x12) - confirms the ~220 Steam figure. Lives = AT resources, 1 spawn = 1 resource of a same-class AT [W:Assault_Team]. Per-death/per-vehicle ticket cost per mode: `[?]`.
- **Timers `[S]`** [W:Action_Rules, W:Assault, W:Battle_System]: Assault defence timer **30 min** + capture bonus, then sudden death; attackers with no capture in 30 min are pushed back. Skirmish = first to **100 pts** (2x speed with +2 zones), hard cap 3 h. Prep 5 min; empty server closes after 2 min.
- **Capture times `[S]`** [W:Action_Rules]: normal point 30 s + 30 s (neutral then own), objective 45 s + 45 s, access point 90 s. Contested if an owner died within 80 m in the last 10 s.
- **Spawn delays `[S]`** [W:Action_Rules]: soldier 10 s + 0.5 s/link, tank 10 s + 2.5 s/link, other vehicles 10 s + 1.5 s/link, half-track mobile spawn 20 s, air spawn 15 s. Max active: 1 half-track (Mech Inf), 3 tanks (Armor/TD), 1 transport plane.
- **Conflict**: earlier note "Capture & Hold" mode `[S]` (name) - the fandom wiki documents only **Assault** and **Skirmish** (+ "Encounter"); C&H probably came in a later patch not covered by the wiki.
- **Bots**: no AI in normal infantry battles; AI infantry only in the late "Tank" variant `[S]`.

## 6. War map / strategy layer

| Element | Behaviour | Flag |
|---|---|---|
| Factions | US, Germany, USSR, all at war with each other | `[S]` |
| Map | Strategic map of European towns linked by roads/rails; battles happen when teams meet at a town | `[S]` |
| Generals | Players past an officer rank; buy and move **assault teams**, cannot play normal soldiers on that character | `[S]` |
| Assault teams | **16 types** (Guard, Infantry, Motorized/Mechanized variants, Recon ×3, Light/Medium/Heavy Armor, Tank Destroyer, Fighter Squadron, Fighter Recon, Pathfinders, Paratroopers) `[S]`. Each defines which classes/vehicles spawn and how many lives | `[S]` |
| Team class gate | Infantry → Guard/Infantry types; Recon → Recon types; Tank Crew → armour types; Pilot → fighter types; Para → Pathfinders/Paratroopers | `[S]` |
| Team XP | Teams gain XP in battle and can be upgraded to stronger forms | `[S]` |
| War funds | Third currency. Soldiers earn it by capturing/defending/raiding objectives; generals by team kills/vehicle kills | `[S]` |
| Buy / resupply | Buy teams with war funds; resupply losses after a battle costs several times less than buying `[S]`. Cost table in `HG_CONTENT.md` §4 |
| Resources | Factions get resource deliveries from controlled towns / factories; lines can be cut by capturing a town on the route | `[M]` |
| War end | Capture **15 of 23 capitals** (3 starting capitals each), or hold most capitals when a faction loses all its own. Deploy cost refunded pro rata to remaining resources; winner +250 WF per active AT (1.27) | `[S]` [W:Strategy_Game, W:Victory_conditions] |
| Resupply price | Full resupply ("Deploy") is far cheaper than buying: Infantry 14 000 buy / 3 601 deploy (0.26), Light Armor 44 500 / 3 540, Medium Armor 30 000 / 5 400 (0.18). Per resource: infantry 40, motorcycle 20, car 30, APC 40 WF | `[S]` [W:Assault_Team, W:Warfunds] |
| WF per kill | Infantry 40, tank crew 45, para 110, recon 76, light/medium/heavy armor 165/250/350, TD 275; Veteran +75 % | `[S]` [W:Assault_Team] |
| AT XP | 30 per win / 5 per loss; ranks at >350 / >3 450 / >9 200; Battle Director "not fun" penalty -25 % resources, -20 morale | `[S]` [W:Assault_Team, W:Battle_Director] |
| Factories | **Conflict** with "Resources" row above: wiki has no factory mechanic; production = "Limited Supply" faction stockpile over time, ATs queue. Rates `[?]` | `[S]` [W:Limited_Supply] |
| Faction balance | Population-dependent; underdog bonuses varied by patch | `[?]` |

## 7. Economy and progression

| Item | Behaviour | Flag |
|---|---|---|
| Currencies | **Credits** (earned in battle), **Gold** (paid), **War Funds** (strategy) | `[S]` |
| XP | Objective play gives several times more XP than kills `[S]`. Scoreboard: kill 5, objective capture 28 (5.6x a kill), point 5, TK -10; old Tactical XP: capture 5, kill-a-capturer 10 vs Combat kill 1 [W:Action_Rules] | `[S]` |
| Ranks / salary | 22 ranks, total XP 10 -> 1 000 (r2) -> 101 000 (r12, command 1 AT) -> 29 M (r22); salary 8 500 -> 14 700 cr/h, x1.1 if staying to the end, >50 % presence required; ~4.2 credits per score point [W:Ranks, W:Soldier_salary, W:Credits] | `[S]` |
| Ribbon XP | 12 levels, cumulative 500 / 1 500 / 3 500 / 7 500 / 15 500 ... 184 500; badges at ribbon levels ~4/8/12 (Bronze/Silver/Gold), 1 badge slot (2 Veteran) [W:Combat_Ribbons, W:Combat_Badge]. **Conflict**: booster +150/200 % vs +75 % on the same page | `[S]` |
| Ranks | Soldier rank unlocks items and new soldier slots; reaching an officer rank allows becoming a General | `[S]` |
| Unlock costs | Each item = rank requirement + credit (or gold) price | `[M]` |
| Upgrades | Weapon/vehicle mods bought per item (see §1) | `[M]` |
| Ribbons | Per-weapon / per-skill ribbons, 12 levels — see `HG_RIBBONS.md` | `[S]` |
| Combat badges | Equipped perks earned from ribbons (small stat buffs) — see `HG_RIBBONS.md` §7 | `[S]` |
| Missions | Daily/weekly missions with credit/XP rewards (e.g. "kill N with X", "capture N points") | `[M]` |
| Premium | "Veteran" account: XP/credit boost | `[S]` |
| Medals/prestige | Cosmetic medals; no reset-prestige system | `[?]` |

## 8. Social

| Feature | Behaviour | Flag |
|---|---|---|
| Spotting | Hold aim on an enemy for a few seconds = red marker for the team `[S]`; binoculars (recon) mark longer `[M]` |
| Squads | Small squads, squad spawn on squad mates/leader `[M]` |
| VOIP | In-game proximity/team voice `[M]` |
| Command | Generals message the soldiers in battles their teams joined; no in-battle commander role `[M]` |

---

## 9. What Arma & Heroes should adopt / adapt / skip

Current state from the README roadmap: war map, battles with tickets and AI, assault-team types (INF/PARA/ARM/REC/MOT),
war funds (`PROGRESSION.md` §5), per-soldier XP/credits/15 ranks/unlocks, loadout point budgets (10/8/8/6),
ribbons, HUD + scoreboard. Arma already provides medical, stamina, weight, fall damage, crew roles, repair.

| Priority | Mechanic | Decision | Why / how |
|---|---|---|---|
| P1 | **Supply crates at sectors** (ammo + medic, hold-to-use, full refill) | Adopt | Core H&G loop; without them AI-heavy battles drain ammo. Arma arsenal/resupply entities at each sector spawn |
| P1 | **AT crate with regenerating single-use launcher** | Adopt | Gives infantry a counter to tanks without unlocks; Panzerfaust/Bazooka prefab, respawn timer ~60-90 s |
| P1 | **Vehicle spawn gated by team type + vehicle death costs tickets** | Adopt | We already have team types; enforce "no ARM team = no tank" and ticket cost per vehicle (e.g. tank = 5) |
| P1 | **Resupply cost after battle** (cheaper than buying a new team) | Adapt | Already noted in `HG_CONTENT.md`; `resupplyFactor` now 0.3 (§10) |
| P2 | Free light vehicles (bikes/kübel/jeep) at sector spawns | Adopt | Mobility on Arland; no ownership needed |
| P2 | Vehicle-mounted crate as an owned upgrade + logistics XP | Adapt | Reward XP when others resupply from your truck/half-track |
| P2 | Objective XP >> kill XP | Already done | `PROGRESSION.md` §1 (capture 80 vs kill 15-40) |
| P2 | Weapon mods per item (sights, mags) | Adapt later | Same rank+price model; Arma attachments exist on some WW2 weapons only |
| P2 | Daily/weekly missions | Adopt | Cheap server-side; first-win-of-day exists already |
| P3 | Resource lines / factories, cut by capturing towns | Adapt | Our city income can depend on connected roads to the capital |
| P3 | Team XP / team upgrades (Guard → Motorized → Mechanized) | Adapt | Fits the existing INF/MOT split |
| P3 | Spotting marker | Adopt if cheap | Recon binocular mark via map marker |
| Skip | Regen to 1/3 HP, no revive | Skip | Arma medical (bleeding, bandages, morphine, revive-by-medic) is richer; keep `docs/MEDICAL.md` design |
| Skip | Gold / premium currency, Veteran account | Skip | Community server, no monetisation |
| Skip | Planes / pilot class | Skip for now | Out of scope (`HG_CONTENT.md`) |
| Skip | Weapon weight micro-tuning | Skip | Arma already simulates weight and stamina |

## 10. Tuning changes from H&G research (2026-11-25)

| Tunable | Old -> New | Why |
|---|---|---|
| `web/teams.json` `resupplyFactor` | 0.5 -> **0.3** | H&G full resupply costs 0.18-0.26 of buying a team (Assault_Team table); 0.3 keeps losses meaningful while matching "several times cheaper" |
| `web/progression.json` `xp.capture` | 60 -> **80** | H&G objective capture = 5.6x a kill on the scoreboard; ours was 1.5x a player kill (40), now 2x |

Checked, no change: medical defaults (wiki has no heal/bleed numbers; 2 bandages + medic extras already mirror First Aid Kit + Medic Pouch "3 kits"); field-supply AT crate (count per crate and refill delay undocumented; H&G "1 launcher carried" already matches single-use launchers); ribbons (H&G uses 12 XP levels, we use 3 kill-count tiers by design); war-fund team costs (H&G AT sizes differ too much from ours to transpose).

## Sources

- **H&G fandom wiki** (read in a browser 2026-11-25, `[W:Page]` = `https://heroesandgenerals.fandom.com/wiki/Page`): Action_Rules, Action_Game, Assault, Battle_System, Battle_Director, Assault_Team, Warfunds, Strategy_Game, Victory_conditions, Limited_Supply, Supply_Crates, Panzerfaust_60, Equipment, Medkits, First_Aid_CB, Combat_Medic_CB, Combat_Badge, Combat_Ribbons, Ranks, Soldier_salary, Credits. Pages are undated and mix builds (2015-1.27).

- GamePressure H&G guide, Assault Teams: https://www.gamepressure.com/heroesandgenerals/what-are-the-assault-teams/z37615
- GamePressure H&G guide, Infantry tips (10 points, 1/3 regen, crates, vehicles): https://www.gamepressure.com/heroesandgenerals/infantry/za75f8
- GamePressure H&G guide, General tactics (spotting, AT boxes regenerate, objective XP): https://www.gamepressure.com/heroesandgenerals/general/z975f7
- GamePressure H&G war mode index: https://www.gamepressure.com/heroesandgenerals/war-mode/z17613
- H&G fandom wiki, Supply Crates (crate prices, one crate per vehicle; seen via search snippet, page itself returns 402): https://heroesandgenerals.fandom.com/wiki/Supply_Crates
- Steam discussion (tickets needed to start a battle): https://steamcommunity.com/app/227940/discussions/0/2549465882915898047
- Softpedia changelog (Capture & Hold Tank with AI infantry, 2019): https://games.softpedia.com/progChangelog/Heroes-and-Generals-Changelog-144847.html
- Namu wiki (Korean) overview, ticket difference decides wars: https://namu.wiki/w/%ED%9E%88%EC%96%B4%EB%A1%9C%EC%A6%88%20%EC%95%A4%20%EC%A0%9C%EB%84%88%EB%9F%B4%EC%8A%A4

**Remaining gaps (not on the wiki):** AT crate respawn timer and launchers per crate, per-death/vehicle ticket cost,
max team size, first-aid heal amount, bleeding, free-vehicle counts/respawn, factory production rates.

## 11. Air force teams and planes (wiki PDFs, 2026-11-25)
Source: two user-supplied wiki page exports (Paratrooper assault team; Fighter Planes). Paraphrased, numbers only.
Note: the "Paratrooper" export is the **assault-team** page, not the soldier page — it gives no jump/chute/landing,
loadout or soldier-rank rules. Those stay `[M]` (section 1 / AIRCRAFT_PARA.md) until a soldier-page export is read.

**Para assault teams `[S]`**
- Airforce branch holds 5 team types: Fighter Recon -> Fighter Squadron -> Heavy Fighter Squadron, and Pathfinders -> Paratroopers.
- Paratroopers team: level 1, map speed 2.7, costs 2 command points, carries max 72 para-soldier resources + 18 transport-plane resources; final tier (no upgrade).
- It is the upgraded Pathfinders: same resource kinds, more of them, slightly slower on the map (same pattern as Fighter Squadron vs Fighter Recon).
- Enables the Paratrooper soldier type + one transport plane per side (US: C-47; GER: Ju 52; USSR: lend-lease C-47).
- Commanded by a rank-15 Paratrooper or a General.
- Ratio 4 soldiers : 1 transport plane resource -> transports are a consumable of the team, like soldiers.

**Fighter planes `[S]`** — 3 classes per faction, same price ladder for every side:
| Class | Purchase (credits / gold) | Spawn price | Rearm time | Seats | Rank req. |
|---|---|---|---|---|---|
| Recon plane | 55 000 / 650 | 55 | 60 s | 2 (pilot + rear gunner) | none |
| Fighter tier 1 | 96 000 / 1 150 | 308 | 65 s | 1 | 5 |
| Fighter tier 2 | 138 000 / 1 650 | 442 | 70 s (US 87 s) | 1 | 7 |
| Heavy fighter (twin) | 167 000 / 2 000 | 668 | 87 s (US 97 s) | 2? (blank) | 9 |
| Fighter tier 3 | 188 000 / 2 250 | 451 | 65-75 s | 1 | 12 |

| Side | Recon | Fighter T1 | Fighter T2 | Heavy | Fighter T3 |
|---|---|---|---|---|---|
| GER | Hs 126 (2 MG, 1x250 kg) | Bf 109D (20 mm + 2 MG, 250 kg) | Bf 109E (2x20 mm + MG, 250 kg) | Me 410 (2x20 mm + 13 mm, 250 kg) | Fw 190 D-9 (MG151/131, 500 kg) |
| US | O-52 Owl (.30, small bombs) | P-35A (.50 + .30, small bombs) | P-40N (.50s, small bombs) | P-38 (20 mm + .50s, 250 kg) | P-51D (.50s, 250 kg) |
| USSR | R-Z (2 MG, FAB-100) | MiG-3 (12.7 + 2x7.62, FAB-100) | Yak-9B (20 mm + 12.7, FAB-100) | Pe-3bis (20 mm + 2x12.7, FAB-250) | La-7 (3x20 mm, FAB-100) |
- Every plane carries guns + **one bomb load**; no rockets listed. "Maintenance load" = rearm/refuel time (on runway/airfield `[M]`).
- Spawn price ~ 1/3 of a tier's gold price; heavy fighters cost most per spawn (best ground attack).
- Not in these exports: damage model, AA, per-battle plane limits (see section 5: plane tickets 10, air spawn 15 s), war-map air interaction beyond team gating.

**Adopt for A&H**: AIR team (fighters) and PARA team keep the "resources = soldiers + vehicles" model; transport is a team resource;
planes owned by Pilot class, bought by rank, spawn price per sortie; 1 bomb per sortie, rearm on airfield with a timer.

## 12. Vehicles and mobile spawn (wiki PDFs: SdKfz 251/1 page + Vehicles page, 2026-11-25)
Paraphrased, numbers only. The pages are print exports with the right-hand "comments" column cut off, so some cells are partial.
**Gap:** the wiki gives NO numeric rules for mobile-spawn conditions (stationary / engine off / enemy distance / cooldown / capacity /
destruction penalty). Everything below marked `[M]` is our design, not H&G data.

**Mobile spawn `[S]` (what the wiki states)**
- Infantry-support vehicles act as a spawn point for the whole team: the SdKfz 251/1 (GER) and the M3 half-track (US, also lend-lease to USSR); the CCKW cargo truck (US) and the GAZ-AAA / Blitz truck rows are in the same "Infantry Support Vehicle" category (the CCKW row mentions a mobile spawn, text cut).
- Driver-owned: class restriction Infantry; the driver needs a Chauffeur ribbon (rank 1 at grade 9 for the 251/1).
- Players pick the vehicle as a spawn point on the map; the vehicle is a "spawn point for the rest of your team".
- Locking matters: if a friendly APC is "unlocked" it stops being usable as a spawn point (spawn capability removed). So a vehicle must be in its locked/owned state to count.
- Counterplay: enemies hunt and spawn-camp them; hide it (riverbanks, trees, hills), the MG is a last resort because firing reveals it; the driver's armour shield can be lowered by holding crouch.
- Destroyed vehicle = spawn point gone `[M]` (implied; no number given).

**SdKfz 251/1 stats `[S]`:** type Infantry Support, HP 900 (hull armour multiplier 1.5), 2x 7.92 mm MG 34 (front + rear), max speed 55 km/h, off-road minimum 34, reverse 19, passengers not listed (our 251 mod: 8 rear + 2 crew), price 79,000 credits or 950 gold, spawn (per-deployment repair) cost 386, maintenance load 3:07, ribbon Chauffeur / 9th grade. Camo skins: 130,000 cr / 520 gold and 40,000 cr / 160 gold.

**Upgrades (supply crates) `[S]`:** crates are fitted to the rear for nearby infantry; trucks can carry 2. Three kinds: ammo (incl. grenades/mines), medical, anti-tank. Crate prices: ammo 1,300 cr / 15 gold (spawn 12), medical 4,400 / 52 (44), anti-tank 7,500 / 90 (68; Panzerfaust), each adds 2 s maintenance load; demolition kit and flamethrower crates are battle-pass items (spawn 44-46). Mounted MGs are for terrain vehicles only (own per-spawn repair cost); ammo types and paint for tanks/planes.

**Category -> role `[S]`** (armour is "min-max mm", armament summarised)

| Class | Germany | United States | USSR |
|---|---|---|---|
| Terrain vehicle (MG passenger) | Kubelwagen (MG34/42), Schwimmwagen (amphibious), SdKfz 7/1 Flakvierling | QMC-4 jeep (M1919/M2), M29 Weasel (amphibious), M16 MGMC | GAZ-67 (DShK/DT), lend-lease QMC-4 |
| Motorcycle | R75, R75 sidecar (MG34), Kettenkrad | 42WLA, with sidecar | M72, with sidecar |
| Infantry support (spawn) | Blitz truck, SdKfz 251/1 | CCKW truck, M3 half-track (12.7 mm) | GAZ-AAA, lend-lease M3 |
| Armoured car / recon | SdKfz 250/3, 222 (20 mm), 234/2 Puma (50 mm L/60) | M20 (.50), T8E1, M8 Greyhound (37 mm) | BA-64B, BA-6, BA-11 (45 mm) |
| Light tank | Pz I A (6-15 mm), Pz II C (20 mm), Pz 38(t) E (37 mm), Pz II L | M5A1 Stuart, M24 Chaffee (75 mm) | T-38, T-26, BT-7, T-70 |
| Medium tank | Pz III J/1 (50 mm), Pz IV H (75 mm), Pz V Panther G (75 L/70) | M3 Lee, M4A1, M4A3E8 (76 mm) | T-34/76, T-34/85 |
| Heavy tank | Tiger (88 mm, up to 110 mm armour), Tiger II (up to 180 mm) | M4A3E2 Jumbo, M26 Pershing (90 mm) | KV-85, IS-2 (122 mm) |
| Tank destroyer | Hetzer (75 mm), StuG III G (75 mm), Jagdpanther (88 mm) | M10, M18 Hellcat (76 mm), M36 / M36B1 (90 mm) | SU-76M, SU-85, SU-100 |
| Planes | see section 11 (Hs 126, Bf 109 D/E, Fw 190 D9, Me 410, Ju 52) | O-52, P-35A, P-40N, P-51D, P-38, C-47 | Polikarpov RZ, MiG-3, Yak-9B, La-7, Pe-3bis, C-47 |

Civilian truck, Universal-1 and bicycle exist but are free/none. Tank ammo: 1 shell 100%; 2 shells 60/40; 3 shells 50/30/20. Vehicle credit/gold prices, seats and unlock ranks are not on the Vehicles overview page (only per vehicle page; only the 251/1 was exported).
