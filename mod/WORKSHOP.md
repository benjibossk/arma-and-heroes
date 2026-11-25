# Workshop page — Arma & Heroes - Core

- Visibility: **Public**
- Preview image: `mod/workshop_preview.jpg` (1280×720)
- Tags: Game Mode, Multiplayer, Scenario, Historical, WW2
- Dependencies: Arma Reforger only (the server adds the content mods, see below)

## Summary
A persistent WW2 war: generals move the front on the web, soldiers fight it in Arma Reforger.

## Description
Arma & Heroes is a persistent World War II campaign for Arma Reforger.

On the website (heroes.collectifxxl.fr), generals raise assault teams and move them across a strategic war map of Normandy. When two teams meet in a city, a battle starts: our orchestrator launches a dedicated server with exactly the troops, vehicles and tickets the generals brought. The result goes back to the war map.

In game:
- Assault and Capture & Hold on sectors, with tickets from the assault teams
- AI bots fill both sides, capture sectors and cost tickets when they die. Their skill and look adapt to the players in the match
- Vehicle pool from the teams: jeeps, trucks, halftracks, Sherman, Panzer IV, Tiger I
- Persistent soldiers: rank, credits, unlocked weapons and loadouts from the website
- Link your game account to Discord with a one-time code or QR code (works on console)
- Live battle view on the website

This mod is the game mode only. Servers running it also load Finest Hour, Chungus and WW2 vehicle mods; all credit goes to their authors.

Source code (MIT): https://github.com/benjibossk/arma-and-heroes

## Changelog (first release)
First public release: game mode, sectors, tickets, AI bots, vehicle pool, loadouts, account linking, live view.

## Changelog 1.0.1 (ready, not yet published by this change)
Battle results are now more reliable: the result POST to the website is retried up to 3 times with backoff (5 s, 10 s), and an HTTP 409 reply (battle already finished) counts as success. Fixes a battle reported as failed (http 0) when the website answered slowly although it had saved the result.

## Changelog 1.1.0 (ready, not yet published by this change)
- AI vehicles: mounted groups follow a chain of road-snapped move waypoints (20-25 m completion radius) and get a cruise speed (transports 45 km/h, armor 28 km/h); stuck vehicles (< 0.5 m/s for 30 s) are re-routed.
- Transports unload about 150 m before the sector, then the driver alone brings the vehicle back to its spawn where it is freed for the next group. Groups still at their spawn pick up respawned vehicles.
- Armor drives to the sector edge facing the attackers and supports from there (attackers and defenders alike; defender tanks no longer just sit on the sector).
- Several vehicle slots per side (2 to 6, from the team pools), lined up along the nearest road; pool vehicles keep respawning in them until the pool count is used up.
- City layouts: Configs/F44/layouts.json defines battle locations on Arland (Arleville, Beauregard, military base, airfield) with 3-4 sectors on real buildings / crossroads, spawn zones per side and road vehicle anchors. A battle loads the layout of its city (cityMap: website cityId -> layout, or battle "layout" / inline "layoutData"); clients rebuild the same sectors. Without a layout the world sectors are used as before.
- Dev: server-side Arland location dump ($profile:F44/dump_locations -> locations.txt). Movement telemetry in the console log ("[F44] AIVEH ... speed=... toDest=...").
- Medical (F44_Medical.c, docs/MEDICAL.md): every soldier (players and AI) is topped up to 2 field dressings at spawn, medics to 6 dressings + 2 morphine + 1 saline; a bandage also heals 20 HP; a medic treating someone else heals to full at double speed; unconsciousness on, only a medic revives (60 s bleed-out); healGiven / revives / stats.bandagesApplied / stats.healHp in the BattleResult. Optional $profile:F44/medical.json override.
- Field supply (F44_FieldSupply.c): free AT crates near each side spawn and next to each sector. "Take Panzerfaust 60" (GER) / "Take Bazooka" (US) and "Take AT grenade" (Geballte Ladung / M7AT), regardless of unlocks; limited stock per crate, full refill every N minutes. Max ONE free launcher per player, it must stay on the soldier (refused in backpacks/boxes/vehicles, removed if left on the ground).
- Ammo and med boxes near the spawns: hold the action to get one magazine for your weapon / one heal step (stops bleeding, +25% health) per completion; limited charges with slow regen. "Load box on nearest vehicle" turns any vehicle into a mobile logistics point; the loader is credited (resupplyGiven / boxHealGiven in the BattleResult).
- Vehicle storage disabled during battles (all vehicles, anti-hoarding); bodies stay lootable.
- Free light vehicles (F44_CivVehicleSpawner.c): WW2 bicycles (new dependency WW2_Bicycle 62B4A8E40D31F94B by Modest23 + Modest VehicleCore 6A7D4E7B69969355) and civilian cars/vans (S1203, S105, UAZ452) by the roads near each side spawn, capped per side, respawn after destruction or when abandoned far away.
- All counts/timers in the battle JSON "fieldSupply" object (cratesPerSector, cratesPerSpawn, launchersPerCrate, grenadesPerCrate, crateRefillMin, ammo/medBoxesPerSpawn, ammo/medBoxCharges, boxRegenPerMin, healStep, looseLauncherSec, civVehicles, bikesPerSide, carsPerSide, vehRespawnSec, vehAbandonDist, vehAbandonSec); explicit points via "supplyCrates"/"civSpawns" [{type: AT|AMMO|MED|BIKE|CAR, side, x, z, yaw}] in fieldSupply or in layouts.json; else fallback around spawns/sectors.

## Changelog 1.2.0 (draft, not published)
- Mobile spawn: SdKfz 251 / M3A1 halftracks (8 uses) and cargo trucks (3 uses) become a team spawn point when stopped (< 2 km/h for 3 s) with a friendly driver, no enemy within 40 m and outside a contested sector; 20 s cooldown between spawns, +10 s deploy penalty, removed when destroyed or the driver is gone for 15 s. Shown as MOBILE in the deploy menu. Spawn timers (base 10 s, forward sector +10 s, mobile +10 s, aligned on H&G) configurable in the battle JSON (spawnTimes / mobileSpawn).
- Paratrooper air drop (optional, battle JSON `airDrop`): an AI-flown C-47 circles the battle area; PARA soldiers deploy on the "AIR DROP" spawn point straight into a cargo seat (30 seats, unavailable when full), jump through the cargo door when they want and open the chute manually (Space). Respawns 90 s after being lost. Uses AeroplaneHavens C47 Dakota + Aviation-Toolkit + USAAF Bomber Attire and Grey Mans Chute Framework when loaded on the server (soft integration, not hard dependencies).
