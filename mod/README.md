# F44_Core (Arma & Heroes - Core)

Source of truth: `Documents/My Games/ArmaReforgerWorkbench/addons/F44_Core` (Workbench project,
GUID `EAD65CE1E596E39C`). This folder is a copy (without `Scripts/WorkbenchGame`, the MCP handlers).

## Running the UNPUBLISHED addon on the dedicated server (tested 2026-11-25)

What does NOT work:
- `-config` + F44_Core in `game.mods` (+ `-addonsDir`): the server asks the Workshop for every
  mod of `game.mods` -> `Failed to fetch addon details from workshop API` -> `Unable to initialize`.
- `-config` + `-addons`: refused (`-config cannot be used together with addons!`).
- `-config` + `-gproj`: the server restarts after downloading the config mods and drops the gproj
  (`Unable to read the mission header`).

What works: **command-line mode, no `-config`**, with the addon UNPACKED (no Workbench pack needed):

1. Copy the project to `D:\Serveur\arma-reforger-ww2\localaddons\F44_Core_EAD65CE1E596E39C\`
   (Configs, Missions, Prefabs, Scripts without WorkbenchGame, UI, Worlds, resourceDatabase.rdb)
   and rename `F44_Core.gproj` to `addon.gproj`. The server lists it as a (non packed) addon.
2. Launch (cwd = server dir):
   ```
   ArmaReforgerServer.exe -addr 0.0.0.0:2002 -bindPort 2002 -a2sPort 17778
     -profile D:\Serveur\arma-reforger-ww2\profile
     -addonsDir D:\Serveur\arma-reforger-ww2\localaddons,D:\Serveur\arma-reforger-ww2\profile\addons
     -addons EAD65CE1E596E39C,<the 22 Workshop mod ids of the WW2 config>
     -server {F1F241FEA0522E73}Missions/F44_Test.conf -maxFPS 60
   ```
   `-addr ip:port` sets the RPL listen port (`-bindPort` alone is ignored in this mode: 2001).
   Workshop mods must already be downloaded in `profile\addons` (run the normal WW2 config once).
3. No `gameProperties` in this mode: the orchestrator writes `$profile:F44/server.json`
   (`{battleId, apiUrl, token, timeLimitOverrideMin}`), read by `F44_MissionHeader`.
   Note `$profile:` = `<-profile>\profile\` (so `...\arma-reforger-ww2\profile\profile\F44\`).

`orchestrator.json` keys: `localAddons` (enables CLI mode), `localAddonsDir`, `catalog`
(copied to `$profile:F44/catalog.json`), `timeLimitOverrideMin` (tests only).

Limits of CLI mode: no server name/password/admins/RCON from the config; not listed in the
server browser the same way (direct connect). Once F44_Core is published, go back to `-config`.

## Test scenario

`Missions/F44_Test.conf` (Arland): Finest Hour factions (`FH_USAWW2` / `FH_HEERWW2`), FH spawn
points, 3 sectors, AI world with the GM_Arland navmeshes (without them the bots throw
`Failed move ... to waypoint`). The player loadout manager is still vanilla (US/USSR).
AI groups: catalog keys `US_AIGROUP` / `GER_AIGROUP` (FH `Group_FH_USAWW2_Base` / `Group_FH_HEERWW2_Base`).
