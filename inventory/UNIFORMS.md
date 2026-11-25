# UNIFORMS.md - WW2 appearance inventory and AI appearance tiers

Generated 2026-11-25 from `inventory/extract/` (.pak extraction of the 22 installed mods). Server and mods untouched. GUID format: `{GUID}Prefabs/...et`. Mod column abbreviations: **FH** = Finest Hour.

## 0. Scope and key findings

* Every WW2 character asset lives in exactly 6 mods: **FH Core** (base chars/catalogs), **FH GER Uniforms**, **FH US Uniforms**, **FH GER Units**, **FH US Units**, plus FH GER/US Gear (only pouches/backpacks/kits - **no clothing**). All other mods (RMS Op Ostfront German Arms, Operation Overlord US Vehicles, Chungus, M4A2/M3/SdKfz 251/Tiger/PzIV/Sturmtiger, WW2 Tank Core, WCS, CIE, Northcom voice pack) are weapons/vehicles/sound and contain **no uniform, helmet, vest or character prefab**.
* Only **two armies** are available: **GER (Heer + Luftwaffe)** and **US Army**. The FH Core also lists `USSR_WW2`/`FIA` catalog entries, but there are no Soviet uniforms/characters (only 2 FIA modern Javelin/Igla characters from vanilla, 2 `USSR_Army` vanilla files). No British/Canadian/Waffen-SS faction.
* **No SS-specific gear exists** (no SS runes, no Totenkopf, no Waffen-SS Erbsenmuster/Plane-tree/Oak-leaf camo). The only GER camo patterns are **Splinter (Splittertarn)** on the M43 cap, M38/M42 helmet covers and the (incomplete) M38 FJ smock, plus *Painted* helmets, nets, green cover and winter-white helmets. No Italian camo, Tan Water, Plane-tree, or Erbsenmuster.
* US has **no camo uniform** (no camo HBT / Beetle-Frog / Mitchell). Camouflage is limited to helmet Netting / Scrim. US variety comes from jacket (M37 / M41 / M42 para / M43), tanker/pilot suits and the Airborne M1936 vest family.
* Insignia flag: all GER headgear/jackets are standard Wehrmacht or Luftwaffe pattern and may carry the Wehrmacht/Luftwaffe eagle (eagle clutching a wreath/swastika, depending on texture). Textures are not visually verified. Items tagged **[LW]** are Luftwaffe (blue-grey, Luftwaffe eagle). No item name contains SS / Totenkopf / swastika. To stay clean, tiers below avoid Luftwaffe items and the Officer/Schirmmuetze caps on rank-and-file.
* **Gaps found in existing character prefabs**: (a) FH US Units 1.0.2 is a skeleton: only 8 Infantry roles have equipment, all other 56+ US characters (ArmoredInfantry, CavRecon, Para stubs) inherit `BaseLoadout` = garrison cap + M41 jacket, no helmet/vest. (b) `Character_FH_USAWW2_BaseLoadout_Paratrooper` is empty. (c) The US Infantry SL prefab references `Helmet_M1_ww2_SquadLeader.et` via GUID `{89EB785CDE795008}`, which resolves to `Helmet_M1_ww2_NCO.et` (name mismatch is harmless). (d) `Jacket_HEER_M38_*` (FJ smock) base prefab has **no worn model**, only a material override pointing to a vanilla BDU M81 material: it probably renders wrong/invisible, test before use. (e) `Pants_HEER_Springerhose` has only a `_base`.

## 1. Germany (OPFOR / Heer + Luftwaffe)

### GER - Headgear (helmets, covers, caps)

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{C3C3B64455FC06C6}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Feldgrau.et` | M40 field cap (Feldmuetze), field grey - plain, no helmet |  |
| `{A2204C99FAEE7437}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Feldgrau_Officer.et` | M40 field cap, officer piping |  |
| `{6B9A925F761EA72E}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Luftwaffe.et` | M40 field cap, Luftwaffe blue-grey (Luftwaffe eagle) | Luftwaffe eagle |
| `{B7E00EB4AEC4CDA9}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Luftwaffe_Officer.et` | M40 field cap Luftwaffe officer | Luftwaffe eagle |
| `{C239E728425F9297}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_TankCommander.et` | M40 black Panzer cap, commander |  |
| `{79770150D9286630}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_TankCrew.et` | M40 black Panzer cap, crew |  |
| `{A34221AB5A70F92C}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Feldgrau.et` | M43 Einheitsfeldmuetze (peaked field cap), field grey |  |
| `{C01593243CEF1CD0}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Feldgrau_Officer.et` | M43 field cap, officer |  |
| `{8D09738BB0973263}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Luftwaffe.et` | M43 cap Luftwaffe | Luftwaffe eagle |
| `{11A1834F37D76D7E}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Luftwaffe_Officer.et` | M43 cap Luftwaffe officer | Luftwaffe eagle |
| `{F6A9BD9EA6F00C13}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Splinter.et` | M43 cap in Splinter (Splittertarn) camo |  |
| `{D35C610DE45084FA}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_TankCommander.et` | M43 cap, Panzer black, commander |  |
| `{868355C2B87B0AB2}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_TankCrew.et` | M43 cap, Panzer black, crew |  |
| `{BE55599A2F4F26D7}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Feldgrau.et` | Schirmmuetze peaked officer cap, field grey |  |
| `{9EFF51DCB14F4F20}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Luftwaffe.et` | Schirmmuetze Luftwaffe | Luftwaffe eagle |
| `{47ECF4DCE5F7DB8F}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Panzer.et` | Schirmmuetze Panzer (black, pink piping) |  |
| `{C962F894BC540739}Prefabs/Characters/HeadGear/Helmet_LkpW10_01/Helmet_LkpW10_01.et` | LKP W/10 (Luftschutz/early helmet) |  |
| `{EFEEC9D16CB32D5F}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_01.et` | Stahlhelm M35 plain feldgrau |  |
| `{D3CA375028BCFBDC}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Blue.et` | M35 Luftwaffe blue-grey | Luftwaffe eagle |
| `{811C6EEB0B5CEF42}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Medic.et` | M35 with red-cross medic marking |  |
| `{470AD17E10E1847D}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Netting.et` | M35 with garnish net |  |
| `{D87EBB4343F30CF5}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Painted.et` | M35 painted camo (sand/green spots) |  |
| `{453C9F329FAD3293}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Painted_Netting.et` | M35 painted camo + net |  |
| `{5043B8775B038535}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_White.et` | M35 winter white |  |
| `{D62A72E894FA605B}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_01.et` | M38 Fallschirmjaeger helmet (no rim), plain |  |
| `{5AE56AB1C4C4C5EB}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Blue.et` | M38 FJ helmet Luftwaffe blue-grey | Luftwaffe eagle |
| `{0EFFE36965141049}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Green_Cover.et` | M38 FJ helmet with green cloth cover |  |
| `{882D47A6BB3F43A2}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Medic.et` | M38 FJ helmet medic |  |
| `{122B49F881289F42}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Medic_Blue.et` | M38 FJ helmet medic, blue | Luftwaffe eagle |
| `{C56B9D549F8B495A}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Painted.et` | M38 FJ helmet painted camo |  |
| `{4E1FC79E3E28A143}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Splinter_Cover.et` | M38 FJ helmet with Splittertarn cover (Splinter) |  |
| `{E975113A3BA2E57B}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Blue.et` | M42 Luftwaffe blue-grey | Luftwaffe eagle |
| `{307C507641AE33C7}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Cover_Splinter.et` | M42 with Splinter helmet cover |  |
| `{747BA3B3D60B3676}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Feldgrau.et` | M42 plain feldgrau |  |
| `{83FA82A5EB3172EB}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Medic.et` | M42 medic |  |
| `{19463B1006FA1EF4}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Net.et` | M42 with garnish net |  |
| `{049D5DAB3BA65BB1}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Painted.et` | M42 painted camo |  |
| `{4A446BA39DF5DAD2}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Painted_Net.et` | M42 painted camo + net |  |
| `{52A55439BB6E189C}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_White.et` | M42 winter white |  |

### GER - Uniforms (jackets, trousers, suits)

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{4DEC71CFC20990F4}Prefabs/Characters/Uniforms/Blouse_M36/Jacket_HEER_M36_Feldgrau.et` | M36 Waffenrock/blouse feldgrau (early, 4 pockets) |  |
| `{9D6868213E70EC18}Prefabs/Characters/Uniforms/Blouse_M36/Jacket_HEER_M36_Luftwaffe.et` | M36 blouse Luftwaffe | Luftwaffe eagle |
| `{B7E538F48ED5B12F}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_Feldgrau.et` | M40 blouse field grey (Heer, 1940-43) |  |
| `{051E32E58B7C5853}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_Luftwaffe.et` | M40 blouse Luftwaffe blue-grey | Luftwaffe eagle |
| `{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et` | M43 blouse field grey (late war, field-grey-green) |  |
| `{AD732800E3688E96}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Luftwaffe.et` | M43 blouse Luftwaffe | Luftwaffe eagle |
| `{DB91C6B1FC2158A8}Prefabs/Characters/Uniforms/Jacket_Pilot_HEER_01/Jacket_Pilot_HEER_01.et` | Pilot flying jacket |  |
| `{7844FAD04826C7EE}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01.et` | Panzer black wrap jacket (Panzerjacke) |  |
| `{F639D6C29B03C5B1}Prefabs/Characters/Uniforms/Pants_Pilot_HEER_01/Pants_Pilot_HEER_01.et` | Pilot trousers |  |
| `{5AC77EE3E5EF3277}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01.et` | Panzer black trousers |  |
| `{10548CE06620830A}Prefabs/Characters/Uniforms/Smock_M38/Jacket_HEER_M38_Olive.et` | M38 Fallschirmjaeger smock, olive - INCOMPLETE (see notes) |  |
| `{B68B08E8EC896865}Prefabs/Characters/Uniforms/Smock_M38/Jacket_HEER_M38_Splinter.et` | M38 FJ smock, Splinter camo - INCOMPLETE (see notes) |  |
| `{BA4CDA33D60F0A3F}Prefabs/Characters/Uniforms/Trousers_M36/Pants_HEER_M36_Feldgrau.et` | M36 trousers feldgrau |  |
| `{3559BE99EAEA4BA1}Prefabs/Characters/Uniforms/Trousers_M36/Pants_HEER_M36_Luftwaffe.et` | M36 trousers Luftwaffe | Luftwaffe eagle |
| `{7033D8D854B77A37}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_Feldgrau.et` | M40 trousers feldgrau |  |
| `{7123BD938A95A2DB}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_Luftwaffe.et` | M40 trousers Luftwaffe | Luftwaffe eagle |
| `{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et` | M43 trousers feldgrau |  |
| `{ACB191B3129C68FA}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Luftwaffe.et` | M43 trousers Luftwaffe | Luftwaffe eagle |

### GER - Vests / webbing

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{DA6B7A838193CAE1}Prefabs/Characters/Vests/M1939/Variants/M1939_AssaultRifleman.et` | StG44 pouches |  |
| `{860510A769453C29}Prefabs/Characters/Vests/M1939/Variants/M1939_AssistantMachinegunner.et` | AMG |  |
| `{C9AEE94D317C4043}Prefabs/Characters/Vests/M1939/Variants/M1939_AutomaticRifleman.et` | FG42 / auto rifleman |  |
| `{2FFAF6DCD6222EEE}Prefabs/Characters/Vests/M1939/Variants/M1939_AutomaticRifleman_Camo.et` | FG42 auto rifleman, camo bandolier |  |
| `{6711E2FD2135C748}Prefabs/Characters/Vests/M1939/Variants/M1939_Crewman.et` | Crew harness |  |
| `{5AB5D3E8FD72FF07}Prefabs/Characters/Vests/M1939/Variants/M1939_Grenadier.et` | Grenade bag |  |
| `{9BB937575F3354A0}Prefabs/Characters/Vests/M1939/Variants/M1939_MG.et` | MG34/42 ammo strap |  |
| `{1EEC4C28D81CE28A}Prefabs/Characters/Vests/M1939/Variants/M1939_Marksman.et` | G43 pouches |  |
| `{C6F7B4EA03C8A818}Prefabs/Characters/Vests/M1939/Variants/M1939_Medic.et` | Medic bib + pouch |  |
| `{09AC57C880380469}Prefabs/Characters/Vests/M1939/Variants/M1939_NCO.et` | NCO (MP40 pouches) |  |
| `{65A429EB8092B68A}Prefabs/Characters/Vests/M1939/Variants/M1939_Officer.et` | Officer (Luger) |  |
| `{372D108439DAFA30}Prefabs/Characters/Vests/M1939/Variants/M1939_Officer_SMG.et` | Officer w/ SMG pouches |  |
| `{07489DC04EBE67C8}Prefabs/Characters/Vests/M1939/Variants/M1939_Rifleman.et` | Kar98k belt pouches |  |
| `{357B72D5B14BF292}Prefabs/Characters/Vests/M1939/Variants/M1939_SMG.et` | MP40 pouches |  |
| `{7A36A297D2B22AF6}Prefabs/Characters/Vests/Vest_M1030/Vest_Model1030.et` | M1030 vest (unknown, check in-game) |  |

### GER - Footwear

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{D7BC415E8552E5F8}Prefabs/Characters/Footwear/Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et` | Fallschirmjaeger jump boots |  |
| `{E454E94528AE883D}Prefabs/Characters/Footwear/Boots_Fliegerstiefel/Boots_HEER_Fliegerstiefel_01.et` | Flying boots |  |
| `{FD5C132FF8AF2185}Prefabs/Characters/Footwear/JackBoots_M39/CombatBoots_HEER_M39_01.et` | M39 jackboots |  |
| `{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et` | M43 lace-up low boots |  |
| `{FDBAE1C8FE705B97}Prefabs/Characters/Footwear/LowBoots_Panzer/TankBoots_HEER_01.et` | Panzer low boots |  |
| `{217BAD5FF36D5083}Prefabs/Characters/Footwear/Offiziersstiefel_01/Boots_HEER_Offiziersstiefel_01.et` | Officer riding boots |  |

### GER - Handwear

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{8D38A884E46F7B85}Prefabs/Characters/Handwear/Gloves_Pilot_HEER_01/Gloves_Pilot_HEER_01.et` | Pilot gloves |  |

### GER - Webbing pouches / backpacks / radio (FH GER Uniforms)

| Prefab | Description |
|---|---|
| `{2EEEC603359D29C3}Prefabs/Items/Equipment/Backpacks/Backpack_M34.et` | Backpack M34 |
| `{11BFF6F49979A267}Prefabs/Items/Equipment/Backpacks/Backpack_M34_Medic.et` | Backpack M34 Medic |
| `{83BA2C46B3FEF331}Prefabs/Items/Equipment/Backpacks/Backpack_M38_GasMaskCannister.et` | Backpack M38 GasMaskCannister |
| `{03E37E2AFC8BCE83}Prefabs/Items/Equipment/Backpacks/Backpack_M38_GasMaskCannister_Medic.et` | Backpack M38 GasMaskCannister Medic |
| `{9DD5617396560272}Prefabs/Items/Equipment/Backpacks/Backpack_M41.et` | Backpack M41 |
| `{0E6D30BF706E290B}Prefabs/Items/Equipment/Backpacks/Backpack_Packframe_AssistantMachinegunner.et` | Backpack Packframe AssistantMachinegunner |
| `{CCD1BDDB37A07BB2}Prefabs/Items/Equipment/Backpacks/Backpack_Packframe_Panzerschreck.et` | Backpack Packframe Panzerschreck |
| `{8FE50B3CD5F26667}Prefabs/Items/Equipment/Headset/Headset_HEER/Headset_HEER_01.et` | Headset HEER 01 |
| `{379324894618F158}Prefabs/Items/Pouches/M1939/A_Frame.et` | A Frame |
| `{44DBB9BD69811F16}Prefabs/Items/Pouches/M1939/Breadbag.et` | Breadbag |
| `{997E0BC2B10F98EC}Prefabs/Items/Pouches/M1939/FG42_Bandolier.et` | FG42 Bandolier |
| `{3D4B33A2A58D50E7}Prefabs/Items/Pouches/M1939/FG42_Bandolier_Camo.et` | FG42 Bandolier Camo |
| `{4BCB530577ACB9D0}Prefabs/Items/Pouches/M1939/G43_Belt_Pouch.et` | G43 Belt Pouch |
| `{2FCF66C7E5714B7B}Prefabs/Items/Pouches/M1939/German_Bayonet.et` | German Bayonet |
| `{B3A2421DB7D2DE9F}Prefabs/Items/Pouches/M1939/German_Canteen.et` | German Canteen |
| `{646C39E2BB456FB0}Prefabs/Items/Pouches/M1939/German_ETool.et` | German ETool |
| `{4153E2126AE8CA44}Prefabs/Items/Pouches/M1939/German_MGPouch.et` | German MGPouch |
| `{6AECC9F5D4326C05}Prefabs/Items/Pouches/M1939/German_Mapbag.et` | German Mapbag |
| `{B3B739938660E2A8}Prefabs/Items/Pouches/M1939/German_MedicPouch.et` | German MedicPouch |
| `{DF903D96CF721C44}Prefabs/Items/Pouches/M1939/German_Messkit.et` | German Messkit |
| `{73C42BB52515AE6D}Prefabs/Items/Pouches/M1939/Grenadier_Bag.et` | Grenadier Bag |
| `{74764888B2CD82D1}Prefabs/Items/Pouches/M1939/Luger_Holster.et` | Luger Holster |
| `{656B69C3334DB02B}Prefabs/Items/Pouches/M1939/MG34_AmmoStrap.et` | MG34 AmmoStrap |
| `{E1EE4EADADE31C06}Prefabs/Items/Pouches/M1939/MP40_Pouches.et` | MP40 Pouches |
| `{E7BBC6C58C5D9187}Prefabs/Items/Pouches/M1939/Meidcal_Bib.et` | Meidcal Bib |
| `{1759135325645AB5}Prefabs/Items/Pouches/M1939/STG_Pouches.et` | STG Pouches |
| `{8977B306AF351625}Prefabs/Items/Pouches/M1939/Schiessbecher_Holster.et` | Schiessbecher Holster |
| `{6764140E525FAE6F}Prefabs/Items/Pouches/M1939/Sleep_Pad.et` | Sleep Pad |
| `{33F3CC501E19B35E}Prefabs/Items/Pouches/M1939/Suspenders.et` | Suspenders |
| `{E11D62CA25A3B8C0}Prefabs/Items/Pouches/M1939/Suspenders_A_Frame.et` | Suspenders A Frame |

## 2. United States (BLUFOR / US Army)

### US - Headgear (helmets, covers, caps)

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{5AA5F82DF6924487}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01.et` | M1 garrison (overseas) cap OD |  |
| `{B3273ADD157E2FB2}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01_Officer.et` | Garrison cap, officer |  |
| `{315D6B514A3B01E1}Prefabs/Characters/HeadGear/Hat_ServiceCap_01/Hat_ServiceCap_01_Captain.et` | Service peaked cap, captain |  |
| `{C919759CBEBFEE3D}Prefabs/Characters/HeadGear/Helmet_A11_01/Helmet_A11_01.et` | A-11 pilot helmet |  |
| `{A5A8AA43CE3F7D24}Prefabs/Characters/HeadGear/Helmet_M1938_01/Helmet_M1938_Tanker_01.et` | M1938 tanker (fiberglass) helmet |  |
| `{50058D39B81A5590}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01.et` | M1 steel helmet, plain |  |
| `{AC96B59D4F15AEC6}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Medic.et` | M1 helmet, medic markings |  |
| `{89EB785CDE795008}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_NCO.et` | M1 helmet, NCO stripe (SL helmet; referenced as "SquadLeader" by FH US SL) |  |
| `{15405974502B16A4}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting.et` | M1 helmet with garnish net |  |
| `{0C6685EF6A9A4382}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting_Goggles.et` | M1 helmet with net + goggles |  |
| `{5364BB5C94391CB5}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_PlatoonLeader.et` | M1 helmet, officer bar |  |
| `{9883084BCFCEB903}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Scrim.et` | M1 helmet with scrim camo cover |  |

### US - Uniforms (jackets, trousers, suits)

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{66E83C69885ADF71}Prefabs/Characters/Uniforms/M37_Jacket/Jacket_USA_M37.et` | M37 wool field jacket / blouse (OD wool) |  |
| `{C15897A19782B5A5}Prefabs/Characters/Uniforms/M37_Jacket/Jacket_USA_M37_Officer.et` | M37 blouse officer |  |
| `{30FF9AF93D543E4B}Prefabs/Characters/Uniforms/M37_Trousers/Pants_USA_M37.et` | M37 wool trousers |  |
| `{5257D583335C6DED}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41.et` | M41 field jacket (OD) |  |
| `{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et` | M41 HBT trousers |  |
| `{441FBCB19F80AB3D}Prefabs/Characters/Uniforms/M42_Jacket/Jacket_USA_M42.et` | M42 paratrooper jump jacket (reinforced) |  |
| `{9DD4E9E9CBF92F4B}Prefabs/Characters/Uniforms/M42_Trousers/Pants_USA_M42.et` | M42 paratrooper trousers (cargo pockets) |  |
| `{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et` | M43 field jacket (late war, 4 pockets) |  |
| `{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et` | M43 HBT trousers |  |
| `{4C4C6BA177B108C2}Prefabs/Characters/Uniforms/Suit_Pilot_USA_01/Suit_Pilot_USA_WW2_01.et` | ANS31 pilot coveralls |  |
| `{1522AC27CAAFFC90}Prefabs/Characters/Uniforms/Suit_Tanker_US_01/Suit_Tanker_HBT_USA_01.et` | HBT tanker coveralls |  |
| `{E765AF7B18C6FD88}Prefabs/Characters/Uniforms/Tanker_Jacket/Jacket_USA_Tanker.et` | Tanker winter jacket (tanker jacket) |  |

### US - Vests / webbing

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{D0145C7C4FFDD972}Prefabs/Characters/Vests/Vest_ALICE/M1936_Pistol_Belt_Dummy.et` |  |  |
| `{B880E46E5B8274BE}Prefabs/Characters/Vests/Vest_M1923/Variants/Vest_M1923_Assembled_Rifleman.et` | M1923 belt, rifleman |  |
| `{E6784908E8AAFF41}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_AssistantMachinegunner_Airborne.et` | M1936 belt/suspenders, AMG (Airborne: musette bag, jump-style) |  |
| `{F92D179AC7D87AF1}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_AutomaticRifleman_Airborne.et` | M1936 belt/suspenders, BAR (Airborne: musette bag, jump-style) |  |
| `{5D61D40F22F0A11D}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Carbine_Airborne.et` | M1936 belt/suspenders, carbine (Airborne: musette bag, jump-style) |  |
| `{823A4966A222ACA5}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Machinegunner_Airborne.et` | M1936 belt/suspenders, MG (Airborne: musette bag, jump-style) |  |
| `{90DCA29685E6922F}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Medic_Airborne.et` | M1936 belt/suspenders, medic (Airborne: musette bag, jump-style) |  |
| `{FDF82EC7ED08EAFF}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_NCO_Airborne.et` | M1936 belt/suspenders, NCO (Airborne: musette bag, jump-style) |  |
| `{6FB2E0AF138F9551}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Officer_Carbine_Airborne.et` | M1936 belt/suspenders, officer carbine (Airborne: musette bag, jump-style) |  |
| `{943F749525AC3C3C}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Officer_SMG_Airborne.et` | M1936 belt/suspenders, officer SMG (Airborne: musette bag, jump-style) |  |
| `{6518CF49D3CAA3B5}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_RifleGrenadier_Airborne.et` | M1936 belt/suspenders, rifle grenadier (Airborne: musette bag, jump-style) |  |
| `{B6EA5F16BCDA1A7E}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Rifleman_Airborne.et` | M1936 belt/suspenders, rifleman (Airborne: musette bag, jump-style) |  |
| `{3C9461BAFF2F4415}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_SMG_Airborne.et` | M1936 belt/suspenders, SMG (Airborne: musette bag, jump-style) |  |
| `{30C248D35F8009BA}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_WeaponCrew_Airborne.et` | M1936 belt/suspenders, weapon crew (Airborne: musette bag, jump-style) |  |
| `{CAB061361A861B31}Prefabs/Characters/Vests/Vest_M1936/M1936_Suspenders.et` | M1936 suspenders |  |
| `{F53DD82A72B21C44}Prefabs/Characters/Vests/Vest_M1936/M1936_Suspenders_Medic.et` | M1936 suspenders medic |  |
| `{2F8CC0551CA5FDD2}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_AssistantMachinegunner.et` | M1936 belt/suspenders, AMG |  |
| `{B1E2EDDB5CBFB294}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_AutomaticRifleman.et` | M1936 belt/suspenders, BAR |  |
| `{ABEE1A07FDC407EE}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Carbine.et` | M1936 belt/suspenders, carbine |  |
| `{D15A8D0B95AF65A6}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Machinegunner.et` | M1936 belt/suspenders, MG |  |
| `{556BFE2077F40EDB}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Medic.et` | M1936 belt/suspenders, medic |  |
| `{DB5ED4EAF58211E7}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_NCO.et` | M1936 belt/suspenders, NCO |  |
| `{805957A3BA1EF663}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Officer.et` | M1936 belt/suspenders, officer |  |
| `{9F0857F80B75F15B}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Officer_Carbine.et` | M1936 belt/suspenders, officer carbine |  |
| `{EC47EB0214B18D75}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Officer_SMG.et` | M1936 belt/suspenders, officer SMG |  |
| `{D0982A9E82AEBE31}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_RifleGrenadier.et` | M1936 belt/suspenders, rifle grenadier |  |
| `{CC2A24E573D91381}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Rifleman.et` | M1936 belt/suspenders, rifleman |  |
| `{E789F1F7C4F1E71C}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_SMG.et` | M1936 belt/suspenders, SMG |  |
| `{8619FA7DC7274A5D}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_VehicleCrew.et` | M1936 belt/suspenders, vehicle crew |  |
| `{886264AF037A0E80}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_WeaponCrew.et` | M1936 belt/suspenders, weapon crew |  |
| `{40914C92A022A4FF}Prefabs/Characters/Vests/Vest_M1936/Vest_M1936_Assembled_Test.et` | M1936 belt/suspenders, Test |  |
| `{8A4A6301DEA24523}Prefabs/Characters/Vests/Vest_M2/Vest_M2_Ammo.et` | M2 ammo vest (ammo bearer) |  |
| `{E9A138F01AD22F99}Prefabs/Characters/Vests/Vest_MaeB4/Vest_MaeB4.et` | Mae West B-4 life vest |  |

### US - Footwear

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{C4C115CE1BBF2864}Prefabs/Characters/Footwear/JumpBoots_USA_M1942.et` | M1942 paratrooper jump boots |  |
| `{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et` | M1939 service boots |  |
| `{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et` | M1943 combat boots (buckle) |  |

### US - Handwear

| Prefab (GUID + path) | Description | Flags |
|---|---|---|
| `{7633E68EF4C16FC6}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_B3_01.et` | B3 pilot gloves |  |

## 3. Existing character prefabs (outfit actually worn)

Resolved through the inheritance chain (BaseLoadout -> role). Counts: 
GER 132, US 68 character prefabs (incl. base/loadout prefabs; plus 2 vanilla FIA + 2 vanilla USSR not WW2).

Team-type mapping used in the tiers: GER Infantry = `HEER_Infantry` + `HEER_Panzergrenadiers` (+ Late), GER Recon = `HEER_Aufklarers`, GER Para = none exists (see gaps), GER Armor crew = `PanzerCrew/PanzerCommander/PanzerOfficer` inside Panzergrenadiers, Luftwaffe = `Luftwaffe`. US Infantry = `USA_Infantry`, US Recon = `USA_CavRecon`, US Armor = `USA_ArmoredInfantry` (all stubs), US Para = none (empty stub).

### US - US_Army (68)

| Character | GUID | Hat | Jacket | Vest |
|---|---|---|---|---|
| USAWW2_Base | `C0D4443FC2CD5DA2` | - | - | - |
| USAWW2_BaseLoadout | `8BF1D8EEEDF8DA42` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_BaseLoadout_Late | `4665398674FD76AA` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_BaseLoadout_Paratrooper | `FB3F64B425AD80AD` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| Character_HEER_Base | `2056FFBCAF2A2261` | - | - | - |
| Character_US_AA_Stinger | `8C2580374687A2D8` | - | - | - |
| Character_US_AT_Javelin | `32B7698A556F7D54` | - | - | - |
| USAWW2_ArmoredInfantry_AAR_Mid | `99D685FEB020E5CF` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_AAT_Mid | `1836C7B5DC19E141` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_AMG_Mid | `0C16F4B30C6351AE` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_AMG_Tripod_Mid | `097764A28FB0CA58` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_AT_Mid | `1BBBDCD6F5108402` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_AmmoBearer_Mid | `81992CFFFD7F0B11` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_ArtilleryCrewLeader_Mid | `2E0D9EBECEF674CE` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_ArtilleryCrewman_Mid | `B97B3112DFBD607F` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Builder_Mid | `E0F0CF96653E6253` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Driver_Mid | `4760BE6073A7B69F` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_GL_Mid | `CD93596BFBFC3742` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_MG_Mid | `0F9BEFD0256A34ED` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Medic_Mid | `08226A202708754E` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Officer_Mid | `F2B306AA5A27FD02` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_PL_Mid | `5CF63CB3C832CE3A` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_RTO_Mid | `D881E54DC3D4E880` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Rifleman_Carbine_Mid | `546CD869820DC333` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Rifleman_Garand_Mid | `E72D32D58A2342B3` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_SL_Mid | `76D21B103EDCA815` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_SMG_Mid | `F9DA5642AE546846` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_SR_Mid | `755293A1B1D5D385` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Sniper_Mid | `C5B1FDAB6A2626C9` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_ArmoredInfantry_Unarmed_Mid | `DAC459B846710039` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_AAT_Mid | `28BB7A4AFAF26BF2` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_AMG_Mid | `16775713008380FF` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_AT_Mid | `3694252803A1DB99` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_Carbine_Mid | `5D4AE46B93DED1C2` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_GL_Mid | `B7D13D4C22F682A1` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_Garand_Mid | `31B0F04F2D4737F1` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_Leader_Mid | `C1D8EFB083436683` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_MG_Mid | `943B36E2A057BE10` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_Medic_Mid | `10F531D621B3BDBC` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_RTO_Mid | `F6DBF57A071C8DBB` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_SMG_GreaseGun_Mid | `1AE7C35A820F2679` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_SMG_Thompson_Mid | `5BBF30C715E3349B` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_SR_Mid | `E6545484D263FEEA` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_Sniper_Mid | `23D2CE154C25F7A7` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_CavRecon_Spotter_Mid | `3790D53BB073082C` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_AAR_Mid | `280B3686A9DD90EC` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_AAT_Mid | `0F5552BBBF73C14F` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_AMG_Mid | `ADFA41711C85A4C6` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_AMG_Tripod_Mid | `3F70EECD71E625A3` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_AT_Mid | `53AE7774B6885DC3` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_AmmoBearer_Mid | `179DE1B2027A2790` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_ArtilleryCrewLeader_Mid | `16DBF7F43DD7F06D` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_ArtilleryCrewman_Mid | `E6CC0ADB3B89D9C4` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_Builder_Mid | `C93138F6731FC3CC` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_Driver_Mid | `C7CB58BC68D294BE` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_GL_Mid | `D2EB6F1097DF04FB` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_MG_Mid | `F10164BE157E384A` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_Medic_Mid | `81644C727A8443CE` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_Officer_Mid | `F41A3C2C2A5C7F3C` | Hat_ServiceCap_01_Captain | Jacket_USA_M41 | Vest_M1936_Assembled_Officer |
| USAWW2_Infantry_PL_Mid | `947E1839B24C4A99` | Helmet_M1_ww2_PlatoonLeader | Jacket_USA_M41 | Vest_M1936_Assembled_Officer_Carbine |
| USAWW2_Infantry_RTO_Mid | `24F432ACD5E41826` | Helmet_M1_ww2_01 | Jacket_USA_M41 | Vest_M1936_Assembled_Carbine |
| USAWW2_Infantry_Rifleman_Carbine_Mid | `417D4E5F13F4AE14` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |
| USAWW2_Infantry_Rifleman_Garand_Mid | `77BA10429E70E72E` | Helmet_M1_ww2_01 | Jacket_USA_M41 | Vest_M1936_Assembled_Rifleman |
| USAWW2_Infantry_SL_Mid | `9A605C718FBB4543` | Helmet_M1_ww2_SquadLeader | Jacket_USA_M41 | Vest_M1936_Assembled_NCO |
| USAWW2_Infantry_SMG_Mid | `EF4AACBDE214E0D9` | Helmet_M1_ww2_01 | Jacket_USA_M41 | Vest_M1936_Assembled_SMG |
| USAWW2_Infantry_SR_Mid | `2047A8B8C3AC526C` | Helmet_M1_ww2_Netting | Jacket_USA_M41 | Vest_M1936_Assembled_Rifleman |
| USAWW2_Infantry_Sniper_Mid | `6098169A3FE24216` | Helmet_M1_ww2_Scrim | Jacket_USA_M41 | Vest_M1936_Assembled_Rifleman |
| USAWW2_Infantry_Unarmed_Mid | `58F205E9DEC060CC` | Hat_GarrisonCap_01 | Jacket_USA_M41 | - |

### GER - HEER (63)

| Character | GUID | Hat | Jacket | Vest |
|---|---|---|---|---|
| HEERWW2_Base | `5AEB39143B0E4517` | - | - | - |
| HEERWW2_BaseLoadout | `22F63F996693A1E5` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | - |
| HEERWW2_BaseLoadoutLuftwaffe | `91C9A7E664E14FC0` | Hat_M40_01_Luftwaffe | Jacket_HEER_M40_Luftwaffe | - |
| HEERWW2_BaseLoadoutLuftwaffe_Late | `9EC28B6A55FE4933` | Hat_M43_01_Luftwaffe | Jacket_HEER_M40_Luftwaffe | - |
| HEERWW2_BaseLoadout_Late | `A3D9A021BC04B51F` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | - |
| HEERWW2_Scout_AMG_Aufklarer_Mid | `3F64C5821ACA14B9` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Scout_G43_Aufklarer_Mid | `A85D2FF4D6237549` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Marksman |
| HEERWW2_Scout_GL_Aufklarer_Mid | `B762312BA7CD8756` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Grenadier |
| HEERWW2_Scout_KAR98_Aufklarer_Mid | `E1CDE94B99E3DD31` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Scout_LAT_Aufklarer_Mid | `8CFCE82745C010B8` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Scout_Leader_Aufklarer_Mid | `2C43CCA068DF65E5` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_NCO |
| HEERWW2_Scout_MG_Aufklarer_Mid | `B359506AED159A7C` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_MG |
| HEERWW2_Scout_Medic_Aufklarer_Mid | `042F1D81F9E6D665` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Medic |
| HEERWW2_Scout_RTO_Aufklarer_Mid | `3710D5EEE46A96D3` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_Scout_SMG_Aufklarer_Mid | `6B82AEE60A5D59C8` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_Sniper_Aufklarer_Mid | `7D2F704752B6F18E` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Spotter_Aufklarer_Mid | `4E574F75F3865A7C` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_AMG_Infantry_Mid | `E3A6F6617ED0EA98` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_AssistantMachinegunner |
| HEERWW2_AMG_Tripod_Infantry_Mid | `3D246DD51CAFA3B0` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_AssistantMachinegunner |
| HEERWW2_AmmoBearer_Infantry_Mid | `72EC139F327664E6` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_ArtilleryCrewLeader_Infantry_Mid | `F70706C4813BE556` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_ArtilleryCrew_Infantry_Mid | `1090D113B917ABBA` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Driver_Infantry_Mid | `9FD3CE4233DA50AF` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_GL_Infantry_Mid | `877ACF3C5693FD66` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Grenadier |
| HEERWW2_LAT_Infantry_Mid | `E0271DC242FA4AD1` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_MG_Infantry_Mid | `F528E83380CC3BB9` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_MG |
| HEERWW2_Medic_Infantry_Mid | `A60E064DBB8FB9FB` | Helmet_Stahlhelm_M35_Medic | Jacket_HEER_M40_Feldgrau | M1939_Medic |
| HEERWW2_Officer_Infantry_Mid | `9D9A4580D0EA3632` | Hat_Schirmmutze_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Officer |
| HEERWW2_PL_Infantry_Mid | `60D5105CFAD83E70` | Hat_Schirmmutze_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Officer_SMG |
| HEERWW2_Pioner_Infantry_Mid | `8A223160E78C7E65` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_RTO_Infantry_Mid | `05D05615C2D75926` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_Rifleman_G43_Infantry_Mid | `B3DE0F3E4008DB38` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Marksman |
| HEERWW2_Rifleman_KAR98_Infantry_Mid | `23C2C71CBBC84D26` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_SL_Infantry_Mid | `CFBDDCF9F635A9DC` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_NCO |
| HEERWW2_SMG_Infantry_Mid | `F0C68EDE9D52D387` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_SR_Infantry_Mid | `B96DE84D5643848C` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Sharpshooter_KAR98_Infantry_Mid | `E562D65BA7E6C18D` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_Unarmed_Infantry_Mid | `44441191EBE21196` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | - |
| HEERWW2_AMG_Panzergrenadier_Mid | `E3A6F6617ED0EA99` | Helmet_Stahlhelm_M35_Painted | Jacket_HEER_M40_Feldgrau | M1939_AssistantMachinegunner |
| HEERWW2_AMG_Tripod_Panzergrenadier_Mid | `3D246DD51CAFA3B1` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_AssistantMachinegunner |
| HEERWW2_AmmoBearer_Panzergrenadier_Mid | `72EC139F327664E7` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_GL_Panzergrenadier_Mid | `877ACF3C5693FD67` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_Grenadier |
| HEERWW2_HalftrackDriver_Panzergrenadier_Mid | `3542FDE33970FEF2` | Helmet_Stahlhelm_M35_Painted | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_LAT_Panzergrenadier_Mid | `E0271DC242FA4AD2` | Helmet_Stahlhelm_M35_Painted | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_MG_Panzergrenadier_Mid | `F528E83380CC3BBA` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_MG |
| HEERWW2_Medic_Panzergrenadier_Mid | `A60E064DBB8FB9FC` | Helmet_Stahlhelm_M35_Medic | Jacket_HEER_M40_Feldgrau | M1939_Medic |
| HEERWW2_PL_Panzergrenadier_Mid | `60D5105CFAD83E71` | Hat_Schirmmutze_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_Officer_SMG |
| HEERWW2_PanzerCommander | `35BE1C03BA709199` | Hat_M40_01_TankCrew | Jacket_Tanker_HEER_01 | M1939_Officer_SMG |
| HEERWW2_PanzerCrew | `434962A4349B2EBF` | Hat_M40_01_TankCrew | Jacket_Tanker_HEER_01 | M1939_Crewman |
| HEERWW2_PanzerOfficer | `C7A017811C5D46E3` | Hat_Schirmmutze_01_Panzer | Jacket_Tanker_HEER_01 | M1939_Officer |
| HEERWW2_Pioner_Panzergrenadier_Mid | `175850A5D4A7CD4D` | Helmet_Stahlhelm_M35_Painted | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_RTO_Panzergrenadier_Mid | `05D05615C2D75927` | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_Rifleman_G43_Panzergrenadier_Mid | `B3DE0F3E4008DB39` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_Marksman |
| HEERWW2_Rifleman_KAR98_Panzergrenadier_Mid | `23C2C71CBBC84D27` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_SL_Panzergrenadier_Mid | `CFBDDCF9F635A9DD` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_NCO |
| HEERWW2_SMG_Panzergrenadier_Mid | `F0C68EDE9D52D388` | Helmet_Stahlhelm_M35_Netting | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_SR_Panzergrenadier_Mid | `B96DE84D5643848D` | Helmet_Stahlhelm_M35_Painted_Netting | Jacket_HEER_M40_Feldgrau | M1939_SMG |
| HEERWW2_Sharpshooter_KAR98_Panzergrenadier_Mid | `E562D65BA7E6C18E` | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_TruckDriver_Panzergrenadier_Mid | `9FD3CE4233DA50B0` | Helmet_Stahlhelm_M35_Painted | Jacket_HEER_M40_Feldgrau | M1939_Rifleman |
| HEERWW2_LW_AAACrewLeader_Mid | `AB01BECE9BF82EC8` | Helmet_Stahlhelm_M35_Blue | Jacket_HEER_M40_Luftwaffe | M1939_Rifleman |
| HEERWW2_LW_AAACrew_Mid | `281564B63D08DAD8` | Helmet_Stahlhelm_M35_Blue | Jacket_HEER_M40_Luftwaffe | M1939_Rifleman |
| HEERWW2_Luftwaffe_Unarmed | `6BA7969B29E35FA9` | Hat_M40_01_Luftwaffe | Jacket_HEER_M40_Luftwaffe | - |
| HEERWW2_Pilot_Luftwaffe_Mid | `989FA3B458311243` | Hat_M40_01_Luftwaffe_Officer | Jacket_HEER_M43_Luftwaffe | M1939_Officer |

### GER - HEER/Late (69)

| Character | GUID | Hat | Jacket | Vest |
|---|---|---|---|---|
| HEERWW2_Scout_AAT_Aufklarer_Late | `F6174EF177BE2E3D` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_Scout_AMG_Aufklarer_Late | `0D3611342009A368` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Scout_AT_Aufklarer_Late | `B235C0C50A581708` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Crewman |
| HEERWW2_Scout_G43_Aufklarer_Late | `40C12E423B0057DC` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Marksman |
| HEERWW2_Scout_GL_Aufklarer_Late | `F628D66F9C3097BF` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Grenadier |
| HEERWW2_Scout_KAR98_Aufklarer_Late | `D1731D2619EB9925` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Scout_LAT_Aufklarer_Late | `4BCC9034F097B1DD` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Scout_Leader_Aufklarer_Late | `1266E019AE2F0C5C` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_NCO |
| HEERWW2_Scout_MG_Aufklarer_Late | `51AC917BCB6C8A87` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_MG |
| HEERWW2_Scout_Medic_Aufklarer_Late | `F0DC4ECAB98B8F4B` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Medic |
| HEERWW2_Scout_RTO_Aufklarer_Late | `57C1D630CCB1C2AD` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_Scout_SMG_Aufklarer_Late | `CC62F414C8EF6232` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_Scout_STG44_Aufklarer_Late | `2FAF05342B3D097C` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Sharpshooter_Aufklarer_Late | `B43300BEC83A0866` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Sniper_Aufklarer_Late | `733C854BE3D1C358` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Spotter_Aufklarer_Late | `775BE76163BF18FB` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AAT_Infantry_Late | `54BA0DD50116231E` | Helmet_Stahlhelm_M42_Net | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AMG_Infantry_Late | `743BD8914221F94B` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AMG_Tripod_Infantry_Late | `41B1D0BDEB52B52F` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AT_Infantry_Late | `F6414E8CDD64E035` | Helmet_Stahlhelm_M42_Net | Jacket_HEER_M43_Feldgrau | M1939_Crewman |
| HEERWW2_AmmoBearer_Infantry_Late | `2509BD47688E34D1` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_ArtilleryCrewLeader_Infantry_Late | `86A688C267EF0B7E` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_ArtilleryCrew_Infantry_Late | `7FD60DA909400E28` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Driver_Infantry_Late | `FFE78E448CE7ED37` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_GL_Infantry_Late | `25D046975F1E0064` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Grenadier |
| HEERWW2_LAT_Infantry_Late | `3D767AA5AF2846C2` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_MG_Infantry_Late | `D39E173D592F3223` | Helmet_Stahlhelm_M42_Net | Jacket_HEER_M43_Feldgrau | M1939_MG |
| HEERWW2_Medic_Infantry_Late | `D99D706854F57520` | Helmet_Stahlhelm_M42_Medic | Jacket_HEER_M43_Feldgrau | M1939_Medic |
| HEERWW2_Officer_Infantry_Late | `CD86324ACD037638` | Hat_Schirmmutze_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Officer |
| HEERWW2_PL_Infantry_Late | `976F9E5BFBABF9AC` | Hat_M43_01_Feldgrau_Officer | Jacket_HEER_M43_Feldgrau | M1939_Officer_SMG |
| HEERWW2_Pioner_Infantry_Late | `99FD7C3B816783F3` | Helmet_Stahlhelm_M42_Net | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_RTO_Infantry_Late | `73BD8EC5572B257E` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_Rifleman_G43_Infantry_Late | `5D44A49196700A8D` | Helmet_Stahlhelm_M42_Painted | Jacket_HEER_M43_Feldgrau | M1939_Marksman |
| HEERWW2_Rifleman_KAR98_Infantry_Late | `3801E0D7B3EFC905` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_SL_Infantry_Late | `CC7A9A2B95BAF0F1` | Helmet_Stahlhelm_M42_Net | Jacket_HEER_M43_Feldgrau | M1939_NCO |
| HEERWW2_SMG_Infantry_Late | `B50131FC180FDA65` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_SR_Infantry_Late | `7B9402C13BF99F2F` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Marksman |
| HEERWW2_Sharpshooter_G43_Infantry_Late | `7562779B73C34333` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Marksman |
| HEERWW2_Sharpshooter_KAR98_Infantry_Late | `9193B2F3BFC9F31D` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Unarmed_Infantry_Late | `4D9AE67340188688` | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau | - |
| HEERWW2_AAT_Panzergrenadier_Late | `C41E80DE55F8ABB8` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AMG_Panzergrenadier_Late | `8F0F56CC715FAA22` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AMG_Tripod_Panzergrenadier_Late | `D6F7E9F2AEE8DB6C` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AT_Panzergrenadier_Late | `75A6D0F357EF5348` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Crewman |
| HEERWW2_AmmoBearer_Panzergrenadier_Late | `D8C8293A75BAC297` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_AssaultRifleman_Panzergrenadier_Late | `2E2F9390F511BEA3` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_AssaultRifleman |
| HEERWW2_GL_Panzergrenadier_Late | `993341F524144FEA` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Grenadier |
| HEERWW2_HalftrackDriver_Panzergrenadier_Late | `696D5A4622A8D2C9` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_LAT_Panzergrenadier_Late | `C9F5D7CCA1C1B897` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_LAT_STG44_Panzergrenadier_Late | `06DC155EC12FB9D0` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_AssaultRifleman |
| HEERWW2_MG_Panzergrenadier_Late | `3EB706E1734852D2` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_MG |
| HEERWW2_Medic_Panzergrenadier_Late | `306545F66AC82C04` | Helmet_Stahlhelm_M42_Medic | Jacket_HEER_M43_Feldgrau | M1939_Medic |
| HEERWW2_PL_Panzergrenadier_Late | `584B730A2D36B0BC` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Officer_SMG |
| HEERWW2_PanzerCommander_Late | `FFAB28E8CF03B9E5` | Hat_M43_01_TankCommander | Jacket_Tanker_HEER_01 | M1939_Officer_SMG |
| HEERWW2_PanzerCrew_Late | `7A862398A7385FDA` | Hat_M43_01_TankCrew | Jacket_Tanker_HEER_01 | M1939_Crewman |
| HEERWW2_Pioner_Panzergrenadier_Late | `A36D2BD2627BCE8B` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_RTO_Panzergrenadier_Late | `D5F891C89DE7CBE7` | Hat_M43_01_Splinter | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_Rifleman_G43_Panzergrenadier_Late | `0C2A7F202AB75735` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Marksman |
| HEERWW2_Rifleman_KAR98_Panzergrenadier_Late | `765E8F12D2F7E079` | Helmet_Stahlhelm_M42_Painted | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_SL_Panzergrenadier_Late | `16434ACA74A90D7B` | Helmet_Stahlhelm_M42_Painted | Jacket_HEER_M43_Feldgrau | M1939_NCO |
| HEERWW2_SL_STG_Panzergrenadier_Late | `C550597E34B19442` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_AssaultRifleman |
| HEERWW2_SMG_Panzergrenadier_Late | `4E5BB3EC99B96B78` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_SR_Panzergrenadier_Late | `7AE7D0ED045C739C` | Helmet_Stahlhelm_M42_Painted | Jacket_HEER_M43_Feldgrau | M1939_SMG |
| HEERWW2_Sharpshooter_G43_Panzergrenadier_Late | `44AA39FB3BF3C381` | Helmet_Stahlhelm_M42_Painted_Net | Jacket_HEER_M43_Feldgrau | M1939_Marksman |
| HEERWW2_Sharpshooter_KAR98_Panzergrenadier_Late | `EDD22AFF2460FD25` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_Sharpshooter_STG44_Panzergrenadier_Late | `3EC4CB97EDD36563` | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau | M1939_AssaultRifleman |
| HEERWW2_TruckDriver_Panzergrenadier_Late | `3F6CF657031C5B28` | Helmet_Stahlhelm_M42_Feldgrau | Jacket_HEER_M43_Feldgrau | M1939_Rifleman |
| HEERWW2_LW_AAACrewLeader_Late | `E77352FDCC6E04DF` | Helmet_Stahlhelm_M35_Blue | Jacket_HEER_M40_Luftwaffe | M1939_Rifleman |
| HEERWW2_LW_AAACrew_Late | `137DBFF97035AB9F` | Helmet_Stahlhelm_M42_Blue | Jacket_HEER_M40_Luftwaffe | M1939_Rifleman |

## 4. AI appearance tiers by bot skill

Design rules: (1) the silhouette read at distance goes **plain -> helmeted -> netted -> camo-covered**, so a player sees the tougher bot from its head and torso; (2) only prefabs that exist (all entries are validated against ALL_GUIDS); (3) no Luftwaffe and no SS-like items; (4) the tier is a **clothing override** applied on top of an existing role character (rifleman/SMG/MG...), i.e. the tier controls Hat/Jacket/Pants/Boots/Vest slots, the role keeps its weapons. For roles needing a different vest (medic = `M1939_Medic` / `Vest_M1936_Assembled_Medic`, MG = `M1939_MG`, officer...), keep the role character vest and apply only Hat/Jacket/Pants/Boots.

Tier keys (suggested skill bands): Recruit = skill <0.3, Regular = 0.3-0.55, Veteran = 0.55-0.8, Elite = >0.8.

### GER tiers

| Tier | Team | Hat | Jacket / Pants | Boots | Vest |
|---|---|---|---|---|---|
| Recruit | Infantry | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | CombatBoots_HEER_M43_01 | M1939_Rifleman |
| Recruit | Recon | Hat_M40_01_Feldgrau | Jacket_HEER_M40_Feldgrau / Pants_HEER_M40_Feldgrau | CombatBoots_HEER_M43_01 | M1939_Rifleman |
| Recruit | Para | Helmet_Stahlhelm_M38_01 | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | Boots_HEER_JumpBoots_01 | M1939_Rifleman |
| Recruit | Armor crew | Hat_M40_01_TankCrew | Jacket_Tanker_HEER_01 / Pants_Tanker_HEER_01 | TankBoots_HEER_01 | M1939_Crewman |
| Regular | Infantry | Helmet_Stahlhelm_M35_01 | Jacket_HEER_M40_Feldgrau / Pants_HEER_M40_Feldgrau | CombatBoots_HEER_M39_01 | M1939_Rifleman |
| Regular | Recon | Hat_M43_01_Feldgrau | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | CombatBoots_HEER_M43_01 | M1939_Marksman |
| Regular | Para | Helmet_Stahlhelm_M38_Painted | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | Boots_HEER_JumpBoots_01 | M1939_AssaultRifleman |
| Regular | Armor crew | Hat_M43_01_TankCrew | Jacket_Tanker_HEER_01 / Pants_Tanker_HEER_01 | TankBoots_HEER_01 | M1939_Crewman |
| Veteran | Infantry | Helmet_Stahlhelm_M42_Net | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | CombatBoots_HEER_M39_01 | M1939_AssaultRifleman |
| Veteran | Recon | Hat_M43_01_Splinter | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | CombatBoots_HEER_M43_01 | M1939_Marksman |
| Veteran | Para | Helmet_Stahlhelm_M38_Green_Cover | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | Boots_HEER_JumpBoots_01 | M1939_AssaultRifleman |
| Veteran | Armor crew | Hat_Schirmmutze_01_Panzer | Jacket_Tanker_HEER_01 / Pants_Tanker_HEER_01 | TankBoots_HEER_01 | M1939_Crewman |
| Elite | Infantry | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | CombatBoots_HEER_M39_01 | M1939_AssaultRifleman |
| Elite | Recon | Helmet_Stahlhelm_M42_Cover_Splinter | Jacket_HEER_M43_Feldgrau / Pants_HEER_M43_Feldgrau | CombatBoots_HEER_M43_01 | M1939_AutomaticRifleman_Camo |
| Elite | Para | Helmet_Stahlhelm_M38_Splinter_Cover | Jacket_HEER_M38_Splinter / Pants_HEER_M43_Feldgrau | Boots_HEER_JumpBoots_01 | M1939_AutomaticRifleman_Camo |
| Elite | Armor crew | Hat_M43_01_TankCommander | Jacket_Tanker_HEER_01 / Pants_Tanker_HEER_01 | TankBoots_HEER_01 | M1939_Officer_SMG |

### US tiers

| Tier | Team | Hat | Jacket / Pants | Boots | Vest |
|---|---|---|---|---|---|
| Recruit | Infantry | Hat_GarrisonCap_01 | Jacket_USA_M41 / Pants_USA_M41_HBT | ServiceBoots_USA_M1939 | Vest_M1936_Assembled_Rifleman |
| Recruit | Recon | Hat_GarrisonCap_01 | Jacket_USA_M41 / Pants_USA_M41_HBT | ServiceBoots_USA_M1939 | Vest_M1936_Assembled_Carbine |
| Recruit | Para | Helmet_M1_ww2_01 | Jacket_USA_M41 / Pants_USA_M41_HBT | JumpBoots_USA_M1942 | Vest_M1936_Assembled_Rifleman_Airborne |
| Recruit | Armor crew | Helmet_M1938_Tanker_01 | Suit_Tanker_HBT_USA_01 | ServiceBoots_USA_M1939 | - |
| Regular | Infantry | Helmet_M1_ww2_01 | Jacket_USA_M41 / Pants_USA_M41_HBT | ServiceBoots_USA_M1939 | Vest_M1936_Assembled_Rifleman |
| Regular | Recon | Helmet_M1_ww2_01 | Jacket_USA_M43 / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_Carbine |
| Regular | Para | Helmet_M1_ww2_01 | Jacket_USA_M42 / Pants_USA_M42 | JumpBoots_USA_M1942 | Vest_M1936_Assembled_Rifleman_Airborne |
| Regular | Armor crew | Helmet_M1938_Tanker_01 | Jacket_USA_Tanker / Pants_USA_M41_HBT | ServiceBoots_USA_M1939 | Vest_M1936_Assembled_VehicleCrew |
| Veteran | Infantry | Helmet_M1_ww2_Netting | Jacket_USA_M43 / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_AutomaticRifleman |
| Veteran | Recon | Helmet_M1_ww2_Netting | Jacket_USA_M43 / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_SMG |
| Veteran | Para | Helmet_M1_ww2_Netting | Jacket_USA_M42 / Pants_USA_M42 | JumpBoots_USA_M1942 | Vest_M1936_Assembled_AutomaticRifleman_Airborne |
| Veteran | Armor crew | Helmet_M1938_Tanker_01 | Jacket_USA_Tanker / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_VehicleCrew |
| Elite | Infantry | Helmet_M1_ww2_Scrim | Jacket_USA_M43 / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_NCO |
| Elite | Recon | Helmet_M1_ww2_Netting_Goggles | Jacket_USA_M43 / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_SMG |
| Elite | Para | Helmet_M1_ww2_Scrim | Jacket_USA_M42 / Pants_USA_M42 | JumpBoots_USA_M1942 | Vest_M1936_Assembled_NCO_Airborne |
| Elite | Armor crew | Hat_ServiceCap_01_Captain | Jacket_USA_Tanker / Pants_USA_M43_HBT | ServiceBoots_USA_M1943 | Vest_M1936_Assembled_Officer_SMG |

Recognition cheat-sheet:

* **GER Infantry**: Recruit = field cap only, no helmet; Regular = plain M35 stahlhelm; Veteran = M42 with garnish net; Elite = M42 + Splinter cover. **Recon**: M40 cap -> M43 cap -> Splinter cap -> Splinter-covered helmet. **Para** (Fallschirmjaeger look): M38 helmet plain -> painted -> green cover -> Splinter cover + Splinter smock (Elite, smock flagged WIP) with jump boots. **Armor crew**: black Panzer cap M40 -> M43 -> peaked Schirmmuetze -> commander cap with officer SMG vest.
* **US Infantry**: Recruit = garrison cap, no helmet; Regular = plain M1; Veteran = M1 + net and the later M43 jacket/HBT; Elite = M1 scrim + NCO vest. **Recon**: garrison cap -> M1 helmet -> netted -> net+goggles. **Para**: M42 jump jacket/trousers + jump boots + Airborne vest family (M1 plain -> net -> scrim). **Armor crew**: M1938 tanker helmet + coveralls -> tanker jacket -> service cap captain commander.

### Notes / caveats

* GER Para Elite uses `Jacket_HEER_M38_Splinter` (FJ smock): base prefab has no worn model (vanilla BDU material only). If it renders wrong in-game, replace with `Jacket_HEER_M43_Feldgrau` (the Splinter cover helmet + jump boots already give the Elite read). Alternative: Recon-style `Hat_M43_01_Splinter`.
* GER Recruit Infantry / Recon wear no helmet (cap only): visually weakest, matches the brief. If bots must keep head protection, use `Helmet_Stahlhelm_M35_01` for Recruit and shift others up one step.
* Officer / Schirmmuetze caps appear only in the Armor-crew ladder (as the Panzer cap) to avoid enemy officers being confused with elite infantry.
* US vest families: `Vest_M1936_Assembled_*` (ground) vs `*_Airborne` (musette bag, para). Only Para uses Airborne so players can identify it. `Vest_M1936_Assembled_Officer_SMG` is used for the Elite Armor commander.
* No US prefab for "Ranger" exists; Ranger look = M42 para jacket + scrim helmet. Currently the Elite Infantry wears the plain M43 jacket + Scrim helmet to keep the M42 look for Para; swap to M42 if you want a Ranger-looking Elite (all prefabs exist).
* Team types mapped to catalog classes: Infantry = INF, Recon = REC, Para = PARA, Armor crew = CREW.

## 5. JSON snippet for catalog.json

Each tier/team array is ordered **Hat, Jacket, Pants, Boots, Vest** (US ArmorCrew Recruit omits pants/vest: single coverall). Full copy in `inventory/aiAppearance.json`; merge as `catalog["aiAppearance"]`.

```json
{
 "aiAppearance": {
  "GER": {
   "Recruit": {
    "Infantry": [
     "{A34221AB5A70F92C}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Feldgrau.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et",
     "{07489DC04EBE67C8}Prefabs/Characters/Vests/M1939/Variants/M1939_Rifleman.et"
    ],
    "Recon": [
     "{C3C3B64455FC06C6}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Feldgrau.et",
     "{B7E538F48ED5B12F}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_Feldgrau.et",
     "{7033D8D854B77A37}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_Feldgrau.et",
     "{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et",
     "{07489DC04EBE67C8}Prefabs/Characters/Vests/M1939/Variants/M1939_Rifleman.et"
    ],
    "Para": [
     "{D62A72E894FA605B}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_01.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{D7BC415E8552E5F8}Prefabs/Characters/Footwear/Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et",
     "{07489DC04EBE67C8}Prefabs/Characters/Vests/M1939/Variants/M1939_Rifleman.et"
    ],
    "ArmorCrew": [
     "{79770150D9286630}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_TankCrew.et",
     "{7844FAD04826C7EE}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01.et",
     "{5AC77EE3E5EF3277}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01.et",
     "{FDBAE1C8FE705B97}Prefabs/Characters/Footwear/LowBoots_Panzer/TankBoots_HEER_01.et",
     "{6711E2FD2135C748}Prefabs/Characters/Vests/M1939/Variants/M1939_Crewman.et"
    ]
   },
   "Regular": {
    "Infantry": [
     "{EFEEC9D16CB32D5F}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_01.et",
     "{B7E538F48ED5B12F}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_Feldgrau.et",
     "{7033D8D854B77A37}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_Feldgrau.et",
     "{FD5C132FF8AF2185}Prefabs/Characters/Footwear/JackBoots_M39/CombatBoots_HEER_M39_01.et",
     "{07489DC04EBE67C8}Prefabs/Characters/Vests/M1939/Variants/M1939_Rifleman.et"
    ],
    "Recon": [
     "{A34221AB5A70F92C}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Feldgrau.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et",
     "{1EEC4C28D81CE28A}Prefabs/Characters/Vests/M1939/Variants/M1939_Marksman.et"
    ],
    "Para": [
     "{C56B9D549F8B495A}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Painted.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{D7BC415E8552E5F8}Prefabs/Characters/Footwear/Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et",
     "{DA6B7A838193CAE1}Prefabs/Characters/Vests/M1939/Variants/M1939_AssaultRifleman.et"
    ],
    "ArmorCrew": [
     "{868355C2B87B0AB2}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_TankCrew.et",
     "{7844FAD04826C7EE}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01.et",
     "{5AC77EE3E5EF3277}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01.et",
     "{FDBAE1C8FE705B97}Prefabs/Characters/Footwear/LowBoots_Panzer/TankBoots_HEER_01.et",
     "{6711E2FD2135C748}Prefabs/Characters/Vests/M1939/Variants/M1939_Crewman.et"
    ]
   },
   "Veteran": {
    "Infantry": [
     "{19463B1006FA1EF4}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Net.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{FD5C132FF8AF2185}Prefabs/Characters/Footwear/JackBoots_M39/CombatBoots_HEER_M39_01.et",
     "{DA6B7A838193CAE1}Prefabs/Characters/Vests/M1939/Variants/M1939_AssaultRifleman.et"
    ],
    "Recon": [
     "{F6A9BD9EA6F00C13}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Splinter.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et",
     "{1EEC4C28D81CE28A}Prefabs/Characters/Vests/M1939/Variants/M1939_Marksman.et"
    ],
    "Para": [
     "{0EFFE36965141049}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Green_Cover.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{D7BC415E8552E5F8}Prefabs/Characters/Footwear/Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et",
     "{DA6B7A838193CAE1}Prefabs/Characters/Vests/M1939/Variants/M1939_AssaultRifleman.et"
    ],
    "ArmorCrew": [
     "{47ECF4DCE5F7DB8F}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Panzer.et",
     "{7844FAD04826C7EE}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01.et",
     "{5AC77EE3E5EF3277}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01.et",
     "{FDBAE1C8FE705B97}Prefabs/Characters/Footwear/LowBoots_Panzer/TankBoots_HEER_01.et",
     "{6711E2FD2135C748}Prefabs/Characters/Vests/M1939/Variants/M1939_Crewman.et"
    ]
   },
   "Elite": {
    "Infantry": [
     "{307C507641AE33C7}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Cover_Splinter.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{FD5C132FF8AF2185}Prefabs/Characters/Footwear/JackBoots_M39/CombatBoots_HEER_M39_01.et",
     "{DA6B7A838193CAE1}Prefabs/Characters/Vests/M1939/Variants/M1939_AssaultRifleman.et"
    ],
    "Recon": [
     "{307C507641AE33C7}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Cover_Splinter.et",
     "{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et",
     "{2FFAF6DCD6222EEE}Prefabs/Characters/Vests/M1939/Variants/M1939_AutomaticRifleman_Camo.et"
    ],
    "Para": [
     "{4E1FC79E3E28A143}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Splinter_Cover.et",
     "{B68B08E8EC896865}Prefabs/Characters/Uniforms/Smock_M38/Jacket_HEER_M38_Splinter.et",
     "{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et",
     "{D7BC415E8552E5F8}Prefabs/Characters/Footwear/Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et",
     "{2FFAF6DCD6222EEE}Prefabs/Characters/Vests/M1939/Variants/M1939_AutomaticRifleman_Camo.et"
    ],
    "ArmorCrew": [
     "{D35C610DE45084FA}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_TankCommander.et",
     "{7844FAD04826C7EE}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01.et",
     "{5AC77EE3E5EF3277}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01.et",
     "{FDBAE1C8FE705B97}Prefabs/Characters/Footwear/LowBoots_Panzer/TankBoots_HEER_01.et",
     "{372D108439DAFA30}Prefabs/Characters/Vests/M1939/Variants/M1939_Officer_SMG.et"
    ]
   }
  },
  "US": {
   "Recruit": {
    "Infantry": [
     "{5AA5F82DF6924487}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01.et",
     "{5257D583335C6DED}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41.et",
     "{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et",
     "{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et",
     "{CC2A24E573D91381}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Rifleman.et"
    ],
    "Recon": [
     "{5AA5F82DF6924487}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01.et",
     "{5257D583335C6DED}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41.et",
     "{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et",
     "{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et",
     "{ABEE1A07FDC407EE}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Carbine.et"
    ],
    "Para": [
     "{50058D39B81A5590}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01.et",
     "{5257D583335C6DED}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41.et",
     "{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et",
     "{C4C115CE1BBF2864}Prefabs/Characters/Footwear/JumpBoots_USA_M1942.et",
     "{B6EA5F16BCDA1A7E}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Rifleman_Airborne.et"
    ],
    "ArmorCrew": [
     "{A5A8AA43CE3F7D24}Prefabs/Characters/HeadGear/Helmet_M1938_01/Helmet_M1938_Tanker_01.et",
     "{1522AC27CAAFFC90}Prefabs/Characters/Uniforms/Suit_Tanker_US_01/Suit_Tanker_HBT_USA_01.et",
     "{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et"
    ]
   },
   "Regular": {
    "Infantry": [
     "{50058D39B81A5590}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01.et",
     "{5257D583335C6DED}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41.et",
     "{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et",
     "{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et",
     "{CC2A24E573D91381}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Rifleman.et"
    ],
    "Recon": [
     "{50058D39B81A5590}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01.et",
     "{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{ABEE1A07FDC407EE}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Carbine.et"
    ],
    "Para": [
     "{50058D39B81A5590}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01.et",
     "{441FBCB19F80AB3D}Prefabs/Characters/Uniforms/M42_Jacket/Jacket_USA_M42.et",
     "{9DD4E9E9CBF92F4B}Prefabs/Characters/Uniforms/M42_Trousers/Pants_USA_M42.et",
     "{C4C115CE1BBF2864}Prefabs/Characters/Footwear/JumpBoots_USA_M1942.et",
     "{B6EA5F16BCDA1A7E}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_Rifleman_Airborne.et"
    ],
    "ArmorCrew": [
     "{A5A8AA43CE3F7D24}Prefabs/Characters/HeadGear/Helmet_M1938_01/Helmet_M1938_Tanker_01.et",
     "{E765AF7B18C6FD88}Prefabs/Characters/Uniforms/Tanker_Jacket/Jacket_USA_Tanker.et",
     "{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et",
     "{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et",
     "{8619FA7DC7274A5D}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_VehicleCrew.et"
    ]
   },
   "Veteran": {
    "Infantry": [
     "{15405974502B16A4}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting.et",
     "{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{B1E2EDDB5CBFB294}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_AutomaticRifleman.et"
    ],
    "Recon": [
     "{15405974502B16A4}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting.et",
     "{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{E789F1F7C4F1E71C}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_SMG.et"
    ],
    "Para": [
     "{15405974502B16A4}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting.et",
     "{441FBCB19F80AB3D}Prefabs/Characters/Uniforms/M42_Jacket/Jacket_USA_M42.et",
     "{9DD4E9E9CBF92F4B}Prefabs/Characters/Uniforms/M42_Trousers/Pants_USA_M42.et",
     "{C4C115CE1BBF2864}Prefabs/Characters/Footwear/JumpBoots_USA_M1942.et",
     "{F92D179AC7D87AF1}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_AutomaticRifleman_Airborne.et"
    ],
    "ArmorCrew": [
     "{A5A8AA43CE3F7D24}Prefabs/Characters/HeadGear/Helmet_M1938_01/Helmet_M1938_Tanker_01.et",
     "{E765AF7B18C6FD88}Prefabs/Characters/Uniforms/Tanker_Jacket/Jacket_USA_Tanker.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{8619FA7DC7274A5D}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_VehicleCrew.et"
    ]
   },
   "Elite": {
    "Infantry": [
     "{9883084BCFCEB903}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Scrim.et",
     "{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{DB5ED4EAF58211E7}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_NCO.et"
    ],
    "Recon": [
     "{0C6685EF6A9A4382}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting_Goggles.et",
     "{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{E789F1F7C4F1E71C}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_SMG.et"
    ],
    "Para": [
     "{9883084BCFCEB903}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Scrim.et",
     "{441FBCB19F80AB3D}Prefabs/Characters/Uniforms/M42_Jacket/Jacket_USA_M42.et",
     "{9DD4E9E9CBF92F4B}Prefabs/Characters/Uniforms/M42_Trousers/Pants_USA_M42.et",
     "{C4C115CE1BBF2864}Prefabs/Characters/Footwear/JumpBoots_USA_M1942.et",
     "{FDF82EC7ED08EAFF}Prefabs/Characters/Vests/Vest_M1936/Airborne/Vest_M1936_Assembled_NCO_Airborne.et"
    ],
    "ArmorCrew": [
     "{315D6B514A3B01E1}Prefabs/Characters/HeadGear/Hat_ServiceCap_01/Hat_ServiceCap_01_Captain.et",
     "{E765AF7B18C6FD88}Prefabs/Characters/Uniforms/Tanker_Jacket/Jacket_USA_Tanker.et",
     "{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et",
     "{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et",
     "{EC47EB0214B18D75}Prefabs/Characters/Vests/Vest_M1936/Variants/Vest_M1936_Assembled_Officer_SMG.et"
    ]
   }
  }
 }
}
```
