# Item pictures — sources & credits

Files: `web/static/assets/items/` ; mapping catalog key -> image in `manifest.json`
(`kind: photo` = image taken from a mod, `kind: silhouette` = original Arma & Heroes placeholder).

## Investigation (2026-11-25)
None of the installed mods (Finest Hour Core/US/GER Gear/Uniforms/Units/Vehicles, Chungus, WCS, CIE,
RMS Ostfront Arms, Overlord US vehicles, tank mods) ships a 2D icon/preview for individual weapons or
equipment: Reforger renders inventory previews from the 3D model at runtime, and weapon prefabs have no
UI icon path. The only usable 2D art is Finest Hour's editor previews of characters (HEER only, no US).

## Real images (2)
| Key | File | Source mod | Original resource |
|---|---|---|---|
| GER_OUTFIT_M40 | GER_OUTFIT_M40.webp | Finest Hour — GER Units (FinestHour-GERUnits) | UI/Textures/EditorPreviews/Characters/OPFOR/HEER/Infantry/Character_HEER_Rifleman_Kar98k_Infantry.edds |
| GER_OUTFIT_PANZER | GER_OUTFIT_PANZER.webp | Finest Hour — GER Units (FinestHour-GERUnits) | UI/Textures/EditorPreviews/Characters/OPFOR/HEER/Panzergrenadiers/Character_HEER_PanzerCrew_Panzergrenadiers.edds |

All credit to the Finest Hour team. Extracted with PakTools/pakextract.py, EDDS (LZ4 mips) decoded to 400x300 WebP.

## Silhouettes (32 items)
`_sil_*.svg` — flat Style B silhouettes drawn for this project (rifle, scoped, smg, mg, pistol, launcher,
pfaust, grenade, stick, smoke, medkit, explosive, mine, binoculars, uniform, helmet). No third-party art.
To upgrade later: render weapons in Workbench (EnforceCam / photo mode) and drop `<KEY>.webp` + update manifest.
