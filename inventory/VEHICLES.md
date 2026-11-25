# VEHICLES - drivable/crewable inventory (Front 44 WW2 server)

Generated 2026-11-25, read-only, from the `.pak`/`resourceDatabase.rdb` of the 22 mods in `D:\Serveur\arma-reforger-ww2\profile\addons` (method as in INVENTAIRE.md / UNIFORMS.md: PakInspector-style pak extraction of `.et`/`.conf`, GUIDs from the rdb; prefab inheritance chains resolved across mods, vanilla parents are NOT extractable). Machine-readable twin: `inventory/vehicles.json` (keys `GER_*`, `US_*`, `OTHER_*`).

How to read the evidence column
- "Tank class / Vehicle class" = root entity class after resolving the parent chain; `VehicleControllerComponent` present = drivable by a player; `Pilot/Cargo/Turret CompartmentSlot` = real seats; `Editable` = has `SCR_EditableVehicleComponent` (Game Master / spawn catalogs can place it).
- **Seats** = derived crew + passengers from the root prefab plus the seat/turret prefabs it references (e.g. `M3A1_Rear_Seats` = 8 cargo slots). It is a **lower bound** for wheeled vehicles that inherit rear seats from vanilla (non-extractable) parts, and an estimate for tanks (turret prefabs are referenced in several variants). Verify in Workbench before promising capacity.
- BASE/SIMTEST/BACKUP/Chassis prefabs are listed only to say "do not spawn".

## 1. Verdict in 6 lines

1. **New Tiger_H1 gives a playable Tiger I** (3 liveries). WW2TankCore itself still has no hull (it only holds Tiger/Pershing ammo, sounds, BRDM2/UAZ parts).
2. **RMS WW2 PANZER IV gives 6 playable Panzer IV variants + Wirbelwind + Jagdpanzer IV**, **RMS Sturmtiger** gives the Sturmtiger. All three are `Tank` class with VehicleController and turret/loader slots. They live under odd paths (`Prefabs/Vehicles/Wheeled/M151A2/...`) because they were cloned from the vanilla M151A2 tree; names do not collide with vanilla files. Workshop tags mark them "heavy WIP": drive/fire test in-game before relying on them. License is "no derivatives": use unmodified as dependencies.
3. **SDKFZ251 (GER) and M3A1 HalfTrack (US)** are complete halftracks with MG and Transport variants (8 rear seats each).
4. **Operation Overlord US Vehicles** adds the era-correct **Willys Jeep** and **Dodge WC-63** (many variants), replacing the vanilla M151A2 / M923A1 stand-ins used by catalog `US_JEEP` / `US_TRUCK`.
5. **M4A2 Sherman** unchanged: still the only US tank.
6. **No duplicate vehicle prefab exists across the 22 mods** (the only shared path is `Prefabs/Vehicles/Core/Wheeled_Base.et`, identical GUID in FH Core Mod and WCS_Armaments). Overlap is functional, not file-level: see section 4.

## 2. Germany

| Key | Vehicle | Type | Mod | Prefab | Seats | Main weapon | Crewable evidence |
|---|---|---|---|---|---|---|---|
| `GER_TIGER1` | Tiger I H1 (grey) | tank | Tiger_H1 | `{67750217922403FC}Prefabs/Vehicles/Tracked/Tiger_1H/Tiger_H1_Grey.et` | 5 | Tiger1_cannon (88 mm, AP+HE mags), coax MG34, hull MG34 | **yes**: Tank class, VehicleController, Pilot + hull MG + turret gunner + loader + commander cupola slots; Editable, named "TigerH1 Grey" |
| `GER_TIGER1_DUNKELGELB` | Tiger I H1 (Dunkelgelb) | tank | Tiger_H1 | `{E5FD8C6B920C906B}Prefabs/Vehicles/Tracked/Tiger_1H/Tiger_H1_DunkelGelb.et` | 5 | same | **yes**: same as Grey (retexture child) |
| `GER_TIGER1_KURSK` | Tiger I H1 (Kursk camo) | tank | Tiger_H1 | `{FD034D161D354E6B}Prefabs/Vehicles/Tracked/Tiger_1H/Tiger_H1_KurskCamo.et` | 5 | same | **yes**: same as Grey (retexture child) |
| `GER_TIGER1_BASE` | Tiger I H1 BASE | tank | Tiger_H1 | `{93C317E21908A7FE}Prefabs/Vehicles/Tracked/Tiger_1H/Tiger_H1_BASE.et` | 5 | same | **NO (base/WIP/part)**: BASE: no livery, UI name is the generic trunk string; use a livery child instead |
| `GER_TIGER1_SIMTEST` | Tiger I H1 SIMTEST | tank | Tiger_H1 | `{0BA8B33FA94D6966}Prefabs/Vehicles/Tracked/Tiger_1H/Tiger_H1_BASE_SIMTEST.et` | 5 | same | **NO (base/WIP/part)**: test prefab (not editable entity); also BIGWHEELSIM and Core/Tiger_H1_1.2BACKUP are WIP/backups |
| `GER_PZ4` | Panzer IV Ausf H (concrete H; file is named _Base but is the H and parent of G/F2/F/E) | tank | RMS WW2 PANZER IV | `{E44C69F0A2538279}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Ausf_H_Base.et` | 5 | 75 mm long (Cannon_Pnzr_IV_Long_HE + AP mags), hull MG15, commander MG34 cupola, AA HMG option | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_PZ4_G` | Panzer IV Ausf G | tank | RMS WW2 PANZER IV | `{6E1EFC04B7356FE6}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Ausf_G.et` | 5 | 75 mm long, MG15, MG34 | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_PZ4_F2` | Panzer IV Ausf F2 | tank | RMS WW2 PANZER IV | `{3C7E824E65A93FA9}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Ausf_F2.et` | 5 | 75 mm long (Ausf-F2 cannon) | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_PZ4_F` | Panzer IV Ausf F | tank | RMS WW2 PANZER IV | `{70971D2B05042098}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Ausf_F.et` | 5 | 75 mm short (Ausf-E,F1 cannon) | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_PZ4_D` | Panzer IV Ausf E file (UI name says "Ausf D") | tank | RMS WW2 PANZER IV | `{1CC329CBB9C0AFB6}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Ausf_E.et` | 5 | 75 mm short | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_PZ4_WIRBELWIND` | Panzer IV Wirbelwind (AA) | SPAAG | RMS WW2 PANZER IV | `{7E7B98C2756C68DD}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Ausf_WW.et` | 5 | Flak turret (Pnzr_IV_AA_turret) | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_JAGDPANZER4` | Jagdpanzer IV (UI name field is a parse artifact "hatch_gunner") | tank destroyer | RMS WW2 PANZER IV | `{71F142BC3276CE44}Prefabs/Vehicles/Wheeled/M151A2/V2/Jagdpanzer.et` | 4 | Cannon_JagPnzr (75 mm long) | **yes**: Tank class, VehicleController, Pilot slot + hull MG mount + turret (Turret+Loader+commander MG) slots; Editable; model/UI name present |
| `GER_PZ4_CHASSIS` | Panzer IV chassis base (no turret) | chassis (base) | RMS WW2 PANZER IV | `{00889F6B072E47F5}Prefabs/Vehicles/Wheeled/M151A2/V2/Panzer_IV_Chassis_Base.et` | 1 | none | **NO (base/WIP/part)**: Vehicle (not Tank) with only a Pilot slot: BASE ONLY, do not spawn |
| `GER_STURMTIGER` | Sturmtiger (38 cm RW 61) | assault gun (heavy) | RMS WW2 Sturmtiger | `{D49FFA19F12CA057}Prefabs/Vehicles/Wheeled/M151A2/Sturmtiger.et` | 3 | Cannon (Cannon_M242/Cannon.et) firing Ammo_ChudRocket rocket | **yes**: Tank class, VehicleController, Pilot + 1 Cargo (passenger_r01) + turret slot; Editable "Sturmtiger"; inherits vanilla M151A2 tree (derived seats are a lower bound; workshop tag "heavy WIP") |
| `GER_SDKFZ251` | Sd.Kfz. 251 MG | halftrack (armed) | SDKFZ251 | `{561C8A71E69790A1}Prefabs/Vehicles/Sdkfz.251/Sd.kfz_251_MG.et` | 12 | MG42 on pintle (SDKFZ_weapon_mount_base / sdkfz_MG_MG42) | **yes**: Tank class, VehicleController, Pilot + CoDriver + turret slot + SDKFZ_251_RearSeats(_MG) (8 + 2 cargo slots); Editable |
| `GER_SDKFZ251_TRANSPORT` | Sd.Kfz. 251 Transport | halftrack | SDKFZ251 | `{D253646DF96D27EA}Prefabs/Vehicles/Sdkfz.251/Sd.kfz_251_Transport.et` | 10 | none | **yes**: Pilot + CoDriver + 8 rear cargo slots; Editable |
| `GER_SDKFZ251_MG_CAMO` | Sd.Kfz. 251 MG (camo) | halftrack (armed) | SDKFZ251 | `{AB976FB2C94E3BF9}Prefabs/Vehicles/Sdkfz.251/Sd.kfz_251_MG_camo.et` | 12 | MG42 (camo mount) | **yes**: retexture child of MG |
| `GER_SDKFZ251_TRANSPORT_CAMO` | Sd.Kfz. 251 Transport (camo) | halftrack | SDKFZ251 | `{FB19BB480885C8F8}Prefabs/Vehicles/Sdkfz.251/Sd.kfz_251_Transport_Camo.et` | 10 | none | **yes**: retexture child of Transport |
| `GER_SDKFZ251_BASE` | Sd.Kfz. 251 BASE | halftrack (base) | SDKFZ251 | `{4E0A51F1E4013D06}Prefabs/Vehicles/Sdkfz.251/Sd.kfz_251_BASE.et` | 2 | none | **NO (base/WIP/part)**: BASE: UI name "SDKFZ BASE", no rear seats; do not spawn |
| `GER_KFZ` | Kubelwagen | car | FH GER Vehicles | `{1D3D9E6ED1DED30D}Prefabs/Vehicles/Wheeled/Kubelwagen/Kubelwagen.et` | 4 | none | **yes**: Vehicle, VehicleController, Pilot + CoDriver (+ rear seat parts); in HEER vehicle catalog |
| `GER_KFZ_MG34` | Kubelwagen + MG34 | car (armed) | FH GER Vehicles | `{2F09C97004CFD1B5}Prefabs/Vehicles/Wheeled/Kubelwagen/Kubelwagen_MG34.et` | 3 | MG34 | **yes**: Vehicle, VehicleController, Pilot slot + MG34 mount |
| `GER_KFZ_CANVAS` | Kubelwagen (canvas top) | car | FH GER Vehicles | `{0373FCB583E39633}Prefabs/Vehicles/Wheeled/Kubelwagen/Kubelwagen_Canvas.et` | 4 | none | **yes**: variant |
| `GER_TRUCK` | Opel Blitz covered cargo | truck | FH GER Vehicles | `{D0A6F804DFFE34A8}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_transport_covered.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_OPEN` | Opel Blitz open cargo | truck | FH GER Vehicles | `{AEA2E1BD58418A47}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_transport.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_FLAK_TRUCK` | Opel Blitz + Flak 38 | truck (AA) | FH GER Vehicles | `{6CFEE036B08596A6}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_Armed_Flak38.et` | 1 | Flak 38 20 mm | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_FLAKVIERLING_TRUCK` | Opel Blitz + Flakvierling | truck (AA) | FH GER Vehicles | `{EBA339690C35F864}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_Armed_Flakvierling.et` | 1 | Flakvierling 38 4x20 mm | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_COMMAND` | Opel Blitz command | truck | FH GER Vehicles | `{34E9C5F482B11D65}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_command.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_REPAIR` | Opel Blitz repair | truck | FH GER Vehicles | `{55CD8305C5032C21}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_repair.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_ENGINEER` | Opel Blitz engineer | truck | FH GER Vehicles | `{FC9C7C7EFCF8C216}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_engineer.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_AMMO` | Opel Blitz ammo | truck | FH GER Vehicles | `{6CD8B6D9848D7877}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_ammo.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_ARSENAL` | Opel Blitz arsenal | truck | FH GER Vehicles | `{1DA54D8491C30502}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_Arsenal.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_TRUCK_TANKER` | Opel Blitz fuel tanker | truck | FH GER Vehicles | `{974E023D12A97DC7}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_tanker.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot slot (+ codriver/cargo from parent/seat parts; cargo bed carries supplies) |
| `GER_SDKFZ222` | Sd.Kfz. 222 (grey) | armored car | FH GER Vehicles | `{BF2C38CC0B258B42}Prefabs/Vehicles/Wheeled/SDKFZ_222/SDKFZ_222_Grey.et` | 3 | Autocannon KwK 38 20 mm + coax MG34 | **yes**: Vehicle, VehicleController, Pilot + turret slots (2 turret crew); also Camo/Desert/base |
| `GER_SDKFZ223` | Sd.Kfz. 223 | armored car (radio) | FH GER Vehicles | `{DD871923492033E3}Prefabs/Vehicles/Wheeled/SDKFZ_223/SDKFZ_223_base.et` | 3 | MG34 pintle | **yes**: Vehicle, VehicleController, Pilot + pintle turret slot |
| `GER_TRAILER_SDAH51` | Sd.Ah. 51 trailer | trailer | FH GER Vehicles | `{6AC81C73AAA103DD}Prefabs/Vehicles/Wheeled/SdAh51_Trailer/Trailer_SdAh51_transport.et` | 0 | none | **NO (base/WIP/part)**: towed cargo trailer |
| `GER_PAK40` | Pak 40 75 mm | static gun (AT) | FH GER Vehicles | `{FCF55AA8F7F1FECC}Prefabs/Weapons/Artillery/AntiTank/Pak40/AntiTank_Pak40_Grey.et` | 1 | Cannon_Pak_40 | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_PAK36` | Pak 36 37 mm | static gun (AT) | FH GER Vehicles | `{4E7ACC1579DF2772}Prefabs/Weapons/Artillery/AntiTank/Pak36/AntiTank_Pak36_Grey.et` | 1 | Cannon_Pak_36 | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_FLAK36` | Flak 36/37 (88 mm) | static gun (AA/AT) | FH GER Vehicles | `{5A9635F186D9ED7A}Prefabs/Weapons/Artillery/AntiAir/Flak36/AntiAir_Flak36_Grey.et` | 1 | Flak 36 cannon | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_FLAK38` | Flak 38 20 mm | static gun (AA) | FH GER Vehicles | `{CEB82E16B98A70FE}Prefabs/Weapons/Artillery/AntiAir/Flak38/Autocannon_Flak38_Grey.et` | 1 | Autocannon Flak 38 | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_FLAKVIERLING` | Flakvierling 38 (shielded) | static gun (AA) | FH GER Vehicles | `{A1267B6EC31DE460}Prefabs/Weapons/Artillery/AntiAir/Flakvierling/Autocannon_Flakvierling_Shield_Grey.et` | 1 | 4x Flak 38 | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_LEFH18` | leFH 18 105 mm howitzer | static gun (howitzer) | FH GER Vehicles | `{690579024C3EE645}Prefabs/Weapons/Artillery/Howitzer/LeFH18/Howitzer_LeFH18_Grey.et` | 1 | Cannon_LeFH18 | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_LEIG18` | leIG 18 75 mm infantry gun | static gun | FH GER Vehicles | `{A3397E731682B0A1}Prefabs/Weapons/Artillery/Howitzer/Leig18/Howitzer_Leig18_Grey.et` | 1 | Cannon_Leig18 | **yes**: Turret class with TurretCompartmentSlot (gunner); no VehicleController (static); Camo/Desert variants exist; towed Trailer_* variants have VehicleController (wheeled trailer) |
| `GER_SW36` | Sw 36 searchlight trailer | static (searchlight trailer) | FH GER Vehicles | `{6373075B929E4DC7}Prefabs/Weapons/Artillery/AntiAir/SW36/Trailer_SW36_transport.et` | 0 | searchlight | **NO (base/WIP/part)**: Vehicle (trailer), no seats |
| `GER_GRW34` | Granatwerfer 34 81 mm mortar | static gun (mortar) | FH GER Vehicles | `{8BE4C8A97C5FD444}Prefabs/Weapons/Mortars/Grw34/Mortar_Gwr34.et` | 1 | Mortar_Gwr34 | **yes**: Turret class, 2 turret slots |
| `GER_MG34_TRIPOD` | MG34 on tripod | static gun (MG) | RMS Op Ostfront German Arms | `{CF539B000DA7B22D}Prefabs/Weapons/Tripods/Tripod_M122_MG_MG34.et` | 1 | MG34 | **yes**: deployable tripod weapon (RMS Op Ostfront) |

## 3. United States

| Key | Vehicle | Type | Mod | Prefab | Seats | Main weapon | Crewable evidence |
|---|---|---|---|---|---|---|---|
| `US_M3A1` | M3A1 Half-track MG | halftrack (armed) | M3A1 HalfTrack | `{80C7FDE83DE6D000}Prefabs/Vehicles/Tracked/M3A1 HalfTrack/M3A1 HalfTrack_MG.et` | 12 | M2HB pintle (M3A1_Turret -> HMG_M2HB_pintle) | **yes**: Tank class, VehicleController, Pilot + Codriver + turret slot + M3A1_Rear_Seats(_MG) (8 cargo slots); Editable |
| `US_M3A1_TRANSPORT` | M3A1 Half-track Transport | halftrack | M3A1 HalfTrack | `{EC62ED156FC6F05C}Prefabs/Vehicles/Tracked/M3A1 HalfTrack/M3A1 HalfTrack_Transport.et` | 10 | none | **yes**: Pilot + Codriver + 8 rear seats; Editable |
| `US_M3A1_BASE` | M3A1 BASE | halftrack (base) | M3A1 HalfTrack | `{C47A236EC4547CEF}Prefabs/Vehicles/Tracked/M3A1 HalfTrack/M3A1 HalfTrack_BASE.et` | 2 | none | **NO (base/WIP/part)**: BASE (UI name generic trunk string); NewSimulation_..._BASE is the alt physics base: do not spawn |
| `US_JEEP_WILLYS` | Willys Jeep (open) | car | Operation Overlord US Vehicles | `{0CFB9416659192D2}Prefabs/Vehicles/ww2 jeep/Willys Jeep_uncovered.et` | 4 | none | **yes**: Vehicle, VehicleController, Pilot + CoDriver (+ rear bench/seat parts from vanilla M151 tree); Editable |
| `US_JEEP_WILLYS_COVERED` | Willys Jeep (covered) | car | Operation Overlord US Vehicles | `{CC78138E645F75E7}Prefabs/Vehicles/ww2 jeep/Willys Jeep_covered.et` | 4 | none | **yes**: variant |
| `US_JEEP_WILLYS_MG` | Willys Jeep MG | car (armed) | Operation Overlord US Vehicles | `{10448D0351BF0979}Prefabs/Vehicles/ww2 jeep/Willys Jeep_MG.et` | 4 | pintle mount (Willys_Jeep_MOunt; weapon set at spawn/none in prefab) | **yes**: Pilot + CoDriver + turret slot; mount has Turret_Base only (verify weapon in-game) |
| `US_JEEP_WILLYS_MEDICAL` | Willys Jeep Medical | car (medical) | Operation Overlord US Vehicles | `{2B6F31F0D50AB7CD}Prefabs/Vehicles/ww2 jeep/Willys_Jeep_Medical.et` | 5 | none | **yes**: Pilot + CoDriver + patient slot (SCR_PatientCompartmentSlot) |
| `US_JEEP_WILLYS_BASE` | Willys Jeep base (UI "JeepBASE") | car (base) | Operation Overlord US Vehicles | `{99FA6C1DFA682998}Prefabs/Vehicles/ww2 jeep/Willys Jeep_base.et` | 2 | none | **NO (base/WIP/part)**: BASE: do not spawn |
| `US_WC63` | Dodge WC-63 (cargo) | truck | Operation Overlord US Vehicles | `{1DF2AB34BE13692A}Prefabs/Vehicles/Wheeled/WC63/WC63.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_COVERED` | WC-63 covered | truck | Operation Overlord US Vehicles | `{884FF5DF74E94D7F}Prefabs/Vehicles/Wheeled/WC63/WC63_Covered.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_UNCOVERED` | WC-63 uncovered | truck | Operation Overlord US Vehicles | `{AF7DA63E2177F11E}Prefabs/Vehicles/Wheeled/WC63/WC63_Uncovered.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_MG` | WC-63 covered + M2HB | truck (armed) | Operation Overlord US Vehicles | `{B12E900E074349E4}Prefabs/Vehicles/Wheeled/WC63/WC63_covered_mg.et` | 3 | M2HB pintle | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_REPAIR` | WC-63 repair | truck | Operation Overlord US Vehicles | `{6FF58947033154D1}Prefabs/Vehicles/Wheeled/WC63/WC63_Repair.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_ENGINEER` | WC-63 engineer | truck | Operation Overlord US Vehicles | `{CB6185DC2DA8C597}Prefabs/Vehicles/Wheeled/WC63/WC63_Engineer.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_ARSENAL` | WC-63 arsenal | truck | Operation Overlord US Vehicles | `{986F891F5DC8CFAB}Prefabs/Vehicles/Wheeled/WC63/WC63_Arsenal.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_WC63_TRANSPORT` | WC-63 transport (no damage mgr) | truck | Operation Overlord US Vehicles | `{773E8E154D18321C}Prefabs/Vehicles/Wheeled/WC63/WC63_transport.et` | 2 | none | **yes**: Vehicle, VehicleController, Pilot + codriver (+ M2HB mount turret slot); Editable (transport variants: no slots resolved here, seats in referenced parts) |
| `US_SHERMAN_M4A2` | M4A2 Sherman (75 mm) | tank | M4A2 Sherman | `{CFF64B0230B6BD3D}Prefabs/Vehicles/Core/M4A2_Sherman.et` | 10 | 75 mm M3 cannon (AP+HE), coax M240 (invisible), hull MG | **yes**: Tank class, VehicleController, Pilot + hull gunner + turret gunner + commander slots + 5 Rear* cargo (tank riders); Editable; the FakeAxle variant {82ACD6EA85F51C4B} is a physics variant |

## 3b. Other / out of period / libraries

| Key | Vehicle | Type | Mod | Prefab | Seats | Main weapon | Crewable evidence |
|---|---|---|---|---|---|---|---|
| `OTHER_UH1H` | UH-1H (modern, out of period) | helicopter | WCS_Armaments | `{D03C0F6044DB5208}Prefabs/Vehicles/Helicopters/UH1H/UH1H_base.et` | 2 | M261 rockets in Armed variants | **NO (base/WIP/part)**: 2 Pilot slots; base, no VehicleController class match (helicopter sim); not WW2 |
| `OTHER_MI8` | Mi-8MT (modern, out of period) | helicopter | WCS_Armaments | `{AFF6067A5CE8A852}Prefabs/Vehicles/Helicopters/Mi8MT/Mi8_base.et` | 2 | rockets in Armed variants | **NO (base/WIP/part)**: not WW2 |
| `OTHER_TRACKED_BASE` | CIE Tracked vehicle base | base class | CIE Tracked Core | `{4EF4ED8055AB67FC}Prefabs/Vehicles/Tracked/LTM_Tracked_Vehicles_Base.et` | 1 | none | **NO (base/WIP/part)**: library base (Leopard 2 UI name), no WW2 hull |
| `OTHER_TANK_BASE` | WCS Tank_Base | base class | WCS_SpaceCore | `{C2272B5E8EDAB1E2}Prefabs/Vehicles/Core/Tank_Base.et` | 0 | none | **NO (base/WIP/part)**: library base |

Not vehicles: WW2 Tank Core, CIE Tracked Core, WCS_SpaceCore, CIE_Thermal, Chungus Core, FH Core Mod ship only base classes/libraries (`Wheeled_Base`, `Wheeled_Cannon_Base`, `LTM_Tracked_Vehicles_Base`, `Tank_Base`, `Tracked_Base`). There is no Soviet vehicle in any installed mod. FH US Units / US Gear and the HEER/USA vehicle entity catalogs in FH Core Mod list no US vehicle (the HEER Vehicles catalog in FH GER Vehicles references the Kubelwagen family among others; the new mods ship no catalog entries of their own).

## 4. Duplicates / overlaps and which to prefer

| Topic | Candidates | Prefer |
|---|---|---|
| Tiger I | Tiger_H1 (hull) vs WW2 Tank Core (ammo/sounds only) | `Tiger_H1_Grey` (or DunkelGelb/KurskCamo). Avoid `_BASE`, `_BASE_SIMTEST`, `_BIGWHEELSIM`, `Core/Tiger_H1_1.2BACKUP` (WIP/test). |
| Panzer IV | RMS WW2 PANZER IV only (Hinterhalt reskin pack not installed) | `Panzer_IV_Ausf_H_Base` (= Ausf H) as `GER_PZ4`, `_G` as second; not `Panzer_IV_Chassis_Base`. |
| German halftrack | SDKFZ251 only | `Sd.kfz_251_MG` / `_Transport` (camo children available). |
| US halftrack | M3A1 HalfTrack only | `M3A1 HalfTrack_MG` / `_Transport`. The MG/Transport children already inherit the `NewSimulation_` base; `_BASE` variants are not spawnable. |
| US jeep | Operation Overlord Willys vs vanilla M151A2 (off-era, catalog proxy) | Willys (`US_JEEP_WILLYS*`). |
| US truck | Operation Overlord WC-63 vs vanilla M923A1 (off-era, catalog proxy) | WC-63 (`US_WC63*`); it is a Dodge 3/4-ton weapons carrier, so the heavy GMC/Studebaker role stays only approximated. |
| MG34 | FH GER Vehicles `MG_MG34*` (vehicle mounts, tripods) vs RMS Op Ostfront German Arms `MG_MG34` + `Tripod_M122_MG_MG34` | Vehicle mounts use FH/RMS own variants already; for infantry MG keep the existing catalog choice. The RMS tank mods depend on RMS German Arms, which is installed. |
| 88 mm / AT guns | FH GER Vehicles (Pak 36/40, Flak 36/38/Flakvierling, leFH 18, leIG 18, Gw r 34) | Only provider; Grey variants are the neutral livery used in catalog. |
| Helicopters | WCS_Armaments UH-1H / Mi-8 | Out of period; ignore. |

Dependency notes: Tiger_H1 / PANZER IV / Sturmtiger need WCS_SpaceCore + WCS_Armaments (installed); PANZER IV also needs RMS Sturmtiger and RMS Op Ostfront German Arms (installed). All new tank turrets lean on vanilla M242/LAV25-derived parts; the Tiger turret prefabs reference `Character_US_Crew` / `Character_US_CC` as built-in crew characters, so check visually what an AI-crewed Tiger shows.

## 5. H&G vehicle coverage, updated (vs HG_CONTENT.md section 3 and 6)

HG_CONTENT.md baseline: US 1/11 strict (9%), 3/11 with proxies (27%); GER 6/13 (46%); SOV 0/7.

| Faction | H&G type | Before | Now |
|---|---|---|---|
| US | Willys Jeep | proxy M151A2 | **OK** `US_JEEP_WILLYS` |
| US | Truck (GMC CCKW/Studebaker) | proxy M923A1 | **OK/approx** Dodge WC-63 (`US_WC63`) |
| US | M3 Half-track | MISSING | **OK** `US_M3A1` |
| US | M4 Sherman 75 | OK | OK (`US_SHERMAN_M4A2`; Easy Eight still missing) |
| US | M8 Greyhound, M5 Stuart, M10/M18 TD, M26/M24 | MISSING | MISSING (no hull in any installed mod) |
| GER | Kubelwagen, Blitz, Flak truck, 222, 223 | OK | OK |
| GER | Sd.Kfz. 251 | MISSING | **OK** `GER_SDKFZ251` |
| GER | Panzer IV | MISSING | **OK** `GER_PZ4` (+F/F2/G/D/Wirbelwind/Jagdpanzer IV) |
| GER | Tiger I | MISSING | **OK** `GER_TIGER1` |
| GER | Sturmtiger (bonus, not in H&G list) | n/a | `GER_STURMTIGER` |
| GER | Sd.Kfz. 234 Puma, Panther, Tiger II, StuG III, Hetzer | MISSING | MISSING (Jagdpanzer IV is the closest tank destroyer / assault-gun stand-in) |
| GER | Statics: Pak 40/36, Flak 38, Flakvierling, leFH 18 | OK | OK (+Flak 36 88 mm, leIG 18, Gw r 34, Sw 36 searchlight, MG34 tripod now catalogable) |
| SOV | all | MISSING | MISSING |

Updated coverage (mobile vehicle types, same denominators as HG_CONTENT.md):
- **US: 4/11 = 36% strict** (Willys, WC-63 as truck, M3A1, Sherman 75), was 9% strict / 27% with proxies.
- **GER: 9/13 = 69%** (+ Sd.Kfz. 251, Panzer IV, Tiger I), was 46%; 10/13 = 77% if the Jagdpanzer IV is accepted as the TD/assault-gun class. Sturmtiger is a bonus.
- **SOV: 0/7 = 0%**, unchanged.

Remaining gaps, by priority: (1) Soviet vehicles (T-34, GAZ-67, trucks) - nothing installed; (2) US light armor (M8 Greyhound, M5 Stuart) and tank destroyers (M10/M18), M26/M24; (3) GER Panther, StuG III/Hetzer, Puma, Tiger II; (4) US heavy truck (GMC CCKW) and Sherman variants (Easy Eight, M4A3); (5) verification of the new RMS/Tiger prefabs in a real session (WIP tags): spawn each as AI-crewed and as player-crewed, check seat counts, cannon/MG ammo, and that crews do not use US character prefabs.

## 6. Catalog follow-up (do not edit here: another agent owns catalog.json)

Candidate new `catalog.json` prefabs: `GER_TIGER1` -> `{67750217922403FC}Prefabs/Vehicles/Tracked/Tiger_1H/Tiger_H1_Grey.et`; `GER_PZ4` -> `{E44C69F0A2538279}...Panzer_IV_Ausf_H_Base.et`; `GER_SDKFZ251` -> `{561C8A71E69790A1}...Sd.kfz_251_MG.et`; `US_HALFTRACK` -> `{80C7FDE83DE6D000}...M3A1 HalfTrack_MG.et`; `US_JEEP` -> `{0CFB9416659192D2}...Willys Jeep_uncovered.et`; `US_TRUCK` -> `{1DF2AB34BE13692A}...WC63.et`. Full list with GUIDs: `vehicles.json`.

## Free light vehicles (field supply, mod 1.1.0)

Spawned by `F44_CivVehicleSpawner.c`, free for both sides (not in team pools):
- **WW2 bicycle** - Workshop `WW2_Bicycle` 62B4A8E40D31F94B by Modest23, v1.0.21, license APL, dependency Modest VehicleCore 6A7D4E7B69969355 (both added to `D:\Serveur\arma-reforger-ww2\config.json` 2026-11-25, backup `config.json.bak_bike_20261125`). Prefab `{3FDBFE7F1A1EB8F2}Prefabs/Vehicles/Wheeled/WW2_Bike/WW2_Bike_base.et` (no inventory storage). Spawn/respawn verified on a bots-only battle.
- Civilian cars/vans (vanilla Arland): `{23B8C20ACAEC164B}S1203_transport_white`, `{321016E0F9361A22}S105_lightgreen`, `{1D96D899FA313A08}UAZ452_cargo_CIV_blue`.
- No WW2 motorcycle in the current mod set.
