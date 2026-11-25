# Battle maps (worlds)

Battles ran only on Arland (`Missions/F44_Test.conf`). F44_Core now ships one mission per WW2 map;
the orchestrator picks the mission (and the extra Workshop mods) from the battle's `world`.

## Chain

1. **Website** (`web/maps.go`): `cityWorlds[cityId] = {world, layout}`. Sent in the battle JSON as
   `"world"` / `"layout"` **only** when the world is listed in env `F44_MAPS_ENABLED`
   (comma list, e.g. `ww2arland,belleauwood`). Default: nothing sent = Arland, as before.
2. **Orchestrator** (`orchestrator/maps.json`): `worlds[world] = {enabled, scenarioId, mods[]}`.
   `build_config` sets `game.scenarioId` and appends the map mods to the template's `game.mods`.
   `enabled: false` -> the battle is not launched (logged). `allowDisabledMaps: true` in
   orchestrator.json overrides it (tests only). No `world` = template unchanged.
   The live `config.json` is **not** modified.
3. **Mod**: `Missions/F44_<Map>.conf` -> `Worlds/F44_<Map>.ent` (SubScene of the map's base world, not
   its Game Master world) + layer with the F44 entities and the map's navmesh.
   `layouts.json` layouts carry `"world"`; a layout whose world is not loaded is refused
   (warning, falls back to the world sectors) so a wrong mission never spawns players in the sea.
   Battle `"world"` is checked too (warning `expects world ... but ... is loaded`).

## Maps (client copies inspected with a pak parser, 2026-11-26)

| world key | Workshop | base world | terrain | own size | deps | total download |
|---|---|---|---|---|---|---|
| `arland` | vanilla | `Arland.ent` | 4 km | - | - | 0 |
| `ww2arland` | WW2 Arland Map `6072D68D8DCD0C12` 0.0.34 (iRON79) | `{5015A2E872C648D7}worlds/Arland/WW2Arland.ent` | 4.1 km (= Arland heightmap) | 158 MB | none | **158 MB** |
| `belleauwood` | Belleau Wood `629EDC37CDE3AD35` 2.3.43 (Thinwheat) | `{340ED3C1F743457E}Worlds/BelleauWood/BelleauWood.ent` | 12 km | 1215 MB | Rail Enfusion Redux `5DE612963E4B005A` (7 MB), Railway Generator `5F0D245931200FD1` (<1 MB) | **~1.22 GB** |
| `normandy` | Normandy `616D67C41B1DF93D` 0.0.39 (JustMichael) | `{55B6E6A52487C726}Worlds/TESTIsland.ent` | 4.9 km | 527 MB | WW2 Assets `647804D31EC300F0` (1788 MB), iRON79 Compositions `60A68ED7293C691A` (514 MB) | **~2.83 GB** |
| `utahbeach` | UTAHBeach `698C2A44E9972692` 1.0.16 (MorkMasher, GodSilent) | `{B523869E8DFC31DF}Worlds/Layers/Operation OverLord/unnamed.ent` | 5.1 km | 285 MB | WW2 Assets, iRON79 Compositions, Rail Enfusion Redux, Railway Generator | **~2.6 GB** |

`gameVersion` is empty in every ServerData.json (no minimum declared). The Workshop versions above are the client copies.

**Compatibility.** None of the maps or deps ships scripts, except Railway Generator (3 `RER_*` classes,
own names). None overrides weapons, characters, factions, configs or magazine wells, so there is no conflict
like OO's `WW2_MagWells` with Finest Hour. Each world gets its own SubScene; the F44 layer leaves out
entities the base world already has (Belleau: PerceptionManager; Utah: RadioManager).
Navmesh: Belleau `SCR_AIWorld_BelleauWood.et`; Normandy `WW2_Soldier/WW2_BTR.nmn`; Utah `Terrains/soldier|btr.nmn`;
WW2 Arland `GM_WW2Arland(_vehicles).nmn`.

## Named locations (map descriptors)

- **WW2 Arland**: the Arland names (Arleville, Beauregard, Highfield, Signal Hill, the military bases, the airfield), with WW2 buildings,
  ruins and bunkers from iRON79's layers (EastCoastDefense, Fort, Bunkers).
- **Belleau Wood**: Belleau, Belleau Wood, Torcy-en-Valois, Givry, Bouresches (+station), Lucy-le-Bocage, Triangle farm, Vaux, Fort de Vaux,
  Monneaux, Coupru, Bussiares, Licy Clignon, Monthiers, Etrepilly, Champillon, Marigny, Hill 142, Hill 193, Hunting Lodge, Brussels/Paris/Le Mares farms, Outpost Bravo.
- **Normandy**: Sainte Mere, Brecourt Manor, Hubert, La Rue (sparse: about 280 buildings).
- **Utah Beach**: Carentan, Saint-Come-du-Mont, Sainte-Marie-du-Mont, Ste-Mere-Eglise, Chef-du-Pont, La Fiere (bridge / causeway),
  Cauquigny, Le Vey, Angoville-au-Plain, Brecourt, La Madeleine (beach exit), Foucarville, Frivole, Azeville, Fresville, La Gare,
  Le Brot, Quineville, Montebourg.

## Battle layouts (`mod/F44_Core/Configs/F44/layouts.json`)

How they were made: sectors are real buildings, taken from the world layers (or from the server dump for WW2 Arland) and split along
the attack axis. Spawns are next to a building, or on open ground near one, above sea level. The mod snaps vehicle anchors to the nearest road.
Each layout has one AMMO supply point between sectors B and C and one bike per side next to the spawns.

| world | layout id | sectors |
|---|---|---|
| ww2arland | `ww2-arleville` | Highfield crossroads, Arleville pub, Farmhouse, Chapel |
| ww2arland | `ww2-beauregard` | West houses, Drugstore, Workshop, Harbour warehouse |
| ww2arland | `ww2-military-base` | Water station, Medical barracks, Bunkers |
| belleauwood | `bw-belleau`, `bw-bouresches`, `bw-lucy`, `bw-torcy` | 4 building sectors each |
| normandy | `nor-sainte-mere`, `nor-brecourt` (3), `nor-hubert` (3) | |
| utahbeach | `utah-sainte-mere-eglise`, `utah-carentan`, `utah-sainte-marie-du-mont`, `utah-montebourg` | 4 each |

Before opening a map, check each layout in game (Game Master camera, or the `[F44] Layout:` log plus a quick fly-over).
The layouts on Belleau, Normandy and Utah come from layer data only and have not been played.
To regenerate the building list on a map, set the `$profile:F44/dump_locations` flag (`F44_WorldDump.c`).

## Enabling a map

1. Approve the download: add the mods in the `maps.json` row to the battle servers. They download from the Workshop
   in `-config` mode, or go into `profile\addons` in CLI mode.
2. Publish or copy the F44_Core version that has `Missions/F44_<Map>.conf`.
3. Set `"enabled": true` in `orchestrator/maps.json`.
4. Add the world to `F44_MAPS_ENABLED` on the website, then map the war cities in `web/maps.go`.
   Clients download the map when they join; the server browser shows the mod list.

## Tested

- 2026-11-26, isolated server (port 2013, scratchpad profile, CLI mode, WW2 Arland from the client copy):
  `F44_WW2Arland.conf` loads, the F44 game mode starts the battle, the AI director spawns both sides and the vehicles drive, and the location dump works.
  Then a battle.json with `world: ww2arland` and `layout: ww2-beauregard`: `[F44] Layout: 'ww2-beauregard'`, 4 sectors, 4 spawns and 3 supply points placed, battle running (status at t=180 s).
- Belleau, Normandy and Utah: mission and world files are generated and the GUIDs come from the map paks, but these maps have not been launched
  (1.2 to 2.8 GB of downloads to approve first).
