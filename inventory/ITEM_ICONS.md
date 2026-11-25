# Item pictures — sources & credits

Files: `web/static/assets/items/` ; mapping catalog key -> image in `manifest.json`
(`kind: photo` = image taken from a mod, `kind: silhouette` = original Arma & Heroes placeholder).

## Investigation (2026-11-25)
None of the installed mods (Finest Hour Core/US/GER Gear/Uniforms/Units/Vehicles, Chungus, WCS, CIE,
RMS Ostfront Arms, Overlord US vehicles, tank mods) ships a 2D icon/preview for individual weapons or
equipment: Reforger renders inventory previews from the 3D model at runtime, and weapon prefabs have no
UI icon path. The only usable 2D art is Finest Hour's editor previews of characters (HEER only, no US).

## Mod-shipped images (0)
The two Finest Hour editor previews used at first (GER_OUTFIT_M40, GER_OUTFIT_PANZER, from
`UI/Textures/EditorPreviews/Characters/OPFOR/HEER/...`) were replaced by in-engine renders (below);
no `kind: photo` entry remains in `manifest.json`.

## Silhouettes (32 items)
`_sil_*.svg` — flat Style B silhouettes drawn for this project (rifle, scoped, smg, mg, pistol, launcher,
pfaust, grenade, stick, smoke, medkit, explosive, mine, binoculars, uniform, helmet). No third-party art.
To upgrade later: render weapons in Workbench (EnforceCam / photo mode) and drop `<KEY>.webp` + update manifest.

## In-engine renders (119)
Rendered in-engine from the mod named in each row (credit to each mod's authors) through Arma Reforger's inventory item
preview (ItemPreviewWidget) by `tools/icon-renderer` (AH_IconRenderer addon, `-ahStudio` CLI flag,
`make_jobs.py` -> game -> `postprocess.py`). The addon spawns its own ItemPreviewManager whose preview world
(`Prefabs/AH/AH_ChromaPreviewWorld.et`) has a flat chroma-green sky (`UI/AH/AH_ChromaSky.emat`, probes desaturated
and lowered so the green does not light the item); `postprocess.py` keys the green out (soft alpha, colour
un-mixing, edge despill), trims to content with 5% padding. WebP with alpha: weapons 512x256, others 512x384.
Not renderable through ItemPreviewWidget (prefab preview shows nothing, even with a fresh PreviewRenderAttributes
or spawned in the world first): GER_GRW34, GER_SW36, GER_TRAILER_SDAH51, US_WC63, US_WC63_TRANSPORT (no manifest entry).
`manifest.json` entries with `kind: render` take precedence over the SVG placeholders.
Still on silhouettes (no prefab / no render): GER_G43, US_M3_GREASE, US_THOMPSON.

| Key | File | Rendered in-engine from |
|---|---|---|
| US_M1_GARAND | US_M1_GARAND.webp | installed WW2 mods (Finest Hour & co.) |
| US_M1_CARBINE | US_M1_CARBINE.webp | installed WW2 mods (Finest Hour & co.) |
| US_BAR | US_BAR.webp | installed WW2 mods (Finest Hour & co.) |
| US_M1903A4 | US_M1903A4.webp | installed WW2 mods (Finest Hour & co.) |
| US_M1911 | US_M1911.webp | installed WW2 mods (Finest Hour & co.) |
| US_BAZOOKA | US_BAZOOKA.webp | installed WW2 mods (Finest Hour & co.) |
| US_MK2_GRENADE | US_MK2_GRENADE.webp | installed WW2 mods (Finest Hour & co.) |
| US_SMOKE | US_SMOKE.webp | installed WW2 mods (Finest Hour & co.) |
| US_MEDKIT | US_MEDKIT.webp | installed WW2 mods (Finest Hour & co.) |
| US_TNT | US_TNT.webp | installed WW2 mods (Finest Hour & co.) |
| US_BINOCULARS | US_BINOCULARS.webp | installed WW2 mods (Finest Hour & co.) |
| US_OUTFIT_M41 | US_OUTFIT_M41.webp | installed WW2 mods (Finest Hour & co.) |
| US_OUTFIT_PARA | US_OUTFIT_PARA.webp | installed WW2 mods (Finest Hour & co.) |
| US_OUTFIT_CREW | US_OUTFIT_CREW.webp | installed WW2 mods (Finest Hour & co.) |
| US_OUTFIT_NETTING | US_OUTFIT_NETTING.webp | installed WW2 mods (Finest Hour & co.) |
| GER_KAR98K | GER_KAR98K.webp | installed WW2 mods (Finest Hour & co.) |
| GER_MP40 | GER_MP40.webp | installed WW2 mods (Finest Hour & co.) |
| GER_MG42 | GER_MG42.webp | installed WW2 mods (Finest Hour & co.) |
| GER_KAR98K_SCOPE | GER_KAR98K_SCOPE.webp | installed WW2 mods (Finest Hour & co.) |
| GER_P38 | GER_P38.webp | installed WW2 mods (Finest Hour & co.) |
| GER_PANZERFAUST | GER_PANZERFAUST.webp | installed WW2 mods (Finest Hour & co.) |
| GER_PANZERSCHRECK | GER_PANZERSCHRECK.webp | installed WW2 mods (Finest Hour & co.) |
| GER_M24_GRENADE | GER_M24_GRENADE.webp | installed WW2 mods (Finest Hour & co.) |
| GER_SMOKE | GER_SMOKE.webp | installed WW2 mods (Finest Hour & co.) |
| GER_MEDKIT | GER_MEDKIT.webp | installed WW2 mods (Finest Hour & co.) |
| GER_MINES | GER_MINES.webp | installed WW2 mods (Finest Hour & co.) |
| GER_BINOCULARS | GER_BINOCULARS.webp | installed WW2 mods (Finest Hour & co.) |
| GER_OUTFIT_M40 | GER_OUTFIT_M40.webp | installed WW2 mods (Finest Hour & co.) |
| GER_OUTFIT_FJ | GER_OUTFIT_FJ.webp | installed WW2 mods (Finest Hour & co.) |
| GER_OUTFIT_PANZER | GER_OUTFIT_PANZER.webp | installed WW2 mods (Finest Hour & co.) |
| GER_OUTFIT_SMOCK | GER_OUTFIT_SMOCK.webp | installed WW2 mods (Finest Hour & co.) |
| GER_TIGER1 | GER_TIGER1.webp | Tiger_H1 |
| GER_TIGER1_DUNKELGELB | GER_TIGER1_DUNKELGELB.webp | Tiger_H1 |
| GER_TIGER1_KURSK | GER_TIGER1_KURSK.webp | Tiger_H1 |
| GER_TIGER1_BASE | GER_TIGER1_BASE.webp | Tiger_H1 |
| GER_PZ4 | GER_PZ4.webp | RMS WW2 PANZER IV |
| GER_PZ4_G | GER_PZ4_G.webp | RMS WW2 PANZER IV |
| GER_PZ4_F2 | GER_PZ4_F2.webp | RMS WW2 PANZER IV |
| GER_PZ4_F | GER_PZ4_F.webp | RMS WW2 PANZER IV |
| GER_PZ4_D | GER_PZ4_D.webp | RMS WW2 PANZER IV |
| GER_PZ4_WIRBELWIND | GER_PZ4_WIRBELWIND.webp | RMS WW2 PANZER IV |
| GER_JAGDPANZER4 | GER_JAGDPANZER4.webp | RMS WW2 PANZER IV |
| GER_PZ4_CHASSIS | GER_PZ4_CHASSIS.webp | RMS WW2 PANZER IV |
| GER_STURMTIGER | GER_STURMTIGER.webp | RMS WW2 Sturmtiger |
| GER_SDKFZ251 | GER_SDKFZ251.webp | SDKFZ251 |
| GER_SDKFZ251_TRANSPORT | GER_SDKFZ251_TRANSPORT.webp | SDKFZ251 |
| GER_SDKFZ251_MG_CAMO | GER_SDKFZ251_MG_CAMO.webp | SDKFZ251 |
| GER_SDKFZ251_TRANSPORT_CAMO | GER_SDKFZ251_TRANSPORT_CAMO.webp | SDKFZ251 |
| GER_SDKFZ251_BASE | GER_SDKFZ251_BASE.webp | SDKFZ251 |
| GER_KFZ | GER_KFZ.webp | FH GER Vehicles |
| GER_KFZ_MG34 | GER_KFZ_MG34.webp | FH GER Vehicles |
| GER_KFZ_CANVAS | GER_KFZ_CANVAS.webp | FH GER Vehicles |
| GER_TRUCK | GER_TRUCK.webp | FH GER Vehicles |
| GER_TRUCK_OPEN | GER_TRUCK_OPEN.webp | FH GER Vehicles |
| GER_FLAK_TRUCK | GER_FLAK_TRUCK.webp | FH GER Vehicles |
| GER_FLAKVIERLING_TRUCK | GER_FLAKVIERLING_TRUCK.webp | FH GER Vehicles |
| GER_TRUCK_COMMAND | GER_TRUCK_COMMAND.webp | FH GER Vehicles |
| GER_TRUCK_REPAIR | GER_TRUCK_REPAIR.webp | FH GER Vehicles |
| GER_TRUCK_ENGINEER | GER_TRUCK_ENGINEER.webp | FH GER Vehicles |
| GER_TRUCK_AMMO | GER_TRUCK_AMMO.webp | FH GER Vehicles |
| GER_TRUCK_ARSENAL | GER_TRUCK_ARSENAL.webp | FH GER Vehicles |
| GER_TRUCK_TANKER | GER_TRUCK_TANKER.webp | FH GER Vehicles |
| GER_SDKFZ222 | GER_SDKFZ222.webp | FH GER Vehicles |
| GER_SDKFZ223 | GER_SDKFZ223.webp | FH GER Vehicles |
| GER_PAK40 | GER_PAK40.webp | FH GER Vehicles |
| GER_PAK36 | GER_PAK36.webp | FH GER Vehicles |
| GER_FLAK36 | GER_FLAK36.webp | FH GER Vehicles |
| GER_FLAK38 | GER_FLAK38.webp | FH GER Vehicles |
| GER_FLAKVIERLING | GER_FLAKVIERLING.webp | FH GER Vehicles |
| GER_LEFH18 | GER_LEFH18.webp | FH GER Vehicles |
| GER_LEIG18 | GER_LEIG18.webp | FH GER Vehicles |
| GER_MG34_TRIPOD | GER_MG34_TRIPOD.webp | RMS Op Ostfront German Arms |
| US_M3A1 | US_M3A1.webp | M3A1 HalfTrack |
| US_M3A1_TRANSPORT | US_M3A1_TRANSPORT.webp | M3A1 HalfTrack |
| US_M3A1_BASE | US_M3A1_BASE.webp | M3A1 HalfTrack |
| US_JEEP_WILLYS | US_JEEP_WILLYS.webp | Operation Overlord US Vehicles |
| US_JEEP_WILLYS_COVERED | US_JEEP_WILLYS_COVERED.webp | Operation Overlord US Vehicles |
| US_JEEP_WILLYS_MG | US_JEEP_WILLYS_MG.webp | Operation Overlord US Vehicles |
| US_JEEP_WILLYS_MEDICAL | US_JEEP_WILLYS_MEDICAL.webp | Operation Overlord US Vehicles |
| US_JEEP_WILLYS_BASE | US_JEEP_WILLYS_BASE.webp | Operation Overlord US Vehicles |
| US_WC63_COVERED | US_WC63_COVERED.webp | Operation Overlord US Vehicles |
| US_WC63_UNCOVERED | US_WC63_UNCOVERED.webp | Operation Overlord US Vehicles |
| US_WC63_MG | US_WC63_MG.webp | Operation Overlord US Vehicles |
| US_WC63_REPAIR | US_WC63_REPAIR.webp | Operation Overlord US Vehicles |
| US_WC63_ENGINEER | US_WC63_ENGINEER.webp | Operation Overlord US Vehicles |
| US_WC63_ARSENAL | US_WC63_ARSENAL.webp | Operation Overlord US Vehicles |
| US_SHERMAN_M4A2 | US_SHERMAN_M4A2.webp | M4A2 Sherman |
| AI_GER_Recruit_Infantry | AI_GER_Recruit_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Recruit_Recon | AI_GER_Recruit_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Recruit_Para | AI_GER_Recruit_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Recruit_ArmorCrew | AI_GER_Recruit_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Regular_Infantry | AI_GER_Regular_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Regular_Recon | AI_GER_Regular_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Regular_Para | AI_GER_Regular_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Regular_ArmorCrew | AI_GER_Regular_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Veteran_Infantry | AI_GER_Veteran_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Veteran_Recon | AI_GER_Veteran_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Veteran_Para | AI_GER_Veteran_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Veteran_ArmorCrew | AI_GER_Veteran_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Elite_Infantry | AI_GER_Elite_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Elite_Recon | AI_GER_Elite_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Elite_Para | AI_GER_Elite_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_GER_Elite_ArmorCrew | AI_GER_Elite_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Recruit_Infantry | AI_US_Recruit_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Recruit_Recon | AI_US_Recruit_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Recruit_Para | AI_US_Recruit_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Recruit_ArmorCrew | AI_US_Recruit_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Regular_Infantry | AI_US_Regular_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Regular_Recon | AI_US_Regular_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Regular_Para | AI_US_Regular_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Regular_ArmorCrew | AI_US_Regular_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Veteran_Infantry | AI_US_Veteran_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Veteran_Recon | AI_US_Veteran_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Veteran_Para | AI_US_Veteran_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Veteran_ArmorCrew | AI_US_Veteran_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Elite_Infantry | AI_US_Elite_Infantry.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Elite_Recon | AI_US_Elite_Recon.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Elite_Para | AI_US_Elite_Para.webp | installed WW2 mods (Finest Hour & co.) |
| AI_US_Elite_ArmorCrew | AI_US_Elite_ArmorCrew.webp | installed WW2 mods (Finest Hour & co.) |
