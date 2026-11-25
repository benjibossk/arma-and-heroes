# Heroes & Generals (H&G) content inventory vs. Arma Reforger

> **Source caveat.** The Fandom wiki (heroesandgenerals.fandom.com) returned HTTP 402 to automated fetches on every page
> (Characters, Weapons, Equipment_Points, archive mirrors), so it could not be read directly. This document is built from
> search-engine excerpts of the wiki, GamePressure, Steam guides, NamuWiki and IMFDB, plus general knowledge of the game.
> H&G was shut down in May 2023 and "restored" builds appeared in 2024-25, so numbers are version dependent.
> Values tagged `~` are approximate and **must be re-checked** against the wiki before being hard-coded.
> All text is paraphrased; no images were downloaded.
>
> Reforger side: `catalog.json` (Front 44 logical keys) and `inventory/INVENTAIRE.md`
> (mods: Finest Hour US/GER, Chungus Bolt Guns, M4A2 Sherman, WW2 Tank Core, CIE Tracked Core).
> "Reforger" column = catalog key (C:) or weapon/vehicle prefab name from INVENTAIRE.md (P:); `MISSING` otherwise.
> Candidate Workshop mods are unverified suggestions: Chungus SMGs/Rifles/LMGs/Launchers, OO weapons redone,
> JMPanzerabwehr, M7 rifle grenade (check version/compat before use).

## 1. Character classes

H&G has six classes. Each has a loadout "equipment points" budget (the 10/6 system): every weapon, gadget and
grenade has a point cost, and the sum must stay within the class budget. Budgets below follow the official wiki
excerpt (Infantry 10, Paratrooper 8, Recon 8, Tanker 6, Fighter Pilot 6); one source lists Paratrooper at 6, treat as `~`.

| Class | Role | Typical base weapons | Equipment / gadgets | Points | Unlock |
|---|---|---|---|---|---|
| Infantry (specs: Assault, Support/LMG, Anti-tank, Engineer, Medic, Sniper-lite) | Core ground fighter, holds objectives | Bolt/semi rifle, SMG, LMG, AT launcher by spec | Grenades, medic kit, repair/engineer tools, binoculars, radio (spec dependent) | 10 | Start |
| Paratrooper | Air-dropped infantry, flanks/behind lines | Faction para rifle/SMG (FG42, M1 Carbine, etc.) | Same as Infantry, fewer slots, parachute | 8 `~` | Mid rank, via Paratrooper Assault Team |
| Recon | Scouting, sniping, spotting, designating | Scoped rifles (Kar98k/M1903A4/Mosin scoped), SMG | Binoculars, radio for artillery, light kit | 8 `~` | Early rank |
| Tanker (Tank Crew) | Crews tanks, armoured cars, tank destroyers | Pistol/SMG, crew kit | Repair kit, binoculars | 6 | Around rank 5 per a Steam guide `~` |
| Fighter Pilot | Air superiority / ground attack | Pistol | Parachute | 6 | Rank-gated (late-early game) |
| General (strategy layer) | Not a body: commands the war map, buys Assault Teams | n/a | n/a | n/a | War-map role |

Notes:
- Ranks: H&G has per-class ranks (e.g. "LMG Assault rank 12" gates the Maxim-Tokarev) plus a global character rank.
  Weapons unlock by class-spec rank or by spending Credits/Gold.
- Reforger mapping (catalog): INF ~ Infantry, PARA ~ Paratrooper, REC ~ Recon, ARM crew ~ Tanker. No pilot/General body exists; General maps to the commander/orchestrator layer.

### Reforger coverage of classes (catalog.json)

| Faction | Class keys OK | Missing |
|---|---|---|
| US | RIFLEMAN, SMG, SL, PL, RTO, OFFICER, SNIPER, SR (8) | MG, MEDIC, AT, ENGINEER, CREW, PARA (6): FH US Units 1.0.2 is a shell; compose from stubs |
| GER | RIFLEMAN, RIFLEMAN_G43, MG, MEDIC, AT, LAT, OFFICER, SNIPER, ENGINEER, SMG, SL, PL, RTO, CREW, PANZER_COMMANDER (14 of 15) | PARA (Fallschirmjager) |
| SOV | none | all (no Soviet faction mod known) |

## 2. Weapons by faction

Type codes: R = rifle, SR = semi-auto rifle, SMG, LMG, SN = sniper, AT = anti-tank launcher, P = pistol, RG = rifle grenade, SG = shotgun.
Rank column is the approximate H&G unlock tier (E early, M mid, L late); exact rank numbers are unverified.

### 2.1 United States

| Type | Weapon | H&G unlock | Reforger |
|---|---|---|---|
| SR | M1 Garand | E | P: `Rifle_M1` (C: US_RIFLEMAN) |
| R | M1903 Springfield | E | P: `Rifle_M1903A3` |
| SN | M1903A4 scoped | M | P: `Rifle_M1903A4` (C: US_SNIPER) |
| SR | M1 Carbine | E-M | P: `Rifle_M1_Carbine` / `Rifle_M1A1_Carbine` / `Rifle_M2_Carbine` |
| SMG | Thompson M1A1 | E | P: `Submachinegun_M1A1` (C: US_SMG) |
| SMG | M3 Grease Gun | M | P: `Submachinegun_M3A1` |
| LMG | BAR M1918 | M | P: `Machinegun_M1918` (class US_MG empty, compose) |
| LMG | M1919A4 / A6 | M-L | P: `MG_M1919A4_Dismounted`, `MG_M1919A6` |
| LMG | Johnson M1941 | M | MISSING (candidate: Chungus LMGs) |
| AT | M1A1 Bazooka | M | P: `Launcher_M1A1_Bazooka` (class US_AT empty) |
| P | M1911 | E | P: `Handgun_M1911A1` (C: US_OFFICER) |
| SG | Winchester M12 trench gun | M | MISSING (candidate: OO weapons redone / Chungus) |
| RG | M7/M9 rifle grenade launcher | M | MISSING (candidate: M7 rifle grenade mod) |
| SR | M1 Carbine paratrooper (folding) | para | MISSING (candidate: Chungus Rifles) |
| SMG | Reising M50 | M | MISSING (candidate: Chungus SMGs) |

US: 11 of 15 present (73%).

### 2.2 Germany

| Type | Weapon | H&G unlock | Reforger |
|---|---|---|---|
| R | Kar98k | E | P: `Rifle_Kar98K` (C: GER_RIFLEMAN) |
| SN | Kar98k scoped | M | P: `Rifle_Kar98K_Sniper` (C: GER_SNIPER) |
| SR | Gewehr 43 | M | P: `Rifle_Gewehr43` (C: GER_RIFLEMAN_G43) |
| AR | StG 44 | L | P: `Rifle_STG44`, `Rifle_STG44_Optic` |
| AR | FG 42 (para) | L | P: `Machinegun_FG42G`, `_Optic` |
| SMG | MP40 | E | P: `Submachinegun_MP40` (C: GER_SMG) |
| SMG | MP38 | E | P: `Submachinegun_MP38` (C: GER_PL) |
| LMG | MG34 | M | P: `MG_MG34` (FH GER Vehicles) |
| LMG | MG42 | M-L | P: `MG_MG42` (C: GER_MG) |
| AT | Panzerschreck RPzB 54 | M | P: `Launcher_RPzB54_*` (C: GER_AT) |
| AT | Panzerfaust 60/100 | M | P: `Launcher_Panzerfaust60/100` (C: GER_LAT) |
| P | P38 | E | P: `Handgun_P38` (C: GER_OFFICER) |
| P | Luger P08 | E | MISSING (candidate: Chungus pistols/OO weapons redone) |
| SMG | MP28 | M | MISSING (candidate: Chungus SMGs) |
| SR | Gewehr 41 | M | MISSING (candidate: Chungus Rifles) |
| RG | Kar98k Schiessbecher grenade launcher | M | partial: `Rifle_Kar98K_Scheissbecher` present, check ammo |

Germany: 12 of 16 present, +1 partial (75%, 81% counting partial).

### 2.3 Soviet Union

| Type | Weapon | H&G unlock | Reforger |
|---|---|---|---|
| R | Mosin-Nagant M91/30 | E | P: `BC_Rifle_Mosin_Nagant_9130` (ChungusBoltGuns) |
| SN | Mosin M91/30 PU scoped | M | P: `BC_Rifle_Mosin_Nagant_9130_PU` |
| SR | SVT-40 | M | MISSING (candidate: Chungus Rifles) |
| SMG | PPSh-41 | E-M | MISSING (candidate: Chungus SMGs) |
| SMG | PPS-43 | M-L | MISSING (candidate: Chungus SMGs) |
| LMG | DP-28 | M | MISSING (candidate: Chungus LMGs) |
| LMG | Maxim-Tokarev MT | LMG rank 12 | MISSING (candidate: Chungus LMGs) |
| LMG | DT-29 | M | MISSING |
| AT | PTRD-41 AT rifle | M | MISSING (candidate: JMPanzerabwehr) |
| AT | RPG-43 / RPG-6 grenade | L | MISSING (candidate: JMPanzerabwehr) |
| P | TT-33 | E | MISSING (candidate: OO weapons redone) |
| P | Nagant M1895 | E | MISSING |

USSR: 2 of 12 present (17%). Also no Soviet uniforms/faction/units in the current mod list.

## 3. Vehicles by faction

(H&G fields roughly 15 tanks, 3 tank destroyers and about a dozen light/soft vehicles as of the 2014 content update; the exact list is version dependent.)

### 3.1 United States

| Class | Vehicle | Reforger |
|---|---|---|
| Light car | Willys Jeep | C: US_JEEP (PROXY: vanilla M151A2, off-era) |
| Truck | GMC CCKW / Studebaker | C: US_TRUCK (PROXY: M923A1, off-era) |
| Half-track | M3 Half-track | MISSING (C: US_HALFTRACK) |
| Armoured car | M8 Greyhound | MISSING (C: US_STUART_OR_ARMORED_CAR) |
| Light tank | M5 Stuart | MISSING |
| Medium tank | M4 Sherman (75), M4A3E8 Easy Eight | C: US_SHERMAN = `M4A2_Sherman` (M4A2 75 mm; Easy Eight missing) |
| Tank destroyer | M10 Wolverine, M18 Hellcat | MISSING |
| Heavy/other | M26 Pershing, M24 Chaffee | MISSING |

US: 1 OK, 2 proxy, 8 MISSING (of 11 types: 9% strict, 27% with proxies).

### 3.2 Germany

| Class | Vehicle | Reforger |
|---|---|---|
| Light car | Kubelwagen (+MG34 variant) | C: GER_KFZ, GER_KFZ_MG34 / P: `Kubelwagen*` |
| Truck | Opel Blitz (cargo, open, command, repair, etc.) | C: GER_TRUCK, GER_TRUCK_OPEN |
| AA truck | Opel Blitz + Flak 38 / Flakvierling | C: GER_FLAK_TRUCK |
| Armoured car | Sd.Kfz. 222 / 223 | C: GER_SDKFZ222, GER_SDKFZ223 |
| Armoured car (heavy) | Sd.Kfz. 234 Puma | MISSING |
| Half-track | Sd.Kfz. 251 | MISSING |
| Medium tank | Panzer IV, Panther | MISSING (WW2 Tank Core is a base only) |
| Heavy | Tiger I / Tiger II | MISSING (C: GER_TIGER, not supplied) |
| Assault gun / TD | StuG III, Hetzer | MISSING |
| Static | Pak 40, Pak 36, Flak 38, Flakvierling, leFH 18 | C: GER_PAK40, GER_PAK36, GER_FLAK38, GER_FLAKVIERLING, GER_LEFH18 |

Germany: 6 of 13 vehicle types present, plus 5 statics (46% for mobile; statics are a bonus).

### 3.3 Soviet Union

| Class | Vehicle | Reforger |
|---|---|---|
| Light car | GAZ-67 | MISSING |
| Truck | GAZ-AA / ZiS-5 | MISSING |
| Armoured car | BA-64 | MISSING |
| Light tank | T-70 | MISSING |
| Medium tank | T-34/76, T-34/85 | MISSING |
| Heavy tank | KV-1, IS-2 | MISSING |
| Tank destroyer | SU-85 | MISSING |

USSR: 0%.

## 4. Assault Teams (H&G strategy layer) vs. our team types

In H&G, Generals spend War Funds and Command Points to field Assault Teams; each team has a purchase cost, a per-sortie
resupply cost and a type that decides which classes/vehicles the players can spawn with. Numbers are from a very old
GamePressure table and a wiki talk page (inconsistent, `~`).

| H&G Assault Team | Composition (paraphrased) | Cost to buy / resupply `~` | Our type |
|---|---|---|---|
| Guard | Basic riflemen; simplest, 1 command point | cheap (~14k buy) | INF |
| Infantry | Infantry with specs (LMG, AT, engineer, medic) | ~14k / moderate | INF |
| Motorized Guard | Guard carried by trucks/cars | ~24k | MOT |
| Recon | Scouts, scoped rifles, spotters | ~28.5k / 1.6k | REC |
| Mechanized Recon | Recon with light vehicles | ~24k / 2.8k | REC + MOT |
| Pathfinders | Recon-style advance infantry | n/a | REC |
| Fighter Recon | Recon + air element | n/a | REC (no air) |
| Light Armor | Light tanks / armoured cars | ~44.5k / 3.9k | ARM |
| Tank Destroyer | AT-focused armour | n/a | ARM (heavy AT) |
| Paratroopers | Air-dropped infantry, costliest resupply | ~46k / 24.8k | PARA |
| Fighter Pilots (air teams) | Aircraft | n/a | none (out of scope) |

Mapping to our existing types: INF = Guard + Infantry; PARA = Paratroopers; ARM = Light Armor + Tank Destroyer;
REC = Recon, Pathfinders, Mechanized Recon (vehicle part); MOT = Motorized Guard + trucks/half-tracks.
Costs in `catalog.json` (class 1-2, jeep 2, truck 3, light armour 6, tank 12) already follow a similar cheap-to-expensive ladder;
the main H&G idea worth copying is the separate "resupply cost" per sortie.

## 5. Progression

H&G elements (paraphrased): a global character rank with per-class/spec ranks that unlock gear; Combat Ribbons that track
deeds (up to level 12, with steep point requirements, e.g. one ribbon going from 500 points at level 1 up to 52,000 at level 12);
badges (bronze/silver/gold) and medals awarded on ribbon level-ups; boosters (ribbon +150/200%, joining a war +50%, veteran +25%);
currencies: Credits (soft, buys weapons), Gold (premium), War Funds (team purchases, General layer), Command Points (limit teams in the field).

**Proposal for Arma & Heroes (simple, V3):**

1. Two currencies only: **Credits** (earned per kill/objective/heal/repair, spent on unlocks) and **War Funds** (team-level, spent by the commander on teams; refilled by owned towns). Skip Gold.
2. **Rank** = total XP, 10 ranks (Private ... Colonel). Each rank unlocks one weapon tier (E/M/L from the tables above) and one class (e.g. Recon r2, Tanker r4, Para r6).
3. **Loadout points:** Infantry 10, Para 8, Recon 8, Crew 6. Each prefab in `catalog.json` gets a `points` field (rifle 2, SMG 2, LMG 4, AT 4, sniper 4, grenade 1, medkit 1).
4. **Ribbons (V3.5):** 6 ribbons (Kills, Captures, Medic, Engineer, Armor, Recon), 5 levels each, a cosmetic title plus small credit bonus.
5. **Persistence** server-side in the orchestrator database (per player UID): xp, credits, unlocked weapon keys.

## 6. Summary

| Faction | Weapons present | Class bodies in catalog | Mobile vehicles present | Overall (avg) |
|---|---|---|---|---|
| US | 11/15 (73%) | 8/14 (57%) | 1/11 strict (9%), 3/11 with proxies | ~46% |
| GER | 12/16 (75%) | 14/15 (93%) | 6/13 (46%) | ~71% |
| SOV | 2/12 (17%) | 0 | 0/7 (0%) | ~6% |

### Top 10 priority gaps

1. Soviet faction (units + uniforms + vehicles): nothing exists; biggest hole for a 3-way H&G.
2. US MG, AT, Medic, Engineer, Crew classes (FH US Units shell): compose from stubs; weapons (BAR, Bazooka) already exist.
3. German and US medium/heavy tanks (Panzer IV, Panther, Tiger): needs a tank mod on top of WW2 Tank Core / CIE Tracked Core.
4. US light vehicles that are era-correct (Willys Jeep, GMC truck) replacing the off-era proxies.
5. M3 Half-track (US) and Sd.Kfz. 251 (GER) for the MOT type.
6. Paratrooper classes (US_PARA, GER_PARA) plus aerial spawn support.
7. SMG/rifle gap fillers: PPSh-41, PPS-43, SVT-40, MP28, Reising (Chungus SMGs / Rifles).
8. LMGs: DP-28, DT, Johnson M1941 (Chungus LMGs).
9. AT: PTRD-41, RPG-43 (JMPanzerabwehr), US/GER rifle grenades (M7 rifle grenade mod).
10. Armoured cars and light tanks: M8 Greyhound, M5 Stuart, Puma, BA-64, T-70.

Not covered by H&G import: aircraft (Fighter Pilot class) - out of scope for the current server.
