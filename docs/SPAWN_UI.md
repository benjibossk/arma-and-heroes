# Spawn / Deploy UI — "tactical overhead map"

![design board](screenshots/spawn-ui.png)

## How vanilla does it (studied before coding)

- Menu: `SCR_DeployMenuMain` (`Scripts/Game/UI/Menu/DeployMenu/SCR_DeployMenuBase.c`), preset `ChimeraMenuPreset.RespawnSuperMenu`.
  It opens the map with `SCR_MapEntity.GetMapInstance()` in `EMapEntityMode.SPAWNSCREEN` (config `SCR_MapConfigComponent.GetSpawnMapConfig()`).
  The root handler `SCR_DeployMenuHandler` holds the loadout, group and spawn point selectors (`SCR_LoadoutRequestUIComponent`, `SCR_GroupRequestUIComponent`, `SCR_SpawnPointRequestUIComponent`).
- Spawn points: each `SCR_SpawnPoint` gets an `SCR_MapUISpawnPoint` icon (an `SCR_MapUIElement`) from `SCR_MapUIElementContainer`.
  A click fires `GetOnSpawnPointSelected()` (RplId), which calls the menu's `SetSpawnPointExt` → `SetSpawnPoint`.
  On gamepad, `NextSpawn` / `PrevSpawn` cycle the points.
- Spawning: `RequestRespawn()` sends `SCR_SpawnPointSpawnData(loadout, m_iSelectedSpawnPointId)` through `SCR_RespawnComponent.RequestSpawn`. The server validates the request (server authoritative).
- Deploy timer: `SCR_RespawnTimerComponent` (or `SCR_TimedSpawnPointComponent`) on the game mode.
  The countdown is `GetPlayerRemainingTime(playerId, spawnPoint.GetRespawnTime())`, and each point adds its own `m_fRespawnTime`.
- After spawning, the menu closes and goes straight to the character camera, with an optional fade (`SCR_MenuSpawnLogic`). Vanilla has no spawn cinematic.

## What F44 does

F44 already used vanilla spawning. `F44_CityLayout.ApplySpawns` replaces the HQ `SCR_SpawnPoint`s with the layout points, and `F44_GameModeComponent.ConfigureSpawnPoints` enables only the two battle factions. We keep all of that. Nothing new is decided client-side.

### Server: forward sector spawns — `Scripts/Game/F44/F44_SectorSpawn.c`

`F44_SectorSpawns.Tick` runs from `ServerTick`, right after `F44_SectorManager.Simulate`.

- A sector owned by a side becomes a spawn point for that side when it touches the front, meaning its neighbour in assault order is not owned by the same side.
- The point is a copy of that side's HQ spawn prefab, so the vanilla menu lists it, with:
  - name `SECTOR <name>`
  - `SetRespawnTime(10)`: the **deploy delay penalty**, enforced by the vanilla server timer.
- While the sector is contested (capture progress not full), the point is disabled.
- When the sector is lost or leaves the front, the point is deleted.
- Vehicles and logistics: any vanilla `SCR_SpawnPoint` attached to a vehicle shows up automatically.
  The F44_FieldSupply "loaded box" vehicles do not expose a spawn point yet (out of scope: another agent owns those files). Next step: add an `SCR_SpawnPoint` child to the loaded transport, plus the same penalty.

### Client: overlay on the vanilla menu — `Scripts/Game/F44/F44_DeployMenu.c`

`modded class SCR_DeployMenuMain` overrides `OnMenuOpen/Update/Close/Hide/FocusGained` and drives `F44_DeployOverlay`.

The overlay uses three layouts:
- `UI/layouts/F44_DeployOverlay.layout`
- `UI/layouts/F44_DeployMarker.layout`
- `UI/layouts/F44_DeployLink.layout`

All overlay widgets are `IGNORE_CURSOR`. Clicks and gamepad input still go to the vanilla map, icons and buttons, so console compatibility is unchanged.

| Element | Source |
|---|---|
| Fog outside the battle area (4 dark panels around the sectors' bounding box + 120 m) | `F44_SectorManager` centres → `SCR_MapEntity.WorldToScreen(..., withPan=true)` → `DPIUnscale`, recomputed every frame (pan/zoom) |
| Sector diamonds A/B/C (owner colour, amber ring = contested, sub-label) + links (amber = front, else owner colour) | `GetSectorOwners/GetSectorProgress` (RplProp) |
| Top bar: both sides' names, tickets, bars, sector pips `[A] [B] .`, timer | `F44_GameModeComponent` getters (same data as `F44_Hud`) |
| SOLDIER card | `SCR_PlayerLoadoutComponent.GetLoadout().GetLoadoutName()`; the soldier/loadout is changed with the vanilla loadout list (keyboard + gamepad). The real A&H kit is applied server-side at spawn (`F44_Loadout`), so the card says so. |
| ASSAULT TEAM card | `SCR_GroupsManagerComponent.GetPlayerGroup` → custom name or callsign, members / max |
| DEPLOY panel | "SELECT A SPAWN POINT" → "DEPLOY IN n S" (bar = vanilla timer + point penalty) → "READY" → "DEPLOYING..." (`m_bRespawnRequested`); shows `DEPLOY DELAY PENALTY +10 S` on forward points |

Spawn point icons and selection are the vanilla `SCR_MapUISpawnPoint` ones. We do not duplicate them.

## Not done / to verify

- **Descending top-down camera after spawn:** not implemented. Vanilla closes the menu immediately, so a camera would mean hooking `SCR_RespawnComponent` / a custom `CameraBase` after `OnRespawnResponse`. Planned as a follow-up. The vanilla fade is kept for now.
- **Rank and per-slot weapon icons on the SOLDIER card:** not shown. The client doesn't have the A&H loadout before spawn. Needs a small owner RPC from `F44_LoadoutService`.
- **Overlap:** the cards (bottom corners, Y −372) may overlap the vanilla left/right deploy panels at some resolutions. Adjust in `F44_DeployOverlay.layout` after an in-game look.
- **Verified:** compiles (dedicated server load of the addon, no `SCRIPT (E)` from F44 scripts).
- **Not verified in play:** visual rendering in-game, and forward-point creation (the test battle never reached a sector capture).
