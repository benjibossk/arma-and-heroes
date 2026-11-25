# INVENTAIRE ASSETS WW2 - Front 44

Genere le 2026-11-25 depuis les .pak de `D:\Serveur\arma-reforger-ww2\profile\addons` (lecture seule). Listings et textes extraits dans `inventory/extract/` (`<mod>.listing.txt` = tout le pak, `<mod>.guids.txt` = `{GUID}chemin` des .et/.conf lus dans resourceDatabase.rdb, `ALL_GUIDS.txt` = fusion). Les resource names ci-dessous sont complets et utilisables tels quels.

## Mods

| Mod | GUID addon | Version | Pak (Mo) |
|---|---|---|---|
| CIE Tracked Core (`CIETrackedCore`) | `68CD35053063C6D5` | 0.0.122 | 46 |
| CIE_Thermal (`CIE_Thermal`) | `68F8A813D7EF1E30` | 1.0.75 | 2 |
| Chungus Bolt Guns (`ChungusBoltGuns`) | `61BD6595183FCEBD` | 1.0.68 | 88 |
| Chungus Core (`ChungusCore`) | `66D6340C1D565C98` | 1.0.14 | 0 |
| Finest Hour - Core Mod (`FinestHour-CoreMod`) | `69392E57A68724BD` | 1.0.16 | 829 |
| Finest Hour - GER Gear (`FinestHour-GERGear`) | `693931E2091E57D1` | 1.0.14 | 407 |
| Finest Hour - GER Uniforms (`FinestHour-GERUniforms`) | `693931B9076BE89D` | 1.0.9 | 569 |
| Finest Hour - GER Units (`FinestHour-GERUnits`) | `693B24A2038949EA` | 1.0.4 | 24 |
| Finest Hour - GER Vehicles (`FinestHour-GERVehicles`) | `693AE5B800A316FE` | 1.0.11 | 502 |
| Finest Hour - US Gear (`FinestHour-USGear`) | `6939316200A674C4` | 1.0.15 | 562 |
| Finest Hour - US Uniforms (`FinestHour-USUniforms`) | `693931291AD52242` | 1.0.12 | 527 |
| Finest Hour - US Units (`FinestHour-USUnits`) | `693B23F6421096C4` | 1.0.2 | 0 |
| M4A2 Sherman (`M4A2Sherman`) | `672EFC7887D6AEA7` | 1.0.9 | 61 |
| Northcom German Voice Pack (`NorthcomGermanVoicePack`) | `66CC0D8D7EF6A8FF` | 1.0.6 | 30 |
| WCS_Armaments (`WCS_Armaments`) | `629B2BA37EFFD577` | 9.0.3 | 422 |
| WCS_SpaceCore (`WCS_SpaceCore`) | `5E389BB9F58B79A6` | 9.0.1 | 24 |
| WW2 Tank Core (`WW2TankCore`) | `696372CF8EB6C647` | 1.0.6 | 19 |

> Alerte : **Finest Hour - US Units 1.0.2 est largement un squelette** (pak 39 Ko) : seules 8 classes Infantry ont un equipement (Officer, PL, Rifleman Garand, RTO, SL, SMG, SR, Sniper). MG/AT/Medic/Crew/etc. US = coquilles vides heritant de BaseLoadout. `Group_FH_USAWW2_RifleSquad` reference `Character_FH_USAWW2_SL/Medic/Rifleman.et` qui n'existent dans aucun mod installe. Le catalogue `Characters_EntityCatalog_USA_WW2.conf` est vide. Idem Vehicles catalogs USA/HEER (listes de categories vides).

## 1. Factions

| FactionKey | Fichier | Remarque |
|---|---|---|
| `FH_USAWW2` | `{9B7E80D183825E85}Configs/Factions/USA_WW2.conf` | herite de BLUFOR.conf ; amis : US, CIV |
| `FH_HEERWW2` | `{CE39CF0CCC42D01D}Configs/Factions/HEER_WW2.conf` | herite de OPFOR.conf |
| `FH_USSRWW2` | `{93674D5488FC1B18}Configs/Factions/USSR_WW2.conf` | NE PAS UTILISER : catalogues = unites USSR vanilla modernes (Spetsnaz, UAZ, BTR-70, Mi-8), pas WW2 |

Catalogues d'entites (CoreMod) : `{3B41B4CA4334CB46}Configs/EntityCatalog/HEER/Characters_EntityCatalog_HEER.conf`, `{CCE06E74A622D605}Configs/EntityCatalog/HEER/Groups_EntityCatalog_HEER.conf`, `{47A5E969D76BE7B4}Configs/EntityCatalog/HEER/Vehicles_EntityCatalog_HEER.conf`, `{11186AB878674529}Configs/EntityCatalog/USA_WW2/Characters_EntityCatalog_USA_WW2.conf`, `{BE6FEB2EDAEEB0A2}Configs/EntityCatalog/USA_WW2/Groups_EntityCatalog_USA_WW2.conf`, `{FF73D510EAA2FF85}Configs/EntityCatalog/USA_WW2/Vehicles_EntityCatalog_USA_WW2.conf`

Autres configs : Callsigns_HEER, FactionHomeTerritory_HEER, FieldManual categories (CoreMod/GERVehicles).

## 2. Personnages / loadouts jouables

### 2.1 Finest Hour - US Units (`FH_USAWW2`, BLUFOR)

Mid-war uniquement (casque M1, Garand). Hierarchie : Base -> BaseLoadout (+_Late, _Paratrooper = coquilles) -> classes `USA_Infantry` / `USA_ArmoredInfantry` / `USA_CavRecon` (suffixe `_Mid`). STUB = prefab vide (149 octets, aucun equipement).

| Prefab | Arme(s) | Casque / Gilet | Etat |
|---|---|---|---|
| `{C0D4443FC2CD5DA2}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_FH_USAWW2_Base.et` | - | - / - | OK |
| `{8BF1D8EEEDF8DA42}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_FH_USAWW2_BaseLoadout.et` | - | Hat_GarrisonCap_01.et / - | OK |
| `{4665398674FD76AA}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_FH_USAWW2_BaseLoadout_Late.et` | - | - / - | OK |
| `{FB3F64B425AD80AD}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_FH_USAWW2_BaseLoadout_Paratrooper.et` | - | - / - | STUB vide |
| `{99D685FEB020E5CF}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_AAR_Mid.et` | - | - / - | STUB vide |
| `{1836C7B5DC19E141}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_AAT_Mid.et` | - | - / - | STUB vide |
| `{0C16F4B30C6351AE}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_AMG_Mid.et` | - | - / - | STUB vide |
| `{097764A28FB0CA58}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_AMG_Tripod_Mid.et` | - | - / - | STUB vide |
| `{1BBBDCD6F5108402}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_AT_Mid.et` | - | - / - | STUB vide |
| `{81992CFFFD7F0B11}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_AmmoBearer_Mid.et` | - | - / - | STUB vide |
| `{2E0D9EBECEF674CE}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_ArtilleryCrewLeader_Mid.et` | - | - / - | STUB vide |
| `{B97B3112DFBD607F}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_ArtilleryCrewman_Mid.et` | - | - / - | STUB vide |
| `{E0F0CF96653E6253}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Builder_Mid.et` | - | - / - | STUB vide |
| `{4760BE6073A7B69F}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Driver_Mid.et` | - | - / - | STUB vide |
| `{CD93596BFBFC3742}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_GL_Mid.et` | - | - / - | STUB vide |
| `{0F9BEFD0256A34ED}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_MG_Mid.et` | - | - / - | STUB vide |
| `{08226A202708754E}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Medic_Mid.et` | - | - / - | STUB vide |
| `{F2B306AA5A27FD02}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Officer_Mid.et` | - | - / - | STUB vide |
| `{5CF63CB3C832CE3A}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_PL_Mid.et` | - | - / - | STUB vide |
| `{D881E54DC3D4E880}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_RTO_Mid.et` | - | - / - | STUB vide |
| `{546CD869820DC333}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Rifleman_Carbine_Mid.et` | - | - / - | STUB vide |
| `{E72D32D58A2342B3}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Rifleman_Garand_Mid.et` | - | - / - | STUB vide |
| `{76D21B103EDCA815}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_SL_Mid.et` | - | - / - | STUB vide |
| `{F9DA5642AE546846}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_SMG_Mid.et` | - | - / - | STUB vide |
| `{755293A1B1D5D385}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_SR_Mid.et` | - | - / - | STUB vide |
| `{C5B1FDAB6A2626C9}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Sniper_Mid.et` | - | - / - | STUB vide |
| `{DAC459B846710039}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_ArmoredInfantry/Character_FH_USAWW2_ArmoredInfantry_Unarmed_Mid.et` | - | - / - | STUB vide |
| `{28BB7A4AFAF26BF2}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_AAT_Mid.et` | - | - / - | STUB vide |
| `{16775713008380FF}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_AMG_Mid.et` | - | - / - | STUB vide |
| `{3694252803A1DB99}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_AT_Mid.et` | - | - / - | STUB vide |
| `{5D4AE46B93DED1C2}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_Carbine_Mid.et` | - | - / - | STUB vide |
| `{B7D13D4C22F682A1}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_GL_Mid.et` | - | - / - | STUB vide |
| `{31B0F04F2D4737F1}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_Garand_Mid.et` | - | - / - | STUB vide |
| `{C1D8EFB083436683}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_Leader_Mid.et` | - | - / - | STUB vide |
| `{943B36E2A057BE10}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_MG_Mid.et` | - | - / - | STUB vide |
| `{10F531D621B3BDBC}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_Medic_Mid.et` | - | - / - | STUB vide |
| `{F6DBF57A071C8DBB}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_RTO_Mid.et` | - | - / - | STUB vide |
| `{1AE7C35A820F2679}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_SMG_GreaseGun_Mid.et` | - | - / - | STUB vide |
| `{5BBF30C715E3349B}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_SMG_Thompson_Mid.et` | - | - / - | STUB vide |
| `{E6545484D263FEEA}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_SR_Mid.et` | - | - / - | STUB vide |
| `{23D2CE154C25F7A7}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_Sniper_Mid.et` | - | - / - | STUB vide |
| `{3790D53BB073082C}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_CavRecon/Character_FH_USAWW2_CavRecon_Spotter_Mid.et` | - | - / - | STUB vide |
| `{280B3686A9DD90EC}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_AAR_Mid.et` | - | - / - | STUB vide |
| `{0F5552BBBF73C14F}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_AAT_Mid.et` | - | - / - | STUB vide |
| `{ADFA41711C85A4C6}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_AMG_Mid.et` | - | - / - | STUB vide |
| `{3F70EECD71E625A3}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_AMG_Tripod_Mid.et` | - | - / - | STUB vide |
| `{53AE7774B6885DC3}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_AT_Mid.et` | - | - / - | STUB vide |
| `{179DE1B2027A2790}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_AmmoBearer_Mid.et` | - | - / - | STUB vide |
| `{16DBF7F43DD7F06D}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_ArtilleryCrewLeader_Mid.et` | - | - / - | STUB vide |
| `{E6CC0ADB3B89D9C4}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_ArtilleryCrewman_Mid.et` | - | - / - | STUB vide |
| `{C93138F6731FC3CC}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Builder_Mid.et` | - | - / - | STUB vide |
| `{C7CB58BC68D294BE}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Driver_Mid.et` | - | - / - | STUB vide |
| `{D2EB6F1097DF04FB}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_GL_Mid.et` | - | - / - | STUB vide |
| `{F10164BE157E384A}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_MG_Mid.et` | - | - / - | STUB vide |
| `{81644C727A8443CE}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Medic_Mid.et` | - | - / - | STUB vide |
| `{F41A3C2C2A5C7F3C}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Officer_Mid.et` | Handgun_M1911A1.et | Hat_ServiceCap_01_Captain.et / Vest_M1936_Assembled_Officer.et | OK |
| `{947E1839B24C4A99}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_PL_Mid.et` | Rifle_M1_Carbine.et, Handgun_ANM8.et | Helmet_M1_ww2_PlatoonLeader.et / Vest_M1936_Assembled_Officer_Carbine.et | OK |
| `{24F432ACD5E41826}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_RTO_Mid.et` | Rifle_M1_Carbine.et | Helmet_M1_ww2_01.et / Vest_M1936_Assembled_Carbine.et | OK |
| `{417D4E5F13F4AE14}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Rifleman_Carbine_Mid.et` | - | - / - | STUB vide |
| `{77BA10429E70E72E}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Rifleman_Garand_Mid.et` | Rifle_M1.et | Helmet_M1_ww2_01.et / Vest_M1936_Assembled_Rifleman.et | OK |
| `{9A605C718FBB4543}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_SL_Mid.et` | - | Helmet_M1_ww2_SquadLeader.et / Vest_M1936_Assembled_NCO.et | OK |
| `{EF4AACBDE214E0D9}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_SMG_Mid.et` | Submachinegun_M1A1.et | Helmet_M1_ww2_01.et / Vest_M1936_Assembled_SMG.et | OK |
| `{2047A8B8C3AC526C}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_SR_Mid.et` | Rifle_M1.et | Helmet_M1_ww2_Netting.et / Vest_M1936_Assembled_Rifleman.et | OK |
| `{6098169A3FE24216}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Sniper_Mid.et` | Rifle_M1903A4.et | Helmet_M1_ww2_Scrim.et / Vest_M1936_Assembled_Rifleman.et | OK |
| `{58F205E9DEC060CC}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Unarmed_Mid.et` | - | - / - | STUB vide |

Groupes : `{766D107569871A6A}Prefabs/Groups/BLUFOR/Group_FH_USAWW2_Base.et`, `{94B2FC86A63AC704}Prefabs/Groups/BLUFOR/Group_FH_USAWW2_RifleSquad.et` (RifleSquad casse : references manquantes).

### 2.2 Finest Hour - GER Units (`FH_HEERWW2`, OPFOR)

Mid et Late (Late = 1944-45 : Stahlhelm M42, Kar98k/G43/MP40/MG42, Panzerfaust/Panzerschreck). Sous-familles : HEER_Infantry, HEER_Panzergrenadiers, HEER_Aufklarers, Luftwaffe. Tableau = versions Late + PanzerCrew + Luftwaffe.

| Prefab | Arme(s) | Casque / Gilet |
|---|---|---|
| `{5AEB39143B0E4517}Prefabs/Characters/Factions/OPFOR/HEER/Character_FH_HEERWW2_Base.et` | - | - / - |
| `{22F63F996693A1E5}Prefabs/Characters/Factions/OPFOR/HEER/Character_FH_HEERWW2_BaseLoadout.et` | - | Hat_M40_01_Feldgrau.et / - |
| `{91C9A7E664E14FC0}Prefabs/Characters/Factions/OPFOR/HEER/Character_FH_HEERWW2_BaseLoadoutLuftwaffe.et` | - | Hat_M40_01_Luftwaffe.et / - |
| `{9EC28B6A55FE4933}Prefabs/Characters/Factions/OPFOR/HEER/Character_FH_HEERWW2_BaseLoadoutLuftwaffe_Late.et` | - | Hat_M43_01_Luftwaffe.et / - |
| `{A3D9A021BC04B51F}Prefabs/Characters/Factions/OPFOR/HEER/Character_FH_HEERWW2_BaseLoadout_Late.et` | - | Hat_M43_01_Feldgrau.et / - |
| `{F6174EF177BE2E3D}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_AAT_Aufklarer_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_Kar98K.et, Launcher_Panzerfaust100.et | Hat_M43_01_Feldgrau.et / M1939_SMG.et |
| `{0D3611342009A368}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_AMG_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{B235C0C50A581708}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_AT_Aufklarer_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Launcher_RPzB54_Green.et, Handgun_P38.et | Hat_M43_01_Feldgrau.et / M1939_Crewman.et |
| `{40C12E423B0057DC}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_G43_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Gewehr43.et | Hat_M43_01_Feldgrau.et / M1939_Marksman.et |
| `{F628D66F9C3097BF}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_GL_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Hat_M43_01_Feldgrau.et / M1939_Grenadier.et |
| `{D1731D2619EB9925}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_KAR98_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{4BCC9034F097B1DD}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_LAT_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et, Launcher_Panzerfaust100.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{1266E019AE2F0C5C}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_Leader_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Submachinegun_MP38.et, Handgun_Leichtpistole34.et | Hat_M43_01_Feldgrau.et / M1939_NCO.et |
| `{51AC917BCB6C8A87}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_MG_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, MG_MG42.et | Hat_M43_01_Feldgrau.et / M1939_MG.et |
| `{F0DC4ECAB98B8F4B}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_Medic_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et, Handgun_P38.et | Hat_M43_01_Feldgrau.et / M1939_Medic.et |
| `{57C1D630CCB1C2AD}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_RTO_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Submachinegun_MP38.et, Handgun_Leichtpistole34.et | Hat_M43_01_Feldgrau.et / M1939_SMG.et |
| `{CC62F414C8EF6232}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_SMG_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Hat_M43_01_Feldgrau.et / M1939_SMG.et |
| `{2FAF05342B3D097C}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Scout_STG44_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_STG44.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{B43300BEC83A0866}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Sharpshooter_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Gewehr43_Optic.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{733C854BE3D1C358}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Sniper_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K_Sniper.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{775BE76163BF18FB}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Aufklarers/Late/Character_FH_HEERWW2_Spotter_Aufklarer_Late.et` | Grenade_M24.et, Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et, Handgun_Leichtpistole34.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{54BA0DD50116231E}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_AAT_Infantry_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_Kar98K.et, Launcher_Panzerfaust60.et | Helmet_Stahlhelm_M42_Net.et / M1939_Rifleman.et |
| `{743BD8914221F94B}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_AMG_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{41B1D0BDEB52B52F}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_AMG_Tripod_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Handgun_P38.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{F6414E8CDD64E035}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_AT_Infantry_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Launcher_RPzB54_Green.et, Handgun_P38.et | Helmet_Stahlhelm_M42_Net.et / M1939_Crewman.et |
| `{2509BD47688E34D1}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_AmmoBearer_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{86A688C267EF0B7E}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_ArtilleryCrewLeader_Infantry_Late.et` | Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{7FD60DA909400E28}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_ArtilleryCrew_Infantry_Late.et` | Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{FFE78E448CE7ED37}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Driver_Infantry_Late.et` | Rifle_Kar98K.et | Hat_M43_01_Feldgrau.et / M1939_Rifleman.et |
| `{25D046975F1E0064}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_GL_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Grenadier.et |
| `{3D767AA5AF2846C2}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_LAT_Infantry_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_Kar98K.et, Launcher_Panzerfaust100.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Rifleman.et |
| `{D39E173D592F3223}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_MG_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, MG_MG42.et | Helmet_Stahlhelm_M42_Net.et / M1939_MG.et |
| `{D99D706854F57520}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Medic_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Handgun_P38.et | Helmet_Stahlhelm_M42_Medic.et / M1939_Medic.et |
| `{CD86324ACD037638}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Officer_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Handgun_P38.et | Hat_Schirmmutze_01_Feldgrau.et / M1939_Officer.et |
| `{976F9E5BFBABF9AC}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_PL_Infantry_Late.et` | Grenade_M39.et, Submachinegun_MP38.et, Handgun_Leichtpistole34.et | Hat_M43_01_Feldgrau_Officer.et / M1939_Officer_SMG.et |
| `{99FD7C3B816783F3}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Pioner_Infantry_Late.et` | Mine_SMine35.et, Smoke_M42.et, Rifle_Kar98K.et, Mine_Tellermine42AT.et | Helmet_Stahlhelm_M42_Net.et / M1939_Rifleman.et |
| `{73BD8EC5572B257E}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_RTO_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Hat_M43_01_Feldgrau.et / M1939_SMG.et |
| `{5D44A49196700A8D}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Rifleman_G43_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Gewehr43.et | Helmet_Stahlhelm_M42_Painted.et / M1939_Marksman.et |
| `{3801E0D7B3EFC905}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Rifleman_KAR98_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{CC7A9A2B95BAF0F1}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_SL_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Helmet_Stahlhelm_M42_Net.et / M1939_NCO.et |
| `{B50131FC180FDA65}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_SMG_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_SMG.et |
| `{7B9402C13BF99F2F}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_SR_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Marksman.et |
| `{7562779B73C34333}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Sharpshooter_G43_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Gewehr43_Optic.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Marksman.et |
| `{9193B2F3BFC9F31D}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Sharpshooter_KAR98_Infantry_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K_Sniper.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Rifleman.et |
| `{4D9AE67340188688}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Late/Character_FH_HEERWW2_Unarmed_Infantry_Late.et` | - | Hat_M43_01_Feldgrau.et / - |
| `{35BE1C03BA709199}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Character_FH_HEERWW2_PanzerCommander.et` | Submachinegun_MP38.et | Hat_M40_01_TankCrew.et / M1939_Officer_SMG.et |
| `{434962A4349B2EBF}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Character_FH_HEERWW2_PanzerCrew.et` | Handgun_P38.et | Hat_M40_01_TankCrew.et / M1939_Crewman.et |
| `{C7A017811C5D46E3}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Character_FH_HEERWW2_PanzerOfficer.et` | Handgun_P38.et | Hat_Schirmmutze_01_Panzer.et / M1939_Officer.et |
| `{C41E80DE55F8ABB8}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_AAT_Panzergrenadier_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_Kar98K.et, Launcher_Panzerfaust60.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Rifleman.et |
| `{8F0F56CC715FAA22}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_AMG_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Rifleman.et |
| `{D6F7E9F2AEE8DB6C}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_AMG_Tripod_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Handgun_P38.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Rifleman.et |
| `{75A6D0F357EF5348}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_AT_Panzergrenadier_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Launcher_RPzB54_Tan.et, Handgun_P38.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Crewman.et |
| `{D8C8293A75BAC297}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_AmmoBearer_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Rifleman.et |
| `{2E2F9390F511BEA3}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_AssaultRifleman_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_STG44.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_AssaultRifleman.et |
| `{993341F524144FEA}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_GL_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Grenadier.et |
| `{696D5A4622A8D2C9}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_HalftrackDriver_Panzergrenadier_Late.et` | Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_SMG.et |
| `{C9F5D7CCA1C1B897}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_LAT_Panzergrenadier_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_Kar98K.et, Launcher_Panzerfaust100.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Rifleman.et |
| `{06DC155EC12FB9D0}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_LAT_STG44_Panzergrenadier_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_STG44.et, Launcher_Panzerfaust100.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_AssaultRifleman.et |
| `{3EB706E1734852D2}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_MG_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, MG_MG42.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_MG.et |
| `{306545F66AC82C04}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Medic_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Handgun_P38.et | Helmet_Stahlhelm_M42_Medic.et / M1939_Medic.et |
| `{584B730A2D36B0BC}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_PL_Panzergrenadier_Late.et` | Grenade_M39.et, Submachinegun_MP38.et, Handgun_Leichtpistole34.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Officer_SMG.et |
| `{FFAB28E8CF03B9E5}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_PanzerCommander_Late.et` | - | Hat_M43_01_TankCommander.et / - |
| `{7A862398A7385FDA}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_PanzerCrew_Late.et` | - | Hat_M43_01_TankCrew.et / - |
| `{A36D2BD2627BCE8B}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Pioner_Panzergrenadier_Late.et` | Grenade_GeballteLadung.et, Smoke_M42.et, Rifle_Kar98K.et, Mine_Tellermine42AT.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Rifleman.et |
| `{D5F891C89DE7CBE7}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_RTO_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Hat_M43_01_Splinter.et / M1939_SMG.et |
| `{0C2A7F202AB75735}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Rifleman_G43_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Gewehr43.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Marksman.et |
| `{765E8F12D2F7E079}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Rifleman_KAR98_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Painted.et / M1939_Rifleman.et |
| `{16434ACA74A90D7B}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_SL_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Helmet_Stahlhelm_M42_Painted.et / M1939_NCO.et |
| `{C550597E34B19442}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_SL_STG_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_STG44.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_AssaultRifleman.et |
| `{4E5BB3EC99B96B78}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_SMG_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_SMG.et |
| `{7AE7D0ED045C739C}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_SR_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Submachinegun_MP40.et | Helmet_Stahlhelm_M42_Painted.et / M1939_SMG.et |
| `{44AA39FB3BF3C381}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Sharpshooter_G43_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Gewehr43_Optic.et | Helmet_Stahlhelm_M42_Painted_Net.et / M1939_Marksman.et |
| `{EDD22AFF2460FD25}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Sharpshooter_KAR98_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_Kar98K_Sniper.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_Rifleman.et |
| `{3EC4CB97EDD36563}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_Sharpshooter_STG44_Panzergrenadier_Late.et` | Grenade_M39.et, Smoke_M42.et, Rifle_STG44_Optic.et | Helmet_Stahlhelm_M42_Cover_Splinter.et / M1939_AssaultRifleman.et |
| `{3F6CF657031C5B28}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_TruckDriver_Panzergrenadier_Late.et` | Rifle_Kar98K.et | Helmet_Stahlhelm_M42_Feldgrau.et / M1939_Rifleman.et |
| `{AB01BECE9BF82EC8}Prefabs/Characters/Factions/OPFOR/HEER/Luftwaffe/Character_FH_HEERWW2_LW_AAACrewLeader_Mid.et` | - | Helmet_Stahlhelm_M35_Blue.et / M1939_Rifleman.et |
| `{281564B63D08DAD8}Prefabs/Characters/Factions/OPFOR/HEER/Luftwaffe/Character_FH_HEERWW2_LW_AAACrew_Mid.et` | - | Helmet_Stahlhelm_M35_Blue.et / M1939_Rifleman.et |
| `{6BA7969B29E35FA9}Prefabs/Characters/Factions/OPFOR/HEER/Luftwaffe/Character_FH_HEERWW2_Luftwaffe_Unarmed.et` | - | - / - |
| `{989FA3B458311243}Prefabs/Characters/Factions/OPFOR/HEER/Luftwaffe/Character_FH_HEERWW2_Pilot_Luftwaffe_Mid.et` | Handgun_Leichtpistole34.et | Hat_M40_01_Luftwaffe_Officer.et / M1939_Officer.et |
| `{E77352FDCC6E04DF}Prefabs/Characters/Factions/OPFOR/HEER/Luftwaffe/Late/Character_FH_HEERWW2_LW_AAACrewLeader_Late.et` | - | Helmet_Stahlhelm_M35_Blue.et / M1939_Rifleman.et |
| `{137DBFF97035AB9F}Prefabs/Characters/Factions/OPFOR/HEER/Luftwaffe/Late/Character_FH_HEERWW2_LW_AAACrew_Late.et` | - | Helmet_Stahlhelm_M42_Blue.et / M1939_Rifleman.et |

Total 132 prefabs personnages GER (cf. `FinestHour-GERUnits.listing.txt` pour les variantes Mid). Groupes GER (76) : voir `FinestHour-GERUnits.guids.txt` (`Prefabs/Groups/OPFOR/HEER_*`).

## 3. Armes (armes d'epaule, pistolets, MG, lanceurs)

### FinestHour-USGear

| Arme | Resource name | Magasin par defaut (-> calibre) |
|---|---|---|
| Handgun_ANM8 | `{DBD5C4E1C81D8363}Prefabs/Weapons/Handguns/ANM8/Handgun_ANM8.et` | Magazine_37mm_ANM8_1rnd_White |
| Handgun_M1911A1 | `{22E222918CBDA02C}Prefabs/Weapons/Handguns/M1911/Handgun_M1911A1.et` | Magazine_1143x23_M1911_7rnd_Base |
| Launcher_M1A1_Bazooka | `{FBBB0FC51FC85173}Prefabs/Weapons/Launchers/M1A1/Launcher_M1A1_Bazooka.et` | - |
| Machinegun_M1918 | `{C2596CA8BD1C2F08}Prefabs/Weapons/Machineguns/M1918_BAR/Machinegun_M1918.et` | Magazine_762x63_M1918_20rnd_4Ball_1Tracer |
| Deployable_MG_M1919A4_Mounted | `{33A5FBAFD8F61DA2}Prefabs/Weapons/Machineguns/M1919/Deployable_MG_M1919A4_Mounted.et` | - |
| Deployable_MG_M1919A6_Mounted | `{5899CD6E40F71057}Prefabs/Weapons/Machineguns/M1919/Deployable_MG_M1919A6_Mounted.et` | - |
| MG_M1919A4_Dismounted | `{72ABF6428CD8D4D7}Prefabs/Weapons/Machineguns/M1919/MG_M1919A4_Dismounted.et` | Belt_762x63_M1919_100rnd_4Ball_1Tracer |
| MG_M1919A6 | `{DE2075CB6E5CAE3A}Prefabs/Weapons/Machineguns/M1919/MG_M1919A6.et` | Belt_762x63_M1919_100rnd_4Ball_1Tracer |
| MG_M1919A6_Mounted | `{2C45408DA5661BD3}Prefabs/Weapons/Machineguns/M1919/MG_M1919A6_Mounted.et` | Belt_762x63_M1919_100rnd_4Ball_1Tracer |
| Submachinegun_M1A1 | `{E59A4E6BBDE7CFBD}Prefabs/Weapons/Machineguns/M1A1/Submachinegun_M1A1.et` | Magazine_1143x23_M1A1_30rnd_Ball_M1 |
| Submachinegun_M3A1 | `{E14DFCA44B4ADB90}Prefabs/Weapons/Machineguns/M3A1/Submachinegun_M3A1.et` | Magazine_1143x23_M3A1_30rnd_Ball_M1 |
| Rifle_M1903A3 | `{7CA5C7688880A688}Prefabs/Weapons/Rifles/M1903A3/Rifle_M1903A3.et` | Clip_762x63_M1903_5rnd_Base |
| Rifle_M1903A4 | `{4191E250FDEC2FBF}Prefabs/Weapons/Rifles/M1903A3/Rifle_M1903A4.et` | Clip_762x63_M1903_5rnd_Base |
| Rifle_M1A1_Carbine | `{8D7E8742E9F08061}Prefabs/Weapons/Rifles/M1_Carbine/Rifle_M1A1_Carbine.et` | - |
| Rifle_M1_Carbine | `{0C95F90EAD15FC6B}Prefabs/Weapons/Rifles/M1_Carbine/Rifle_M1_Carbine.et` | Magazine_762x33_M1_Carbine_15rnd_Ball_M1 |
| Rifle_M2_Carbine | `{F34FA45A2DE16C2D}Prefabs/Weapons/Rifles/M1_Carbine/Rifle_M2_Carbine.et` | Magazine_762x33_M1_Carbine_15rnd_Ball_M1 |
| Rifle_M1 | `{6FF6BC1AA4BC98BE}Prefabs/Weapons/Rifles/M1_Garand/Rifle_M1.et` | Clip_762x63_M1_8rnd_Ball_M2 |

### FinestHour-GERGear

| Arme | Resource name | Magasin par defaut (-> calibre) |
|---|---|---|
| Handgun_Leichtpistole34 | `{72B77F14D2073B5D}Prefabs/Weapons/Handguns/LP34/Handgun_Leichtpistole34.et` | Magazine_26mm_LP34_1rnd_White |
| Handgun_P38 | `{97C3AA4555300B75}Prefabs/Weapons/Handguns/P38/Handgun_P38.et` | Magazine_9x19_P38_8rnd_Ball_sE |
| Launcher_Panzerfaust100 | `{EADCBEFD2705B515}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust100.et` | - |
| Launcher_Panzerfaust60 | `{15F0473A1454A801}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust60.et` | - |
| Launcher_RPzB43_Green | `{BA0ECE0E5883ECE7}Prefabs/Weapons/Launchers/RPzB_43/Launcher_RPzB43_Green.et` | - |
| Launcher_RPzB43_Tan | `{E6972FA7C8CABC7F}Prefabs/Weapons/Launchers/RPzB_43/Launcher_RPzB43_Tan.et` | - |
| Launcher_RPzB54_Green | `{28B66C43B847AE14}Prefabs/Weapons/Launchers/RPzB_54/Launcher_RPzB54_Green.et` | - |
| Launcher_RPzB54_Tan | `{454330C5B5BF0733}Prefabs/Weapons/Launchers/RPzB_54/Launcher_RPzB54_Tan.et` | - |
| Machinegun_FG42G | `{F6BD5104680D2991}Prefabs/Weapons/Machineguns/FG42G/Machinegun_FG42G.et` | Magazine_792x57_FG42_20rnd_4Ball_1Tracer |
| Machinegun_FG42G_Optic | `{73EFFD5F3CCE88AA}Prefabs/Weapons/Machineguns/FG42G/Machinegun_FG42G_Optic.et` | Magazine_792x57_FG42_20rnd_4Ball_1Tracer |
| Deployable_MG_MG42_Mounted | `{81EB5737EB00F71C}Prefabs/Weapons/Machineguns/MG42/Deployable_MG_MG42_Mounted.et` | Belt_792x57_MG34_100rnd_4AP_1APT |
| MG_MG42 | `{885A147293847F5C}Prefabs/Weapons/Machineguns/MG42/MG_MG42.et` | Belt_792x57_MG34_50rnd_4Ball_1Tracer |
| MG_MG42_Mounted | `{E7C0CF953CAD5BEC}Prefabs/Weapons/Machineguns/MG42/MG_MG42_Mounted.et` | Belt_792x57_MG34_100rnd_4AP_1APT |
| Submachinegun_MP38 | `{C81F18A853BE165A}Prefabs/Weapons/Machineguns/MP38/Submachinegun_MP38.et` | Magazine_9x19_MP38_32rnd |
| Submachinegun_MP40 | `{6D18C148A668A0AB}Prefabs/Weapons/Machineguns/MP38/Submachinegun_MP40.et` | Magazine_9x19_MP38_32rnd |
| Rifle_Gewehr43 | `{D0B2A68A3BD9085E}Prefabs/Weapons/Rifles/Gewehr43/Rifle_Gewehr43.et` | Magazine_792x57_Gewehr43_10rnd_Ball_sP |
| Rifle_Gewehr43_Optic | `{75DDC146073EFD22}Prefabs/Weapons/Rifles/Gewehr43/Rifle_Gewehr43_Optic.et` | Magazine_792x57_Gewehr43_10rnd_Ball_sP |
| Rifle_Kar98K | `{3B460BBAF682C13D}Prefabs/Weapons/Rifles/Kar98K/Rifle_Kar98K.et` | Clip_792x57_Kar98k_5rnd_Internal_Base |
| Rifle_Kar98K_Scheissbecher | `{AEE8B1A7356E943E}Prefabs/Weapons/Rifles/Kar98K/Rifle_Kar98K_Scheissbecher.et` | - |
| Rifle_Kar98K_Sniper | `{EB136040466EF11D}Prefabs/Weapons/Rifles/Kar98K/Rifle_Kar98K_Sniper.et` | Clip_792x57_Kar98k_5rnd_Internal_Base |
| Rifle_STG44 | `{D9340572C7EF6CB7}Prefabs/Weapons/Rifles/STG44/Rifle_STG44.et` | Magazine_792x33_STG44_30rnd_Ball_SmE |
| Rifle_STG44_Optic | `{5E600AF664D7E2D7}Prefabs/Weapons/Rifles/STG44/Rifle_STG44_Optic.et` | Magazine_792x33_STG44_30rnd_Ball_SmE |

### ChungusBoltGuns

| Arme | Resource name | Magasin par defaut (-> calibre) |
|---|---|---|
| BC_Rifle_M40A1_3x9 | `{CF19339F31E2B529}Prefabs/Weapons/Rifles/M40A1/BC_Rifle_M40A1_3x9.et` | BC_Magazine_762x51_M40A1_Internal |
| BC_Rifle_M40a1 | `{182A93106D779AB2}Prefabs/Weapons/Rifles/M40A1/BC_Rifle_M40a1.et` | BC_Magazine_762x51_M40A1_Internal |
| BC_Rifle_Mosin_Nagant_9130 | `{C1B4F7B87B720EE8}Prefabs/Weapons/Rifles/Mosin/BC_Rifle_Mosin_Nagant_9130.et` | BC_Magazine_762x54_Stripper_5rnd_Mosin_Nagant |
| BC_Rifle_Mosin_Nagant_9130_PU | `{6545DEABED4759E5}Prefabs/Weapons/Rifles/Mosin/BC_Rifle_Mosin_Nagant_9130_PU.et` | - |

### FinestHour-CoreMod

| Arme | Resource name | Magasin par defaut (-> calibre) |
|---|---|---|
| Deployable_MG_M1919A4_Mounted | `{33A5FBAFD8F61DA2}Prefabs/Weapons/MachineGuns/M1919/Deployable_MG_M1919A4_Mounted.et` | Belt_762x63_M1919_100rnd_4Ball_1Tracer |
| MG_M1919A4 | `{AB3A3ADC8CE3F848}Prefabs/Weapons/MachineGuns/M1919/MG_M1919A4.et` | Belt_762x63_M1919_100rnd_4Ball_1Tracer |
| MG_M1919A4_Mounted | `{80DC4AC8E8C1E69A}Prefabs/Weapons/MachineGuns/M1919/MG_M1919A4_Mounted.et` | Belt_762x63_M1919_100rnd_4Ball_1Tracer |
| MG_M1919A4_Mounted_APT | `{6F2E9F3FE334C731}Prefabs/Weapons/MachineGuns/M1919/MG_M1919A4_Mounted_APT.et` | Box_762x63_M1919_coax_200rnd_4AP_1Tracer |
| MG_M1919_pintle | `{FB4EF18F10D69A50}Prefabs/Weapons/MachineGuns/M1919/MG_M1919_pintle.et` | Box_762x63_M1919_200rnd_Base |
| Deployable_MG_MG34_Mounted | `{2CDBAC89AB5E1A25}Prefabs/Weapons/MachineGuns/MG34/Deployable_MG_MG34_Mounted.et` | Belt_792x57_MG34_100rnd_4Ball_1Tracer |
| MG_MG34 | `{5D838074E66C006C}Prefabs/Weapons/MachineGuns/MG34/MG_MG34.et` | Belt_792x57_MG34_50rnd_4Ball_1Tracer |
| MG_MG34_Mounted | `{B6CFD1B80AF074FF}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_Mounted.et` | Belt_792x57_MG34_100rnd_4Ball_1Tracer |
| MG_MG34_Panzerlauf_BigMag | `{3156C88745C229CB}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_Panzerlauf_BigMag.et` | Box_792x57_MG34_300rnd_4Ball_1Tracer |
| MG_MG34_pintle | `{B39E2727461818B5}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_pintle.et` | Drum_792x57_MG34_50rnd_4Ball_1Tracer |
| MG_MG34_pintle_BigMag | `{E905C02CE2BFE944}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_pintle_BigMag.et` | Box_792x57_MG34_300rnd_4Ball_1Tracer |

### FinestHour-GERVehicles

| Arme | Resource name | Magasin par defaut (-> calibre) |
|---|---|---|
| MG_MG34_Panzerlauf_SDKFZ_222 | `{424A61CB01CFE8D5}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_Panzerlauf_SDKFZ_222.et` | - |
| MG_MG34_pintle_Kubelwagen | `{D6CD01E4FAF32471}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_pintle_Kubelwagen.et` | - |
| MG_MG34_pintle_SDKFZ_223 | `{CE9EB5A0EA96D76A}Prefabs/Weapons/MachineGuns/MG34/MG_MG34_pintle_SDKFZ_223.et` | - |

Tripodes/affuts : `{18BE8D308869EA00}Prefabs/Items/Equipment/Tripods/M1919/DeployablePreview_Tripod_M2_M1919.et`, `{83526B8F395D8AA1}Prefabs/Items/Equipment/Tripods/M1919/DeployablePreview_Tripod_M2_M1919A6.et`, `{257D5BD54A6970BE}Prefabs/Weapons/Tripods/Deployable_Tripod_M2_MG_M1919.et`, `{0D64F5A499E214D2}Prefabs/Weapons/Tripods/Deployable_Tripod_M2_MG_M1919A6.et`, `{F868EA9EDAE19389}Prefabs/Weapons/Tripods/Tripod_M2_MG_M1919.et`, `{3BD629D722CE3FC9}Prefabs/Weapons/Tripods/Tripod_M2_MG_M1919A6.et`, `{1AFD17964A91F765}Prefabs/Weapons/Tripods/Tripod_M3_M2HB_WW2.et`, `{CD869CC64CE42B08}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG34.et`, `{61E2C1C74C4467EA}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG34_MGZ34.et`, `{718AEFB4C08803AC}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG42.et`, `{6C068EF3F8A260AE}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG42_MGZ34.et`, `{8252A0BDB4F46812}Prefabs/Weapons/Tripods/Feldlafette/Deployable_Tripod_Feldlafette_MG34.et`, `{74B0A323A45507F6}Prefabs/Weapons/Tripods/Feldlafette/Deployable_Tripod_Feldlafette_MG34_MGZ34.et`, `{657AFB45E06A45E1}Prefabs/Weapons/Tripods/Feldlafette/Deployable_Tripod_Feldlafette_MG42.et`, `{CF8FB044E06DFB0E}Prefabs/Weapons/Tripods/Feldlafette/Deployable_Tripod_Feldlafette_MG42_MGZ34.et`, `{4F38111D8FBCB6BE}Prefabs/Weapons/Tripods/Feldlafette/Tripod_Feldlafette.et`, `{D201F4FC6F14F443}Prefabs/Weapons/Tripods/Feldlafette/Tripod_Feldlafette_MG34.et`, `{F12099906E6485CA}Prefabs/Weapons/Tripods/Feldlafette/Tripod_Feldlafette_MG34_MGZ34.et`, `{BA495D41AA49AB29}Prefabs/Weapons/Tripods/Feldlafette/Tripod_Feldlafette_MG42.et`, `{CE02B4EEF01D02AF}Prefabs/Weapons/Tripods/Feldlafette/Tripod_Feldlafette_MG42_MGZ34.et`

WCS_Armaments / WCS_SpaceCore : armes **modernes** (AK, M16, lance-roquettes, missiles) - hors sujet WW2, non detaillees (voir `WCS_Armaments.guids.txt`).

## 4. Munitions, grenades, explosifs

### FinestHour-USGear

**Grenades** : `{DB78B0C03A81B662}Prefabs/Weapons/Grenades/M15/Smoke_M15.et`; `{FF76B659B282DFFB}Prefabs/Weapons/Grenades/M16/Smoke_M16_Green.et`; `{59281854D7B77DE3}Prefabs/Weapons/Grenades/M16/Smoke_M16_Purple.et`; `{9BD4DBE5880B377C}Prefabs/Weapons/Grenades/M16/Smoke_M16_Red.et`; `{317E432F7873B898}Prefabs/Weapons/Grenades/M16/Smoke_M16_White.et`; `{692475C5F123A0B3}Prefabs/Weapons/Grenades/M16/Smoke_M16_Yellow.et`; `{F832D80F885FD79A}Prefabs/Weapons/Grenades/M7AT/Grenade_M7AT.et`; `{9404B8186F1501E7}Prefabs/Weapons/Grenades/Mk2/Grenade_Mk2_Olive.et`; `{034A85564D466EF4}Prefabs/Weapons/Grenades/Mk2/Grenade_Mk2_Yellow.et`; `{2456A80E3F243B64}Prefabs/Weapons/Grenades/No82/Grenade_No82_300g.et`; `{3BB958D8E5152FED}Prefabs/Weapons/Grenades/No82/Grenade_No82_600g.et`; `{1B8949A38B461276}Prefabs/Weapons/Grenades/No82/Grenade_No82_900g.et`; `{F50CC22B107BD2A9}Prefabs/Weapons/Grenades/No82/Grenade_No82_Empty.et`

**Explosifs / mines** : `{97F68A24558DD2E0}Prefabs/Weapons/Explosives/DemoBlock_TNT/DemoBlock_USA_TNT_HalfPound.et`; `{4D36B96919838361}Prefabs/Weapons/Explosives/DemoBlock_TNT/DemoBlock_USA_TNT_OnePound.et`; `{6D0188BDB92DE608}Prefabs/Weapons/Explosives/Mine_M16/Mine_M16.et`; `{23858F4966E84B94}Prefabs/Weapons/Explosives/Mine_M1A1/Mine_M1A1AT.et`; `{BA46483D6A6B7C75}Prefabs/Weapons/Explosives/Mine_M7/Mine_M7.et`

**Lanceurs (munitions roquette/rifle grenade)** : `{94247C2AB94C64D8}Prefabs/Weapons/Ammo/No82/Ammo_Grenade_AT_No82_Spall_300g.et`; `{8BCB8CFC637D7051}Prefabs/Weapons/Ammo/No82/Ammo_Grenade_AT_No82_Spall_600g.et`; `{ABFB9D870D2E4DCA}Prefabs/Weapons/Ammo/No82/Ammo_Grenade_AT_No82_Spall_900g.et`; `{877358E6B7C6CA85}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_FRAG_M1.et`; `{6FF2B85ECC531A20}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_FRAG_M17.et`; `{F1FA4CEF5631DADA}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_HEAT_M9A1.et`; `{079F4F4EA54E2C0C}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_M9A1_Spall.et`; `{01BA8A140198C7E2}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_M2A1_Green.et`; `{4D18522AAAC0E777}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_M2A1_Purple.et`; `{D2515CB8AD2A1263}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_M2A1_Red.et`; `{1F27C93E0277491E}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_M2A1_WP.et`; `{7C52A6E738A1BDB0}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_M2A1_White.et`; `{7D143FBB8C543A27}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_M2A1_Yellow.et`; `{AE985EECB43A23CA}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_WP_M19A1.et`

**Magasins (30)** : voir `FinestHour-USGear.guids.txt` (`Prefabs/Weapons/Magazines/*`).

### FinestHour-GERGear

**Grenades** : `{886DA8E3CFF0C54F}Prefabs/Weapons/Grenades/GeballteLadung/Grenade_GeballteLadung.et`; `{C13716F9532136B8}Prefabs/Weapons/Grenades/M24_Stielhandgranate/Grenade_M24.et`; `{BE26CD5EB5FF1374}Prefabs/Weapons/Grenades/M39_Eihandgranate/Grenade_M39.et`; `{25FEB029453493DA}Prefabs/Weapons/Grenades/M39_Nebelhandgranate/Smoke_M39.et`; `{6ECA4AC24EABC2BB}Prefabs/Weapons/Grenades/M42_NebelEihandgranate/Smoke_M42.et`

**Explosifs / mines** : `{43088ECD9718F1B3}Prefabs/Weapons/Explosives/DemoBlock_3KgGeballteLadung/DemoBlock_3kg_GeballteLadung.et`; `{4BC976E7F93ABDBC}Prefabs/Weapons/Explosives/DemoBlock_Bohrpatrone28/DemoBlock_Bohrpatrone.et`; `{BFE70512FA0E536F}Prefabs/Weapons/Explosives/DemoBlock_Sprengbuchse24/DemoBlock_Sprengbuchse.et`; `{4DCC542BB946F1AA}Prefabs/Weapons/Explosives/DemoBlock_Sprengkorper28/DemoBlock_German_Fuze_ZZ35.et`; `{E5733C9B96D19F57}Prefabs/Weapons/Explosives/DemoBlock_Sprengkorper28/DemoBlock_Sprengkorper.et`; `{A3618339FCCAA75A}Prefabs/Weapons/Explosives/Mine_SMine35/Mine_SMine35.et`; `{9049080FDB51254F}Prefabs/Weapons/Explosives/Mine_Tellermine42/Mine_Tellermine42AT.et`

**Lanceurs (munitions roquette/rifle grenade)** : `{01B9CD4D94F42FD0}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_GGP30_Penetrator.et`; `{6844D5060FC7BDED}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_GGP30_Spall.et`; `{55026BAE7B6B09DF}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_GGP40_Penetrator.et`; `{E440D879D718BE27}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_GGP40_Spall.et`; `{79BD0FD9D579383C}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_HEAT_GGP30.et`; `{7E7AF588B3FF82FF}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_HEAT_GGP40.et`; `{882C6D916E53AADD}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_HE_GSprng30.et`; `{E55284937832F657}Prefabs/Weapons/Ammo/Rifle_Grenades/Ammo_Grenade_Smoke_GewehrNebelGrenate_White.et`

**Magasins (25)** : voir `FinestHour-GERGear.guids.txt` (`Prefabs/Weapons/Magazines/*`).

CoreMod : munitions (`Prefabs/Weapons/Ammo`, 122 fichiers), obus (`Configs/Weapons/Ammo/ShellConf`), tables balistiques mortier, warheads : cf. `FinestHour-CoreMod.guids.txt`.

## 5. Tenues, casques, gilets, equipements

### FinestHour-USUniforms

**Casques / couvre-chefs (17)**

- `{5AA5F82DF6924487}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01.et`
- `{B3273ADD157E2FB2}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01_Officer.et`
- `{E96E6CA5A81DEE7C}Prefabs/Characters/HeadGear/Hat_GarrisonCap_01/Hat_GarrisonCap_01_base.et`
- `{315D6B514A3B01E1}Prefabs/Characters/HeadGear/Hat_ServiceCap_01/Hat_ServiceCap_01_Captain.et`
- `{C21EF752A1C5034D}Prefabs/Characters/HeadGear/Hat_ServiceCap_01/Hat_ServiceCap_01_base.et`
- `{C919759CBEBFEE3D}Prefabs/Characters/HeadGear/Helmet_A11_01/Helmet_A11_01.et`
- `{258ED62B31D24B2F}Prefabs/Characters/HeadGear/Helmet_A11_01/Helmet_A11_01_base.et`
- `{A5A8AA43CE3F7D24}Prefabs/Characters/HeadGear/Helmet_M1938_01/Helmet_M1938_Tanker_01.et`
- `{D6591FA032274D7B}Prefabs/Characters/HeadGear/Helmet_M1938_01/Helmet_M1938_Tanker_01_base.et`
- `{50058D39B81A5590}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01.et`
- `{29D5728C6E476630}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_01_base.et`
- `{AC96B59D4F15AEC6}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Medic.et`
- `{89EB785CDE795008}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_NCO.et`
- `{15405974502B16A4}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting.et`
- `{0C6685EF6A9A4382}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Netting_Goggles.et`
- `{5364BB5C94391CB5}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_PlatoonLeader.et`
- `{9883084BCFCEB903}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_ww2_Scrim.et`

**Vestes / pantalons / combinaisons (23)**

- `{66E83C69885ADF71}Prefabs/Characters/Uniforms/M37_Jacket/Jacket_USA_M37.et`
- `{C15897A19782B5A5}Prefabs/Characters/Uniforms/M37_Jacket/Jacket_USA_M37_Officer.et`
- `{E891A69499608AA9}Prefabs/Characters/Uniforms/M37_Jacket/Jacket_USA_M37_base.et`
- `{30FF9AF93D543E4B}Prefabs/Characters/Uniforms/M37_Trousers/Pants_USA_M37.et`
- `{87FF0D45F22379BE}Prefabs/Characters/Uniforms/M37_Trousers/Pants_USA_M37_base.et`
- `{5257D583335C6DED}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41.et`
- `{748265DA39F23623}Prefabs/Characters/Uniforms/M41_Jacket/Jacket_USA_M41_base.et`
- `{35BD45F9F734DC81}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT.et`
- `{06815D81694DB48B}Prefabs/Characters/Uniforms/M41_Trousers/Pants_USA_M41_HBT_base.et`
- `{441FBCB19F80AB3D}Prefabs/Characters/Uniforms/M42_Jacket/Jacket_USA_M42.et`
- `{855368424612F205}Prefabs/Characters/Uniforms/M42_Jacket/Jacket_USA_M42_base.et`
- `{9DD4E9E9CBF92F4B}Prefabs/Characters/Uniforms/M42_Trousers/Pants_USA_M42.et`
- `{EA3DC3932D510112}Prefabs/Characters/Uniforms/M42_Trousers/Pants_USA_M42_base.et`
- `{8877C4F9636D04FC}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43.et`
- `{80A6AB85AD4008B3}Prefabs/Characters/Uniforms/M43_Jacket/Jacket_USA_M43_base.et`
- `{8C9163B5C656BF41}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT.et`
- `{4AFD251A8829E04D}Prefabs/Characters/Uniforms/M43_Trousers/Pants_USA_M43_HBT_base.et`
- `{4C4C6BA177B108C2}Prefabs/Characters/Uniforms/Suit_Pilot_USA_01/Suit_Pilot_USA_WW2_01.et`
- `{F5D6C638F4ED25C2}Prefabs/Characters/Uniforms/Suit_Pilot_USA_01/Suit_Pilot_USA_WW2_01_base.et`
- `{1522AC27CAAFFC90}Prefabs/Characters/Uniforms/Suit_Tanker_US_01/Suit_Tanker_HBT_USA_01.et`
- `{28E8A8A7B09D5D98}Prefabs/Characters/Uniforms/Suit_Tanker_US_01/Suit_Tanker_HBT_USA_01_base.et`
- `{E765AF7B18C6FD88}Prefabs/Characters/Uniforms/Tanker_Jacket/Jacket_USA_Tanker.et`
- `{8055D194276E36FD}Prefabs/Characters/Uniforms/Tanker_Jacket/Jacket_USA_Tanker_Base.et`

**Gilets (6)**

- `{D0145C7C4FFDD972}Prefabs/Characters/Vests/Vest_ALICE/M1936_Pistol_Belt_Dummy.et`
- `{CAB061361A861B31}Prefabs/Characters/Vests/Vest_M1936/M1936_Suspenders.et`
- `{F53DD82A72B21C44}Prefabs/Characters/Vests/Vest_M1936/M1936_Suspenders_Medic.et`
- `{8A4A6301DEA24523}Prefabs/Characters/Vests/Vest_M2/Vest_M2_Ammo.et`
- `{E9A138F01AD22F99}Prefabs/Characters/Vests/Vest_MaeB4/Vest_MaeB4.et`
- `{0399C18DED91EF99}Prefabs/Characters/Vests/Vest_MaeB4/Vest_MaeB4_base.et`

**Chaussures / gants (5)**

- `{C4C115CE1BBF2864}Prefabs/Characters/Footwear/JumpBoots_USA_M1942.et`
- `{EDE057A6A4F9B6E4}Prefabs/Characters/Footwear/ServiceBoots_USA_M1939.et`
- `{C12FF65FE10E1850}Prefabs/Characters/Footwear/ServiceBoots_USA_M1943.et`
- `{7633E68EF4C16FC6}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_B3_01.et`
- `{D93BFA41CC3E1247}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_B3_01_base.et`

**Sacs / radios / accessoires (28)**

- `{56D2D1474E135027}Prefabs/Items/Equipment/Accessories/Pouch_M1928/Vest_M1923_AmmoBelt.et`
- `{1AD1F501C4768406}Prefabs/Items/Equipment/Accessories/Pouch_M1928/Vest_M1937_AmmoBelt.et`
- `{88D9BBE89A05EDFD}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Bandolier_Garand.et`
- `{51C75C83CA956563}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Bandolier_Garand_Opposite.et`
- `{4395BBFFDF743BBF}Prefabs/Items/Equipment/Accessories/Pouch_M1936/M1911_Holster.et`
- `{40883151E6BC4D35}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_Carbine_2cell.et`
- `{FB9D352C9C9326F7}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_GrenadeLauncher.et`
- `{D7DC0462E75A4A22}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_M1911_2cell.et`
- `{D8045482476CE419}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_M1_Ammo.et`
- `{2A32F93D746A34EA}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_Medic_Left.et`
- `{CB13DF744D04EFD2}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_Medic_Right.et`
- `{85DD54528B1C061D}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_Rigger.et`
- `{6D21677886AAAE87}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_Rigger_Base.et`
- `{AA77D2FD336B7B5B}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Pouch_Thompson_5cell.et`
- `{716A64F74E3C8F72}Prefabs/Items/Equipment/Accessories/Pouch_M1936/US_M1910_EToolPouch.et`
- `{4568AFE679D75D4F}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Vest_M1936_Haversack.et`
- `{3C92E00C7D7E8670}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Vest_M1936_MusetteBag.et`
- `{5B4F0765A7130A99}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Vest_M1936_firstaid_01.et`
- `{27B5DCB40FC9BE1D}Prefabs/Items/Equipment/Accessories/Pouch_M1936/Vest_M1936_firstaid_02.et`
- `{D2EC17376735B564}Prefabs/Items/Equipment/Backpacks/Backpack_M1928_Haversack.et`
- `{38BE58A28782DF5D}Prefabs/Items/Equipment/Backpacks/Backpack_M1942_Mountain.et`
- `{32826411AAEBB128}Prefabs/Items/Equipment/Backpacks/Backpack_M1942_Mountain_assembled.et`
- `{58C156A7615F2CC6}Prefabs/Items/Equipment/Backpacks/Backpack_M1942_frame.et`
- `{4A57DA4511C10310}Prefabs/Items/Equipment/Backpacks/Backpack_Packboard_AssistantMachinegunner.et`
- `{CAB7587A8D6D84CF}Prefabs/Items/Equipment/Canteens/Canteen_US_WW2_01.et`
- `{D4FB527D24801975}Prefabs/Items/Equipment/Radios/Radio_SCR300/Radio_SCR300.et`
- `{4BDF41C8A5325732}Prefabs/Items/Equipment/Radios/Radio_SCR300/Radio_SCR300_Object.et`
- `{FA263AEF92D15E82}Prefabs/Items/Equipment/Radios/Radio_SCR300/Radio_SCR300_base.et`

Gilets assembles (classes) : 31 - utilises par les loadouts (`Vest_M1936_Assembled_*`, `Airborne/*` pour paras) ; voir `FinestHour-USUniforms.guids.txt`.

### FinestHour-GERUniforms

**Casques / couvre-chefs (45)**

- `{C3C3B64455FC06C6}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Feldgrau.et`
- `{A2204C99FAEE7437}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Feldgrau_Officer.et`
- `{6B9A925F761EA72E}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Luftwaffe.et`
- `{B7E00EB4AEC4CDA9}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_Luftwaffe_Officer.et`
- `{C239E728425F9297}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_TankCommander.et`
- `{79770150D9286630}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_TankCrew.et`
- `{D6A21035CE207A5E}Prefabs/Characters/HeadGear/Hat_M40_01/Hat_M40_01_base.et`
- `{A34221AB5A70F92C}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Feldgrau.et`
- `{C01593243CEF1CD0}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Feldgrau_Officer.et`
- `{8D09738BB0973263}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Luftwaffe.et`
- `{11A1834F37D76D7E}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Luftwaffe_Officer.et`
- `{F6A9BD9EA6F00C13}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_Splinter.et`
- `{D35C610DE45084FA}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_TankCommander.et`
- `{868355C2B87B0AB2}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_TankCrew.et`
- `{CB89D54C82D92DDA}Prefabs/Characters/HeadGear/Hat_M43_01/Hat_M43_01_base.et`
- `{BE55599A2F4F26D7}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Feldgrau.et`
- `{9EFF51DCB14F4F20}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Luftwaffe.et`
- `{47ECF4DCE5F7DB8F}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_Panzer.et`
- `{E22BE59B322FA676}Prefabs/Characters/HeadGear/Hat_Schirmmutze_01/Hat_Schirmmutze_01_base.et`
- `{C962F894BC540739}Prefabs/Characters/HeadGear/Helmet_LkpW10_01/Helmet_LkpW10_01.et`
- `{255299697AFF6CAB}Prefabs/Characters/HeadGear/Helmet_LkpW10_01/Helmet_LkpW10_01_base.et`
- `{CFEF7AB8A8009A23}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_01_base.et`
- `{EFEEC9D16CB32D5F}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_01.et`
- `{D3CA375028BCFBDC}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Blue.et`
- `{811C6EEB0B5CEF42}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Medic.et`
- `{470AD17E10E1847D}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Netting.et`
- `{D87EBB4343F30CF5}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Painted.et`
- `{453C9F329FAD3293}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_Painted_Netting.et`
- `{5043B8775B038535}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M35_01/Helmet_Stahlhelm_M35_White.et`
- `{D62A72E894FA605B}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_01.et`
- `{D701C67DCF9473F7}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_01_base.et`
- `{5AE56AB1C4C4C5EB}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Blue.et`
- `{0EFFE36965141049}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Green_Cover.et`
- `{882D47A6BB3F43A2}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Medic.et`
- `{122B49F881289F42}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Medic_Blue.et`
- `{C56B9D549F8B495A}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Painted.et`
- `{4E1FC79E3E28A143}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M38_01/Helmet_Stahlhelm_M38_Splinter_Cover.et`
- `{E975113A3BA2E57B}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Blue.et`
- `{307C507641AE33C7}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Cover_Splinter.et`
- `{747BA3B3D60B3676}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Feldgrau.et`
- `{83FA82A5EB3172EB}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Medic.et`
- `{19463B1006FA1EF4}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Net.et`
- `{049D5DAB3BA65BB1}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Painted.et`
- `{4A446BA39DF5DAD2}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_Painted_Net.et`
- `{52A55439BB6E189C}Prefabs/Characters/HeadGear/Helmet_Stahlhelm_M42_01/Helmet_Stahlhelm_M42_White.et`

**Vestes / pantalons / combinaisons (30)**

- `{4DEC71CFC20990F4}Prefabs/Characters/Uniforms/Blouse_M36/Jacket_HEER_M36_Feldgrau.et`
- `{9D6868213E70EC18}Prefabs/Characters/Uniforms/Blouse_M36/Jacket_HEER_M36_Luftwaffe.et`
- `{6B956AA4616FCAC6}Prefabs/Characters/Uniforms/Blouse_M36/Jacket_HEER_M36_base.et`
- `{B7E538F48ED5B12F}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_Feldgrau.et`
- `{051E32E58B7C5853}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_Luftwaffe.et`
- `{F165D224E569B6CD}Prefabs/Characters/Uniforms/Blouse_M40/Jacket_HEER_M40_base.et`
- `{A39F24A1384336F8}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Feldgrau.et`
- `{AD732800E3688E96}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_Luftwaffe.et`
- `{D935F64046FB4180}Prefabs/Characters/Uniforms/Blouse_M43/Jacket_HEER_M43_base.et`
- `{DB91C6B1FC2158A8}Prefabs/Characters/Uniforms/Jacket_Pilot_HEER_01/Jacket_Pilot_HEER_01.et`
- `{E142C9145A3A49CD}Prefabs/Characters/Uniforms/Jacket_Pilot_HEER_01/Jacket_Pilot_HEER_01_base.et`
- `{7844FAD04826C7EE}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01.et`
- `{F11EA277AC283473}Prefabs/Characters/Uniforms/Jacket_Tanker_HEER_01/Jacket_Tanker_HEER_01_base.et`
- `{F639D6C29B03C5B1}Prefabs/Characters/Uniforms/Pants_Pilot_HEER_01/Pants_Pilot_HEER_01.et`
- `{E3EB0260B1BE429F}Prefabs/Characters/Uniforms/Pants_Pilot_HEER_01/Pants_Pilot_HEER_01_base.et`
- `{5AC77EE3E5EF3277}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01.et`
- `{F42DE0F09C464110}Prefabs/Characters/Uniforms/Pants_Tanker_HEER_01/Pants_Tanker_HEER_01_base.et`
- `{10548CE06620830A}Prefabs/Characters/Uniforms/Smock_M38/Jacket_HEER_M38_Olive.et`
- `{B68B08E8EC896865}Prefabs/Characters/Uniforms/Smock_M38/Jacket_HEER_M38_Splinter.et`
- `{55128130E92326EB}Prefabs/Characters/Uniforms/Smock_M38/Jacket_HEER_M38_base.et`
- `{BA4CDA33D60F0A3F}Prefabs/Characters/Uniforms/Trousers_M36/Pants_HEER_M36_Feldgrau.et`
- `{3559BE99EAEA4BA1}Prefabs/Characters/Uniforms/Trousers_M36/Pants_HEER_M36_Luftwaffe.et`
- `{B8B84C0270E32B66}Prefabs/Characters/Uniforms/Trousers_M36/Pants_HEER_M36_base.et`
- `{7033D8D854B77A37}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_Feldgrau.et`
- `{7123BD938A95A2DB}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_Luftwaffe.et`
- `{EE3C6EA02216554F}Prefabs/Characters/Uniforms/Trousers_M40/Pants_HEER_M40_base.et`
- `{8DF157D185B22225}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Feldgrau.et`
- `{ACB191B3129C68FA}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_Luftwaffe.et`
- `{32DF8442E056A8AC}Prefabs/Characters/Uniforms/Trousers_M43/Pants_HEER_M43_base.et`
- `{EC8BE4A0CDFAA8D8}Prefabs/Characters/Uniforms/Trousers_Springerhose/Pants_HEER_Springerhose_base.et`

**Gilets (3)**

- `{7DB9617422FB832D}Prefabs/Characters/Vests/M1939/M1939_Base.et`
- `{7A36A297D2B22AF6}Prefabs/Characters/Vests/Vest_M1030/Vest_Model1030.et`
- `{AA361ECC43C6AE09}Prefabs/Characters/Vests/Vest_M1030/Vest_Model1030_base.et`

**Chaussures / gants (8)**

- `{D7BC415E8552E5F8}Prefabs/Characters/Footwear/Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et`
- `{E454E94528AE883D}Prefabs/Characters/Footwear/Boots_Fliegerstiefel/Boots_HEER_Fliegerstiefel_01.et`
- `{FD5C132FF8AF2185}Prefabs/Characters/Footwear/JackBoots_M39/CombatBoots_HEER_M39_01.et`
- `{6550F264F1E19FCB}Prefabs/Characters/Footwear/LowBoots_M43/CombatBoots_HEER_M43_01.et`
- `{FDBAE1C8FE705B97}Prefabs/Characters/Footwear/LowBoots_Panzer/TankBoots_HEER_01.et`
- `{217BAD5FF36D5083}Prefabs/Characters/Footwear/Offiziersstiefel_01/Boots_HEER_Offiziersstiefel_01.et`
- `{8D38A884E46F7B85}Prefabs/Characters/Handwear/Gloves_Pilot_HEER_01/Gloves_Pilot_HEER_01.et`
- `{AC8BA90A22CAFDD9}Prefabs/Characters/Handwear/Gloves_Pilot_HEER_01/Gloves_Pilot_HEER_01_base.et`

**Sacs / radios / accessoires (11)**

- `{2EEEC603359D29C3}Prefabs/Items/Equipment/Backpacks/Backpack_M34.et`
- `{11BFF6F49979A267}Prefabs/Items/Equipment/Backpacks/Backpack_M34_Medic.et`
- `{83BA2C46B3FEF331}Prefabs/Items/Equipment/Backpacks/Backpack_M38_GasMaskCannister.et`
- `{03E37E2AFC8BCE83}Prefabs/Items/Equipment/Backpacks/Backpack_M38_GasMaskCannister_Medic.et`
- `{9DD5617396560272}Prefabs/Items/Equipment/Backpacks/Backpack_M41.et`
- `{0E6D30BF706E290B}Prefabs/Items/Equipment/Backpacks/Backpack_Packframe_AssistantMachinegunner.et`
- `{CCD1BDDB37A07BB2}Prefabs/Items/Equipment/Backpacks/Backpack_Packframe_Panzerschreck.et`
- `{8FE50B3CD5F26667}Prefabs/Items/Equipment/Headset/Headset_HEER/Headset_HEER_01.et`
- `{21EAF96EF9F99B84}Prefabs/Items/Equipment/Headset/Headset_HEER/Headset_HEER_01_base.et`
- `{E987460B5D904AEB}Prefabs/Items/Equipment/Radios/Radio_Feldfunksprecher-F/Radio_feldfunksprecherf.et`
- `{751AB09F8B2E7256}Prefabs/Items/Equipment/Radios/Radio_Feldfunksprecher-F/Radio_feldfunksprecherf_base.et`

Gilets assembles (classes) : 0 - utilises par les loadouts (`Vest_M1936_Assembled_*`, `Airborne/*` pour paras) ; voir `FinestHour-GERUniforms.guids.txt`.

### FinestHour-USGear - objets (Items) : `{2A2F3A62EDD51EC7}Prefabs/Items/Equipment/Accessories/ETool_M1910/ETool_M1910.et`; `{07114788AA3E7D4D}Prefabs/Items/Equipment/Accessories/ETool_M1910/ETool_M1910_FreeRoamBuilding_Gadget.et`; `{739833307400D844}Prefabs/Items/Equipment/Accessories/ETool_M1910/ETool_M1910_Stowed_base.et`; `{51FF040839F2F611}Prefabs/Items/Equipment/Accessories/ETool_M1910/ETool_M1910_base.et`; `{C9FDB120FE3A9581}Prefabs/Items/Equipment/Backpacks/AmmoVest_M2.et`; `{0D2C38AB8E984AA0}Prefabs/Items/Equipment/BallisticTable/BallisticTable_USA_105mm_M2A1.et`; `{7D6DFB20646F0B79}Prefabs/Items/Equipment/BallisticTable/BallisticTable_USA_75mm_M1A1.et`; `{3B6039C2D0B3DBC0}Prefabs/Items/Equipment/BallisticTable/BallisticTable_USA_81mm_M1.et`; `{0B8B76B888ED5187}Prefabs/Items/Equipment/Binoculars/Binoculars_M9/Binoculars_M9.et`; `{802858C4DE8BF168}Prefabs/Items/Equipment/Binoculars/Binoculars_M9/Binoculars_M9_base.et`; `{9BF601D55A5EFBE2}Prefabs/Items/Equipment/Compass/Compass_M1938.et`; `{A615AF4521400BBC}Prefabs/Items/Equipment/Compass/Compass_M1938_Map.et`; `{8497103FB9E63BA3}Prefabs/Items/Equipment/Kits/RearmingKit_01/RearmingKit_01_USAWW2.et`; `{31820BBB5C327325}Prefabs/Items/Equipment/Radios/Radio_SRC536.et`; `{18BE8D308869EA00}Prefabs/Items/Equipment/Tripods/M1919/DeployablePreview_Tripod_M2_M1919.et`; `{83526B8F395D8AA1}Prefabs/Items/Equipment/Tripods/M1919/DeployablePreview_Tripod_M2_M1919A6.et`; `{D2E04AFED17BC4CE}Prefabs/Items/Equipment/Tripods/M1919/Part_Tripod_M2.et`; `{A96A2B8991F3640D}Prefabs/Items/Equipment/Watches/Watch_A11.et`; `{D243CF3E2362475A}Prefabs/Items/Equipment/Watches/Watch_A11_Map.et`; `{133721236588F5F0}Prefabs/Items/Medicine/FieldDressing_01/AirborneFieldDressing_US_WW2.et`; `{F1F575193D07ECE7}Prefabs/Items/Medicine/FieldDressing_01/AirborneFieldDressing_US_WW2_Base.et`; `{70F1C6C5696344A2}Prefabs/Items/Medicine/FieldDressing_01/AirborneFieldDressing_US_WW2_Green.et`; `{9070F5AB358FE739}Prefabs/Items/Medicine/FieldDressing_01/CarlisleFieldDressing_US_WW2.et`; `{C48E25E788BAF080}Prefabs/Items/Medicine/FieldDressing_01/CarlisleFieldDressing_US_WW2_Base.et`

### FinestHour-GERGear - objets (Items) : `{324AC4246D1FB69A}Prefabs/Items/Equipment/Accessories/ETool_M1938/ETool_M1938.et`; `{01F664CDF986E8D0}Prefabs/Items/Equipment/Accessories/ETool_M1938/ETool_M1938_FreeRoamBuilding_Gadget.et`; `{0974CD2B6A2DBE7C}Prefabs/Items/Equipment/Accessories/ETool_M1938/ETool_M1938_Stowed_base.et`; `{9693F896C4FDC807}Prefabs/Items/Equipment/Accessories/ETool_M1938/ETool_M1938_base.et`; `{69113AAD9D348073}Prefabs/Items/Equipment/BallisticTable/BallisticTable_HEER_Grw34.et`; `{26B500979A1CDB3E}Prefabs/Items/Equipment/BallisticTable/BallisticTable_HEER_LeFH18.et`; `{613D7C4A1C1E6605}Prefabs/Items/Equipment/BallisticTable/BallisticTable_HEER_Leig18.et`; `{30D6485A99328588}Prefabs/Items/Equipment/Binoculars/Binoculars_Dienatglas_6x30/Binoculars_Dienatglas_6x30.et`; `{686CFDB529355849}Prefabs/Items/Equipment/Binoculars/Binoculars_Dienatglas_6x30/Binoculars_Dienatglas_6x30_base.et`; `{D12A730FD8A9DCB6}Prefabs/Items/Equipment/Compass/Compass_MaxKohl.et`; `{B201B618FD8EB37E}Prefabs/Items/Equipment/Compass/Compass_MaxKohl_Map.et`; `{047317DFCD1F5F71}Prefabs/Items/Equipment/Kits/MedicalKit_01/MedicalKit_01_HEER.et`; `{6D57DB51FEF2F660}Prefabs/Items/Equipment/Kits/MedicalKit_01/MedicalKit_01_HEER_Patronenkasten39.et`; `{EBDA5114E47D0308}Prefabs/Items/Equipment/Kits/RearmingKit_01/RearmingKit_01_HEER.et`; `{CD869CC64CE42B08}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG34.et`; `{61E2C1C74C4467EA}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG34_MGZ34.et`; `{718AEFB4C08803AC}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG42.et`; `{6C068EF3F8A260AE}Prefabs/Items/Equipment/Tripods/Feldlafette/DeployablePreview_Tripod_Feldlafette_MG42_MGZ34.et`; `{A20C58071E87D8BD}Prefabs/Items/Equipment/Tripods/Feldlafette/Part_Feldlafette_Tripod.et`; `{2C64E2FEE7B9A056}Prefabs/Items/Equipment/Watches/Watch_Deinstuhr.et`; `{74D6B104D7ED3A6F}Prefabs/Items/Equipment/Watches/Watch_Deinstuhr_Map.et`; `{4DC939F9FA9D5349}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_HEER_WW2.et`; `{77E35E58DA48923C}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_HEER_WW2_Base.et`

## 6. Vehicules

Roles/equipage : Pilote/Cargo/Tourelle = nombre de `CompartmentSlot` dans le .et racine ; les sieges ajoutes par les VehParts (prefabs `VehParts/Seats`) augmentent le total reel.

| Vehicule | Resource name | Role | Slots P/C/T |
|---|---|---|---|
| M4A2_Sherman | `{CFF64B0230B6BD3D}Prefabs/Vehicles/Core/M4A2_Sherman.et` | Char moyen M4A2 Sherman (75 mm M3) | 1/5/0 |
| Kubelwagen | `{1D3D9E6ED1DED30D}Prefabs/Vehicles/Wheeled/Kubelwagen/Kubelwagen.et` | Voiture legere 4x4 (jeep allemande) | 1/1/0 |
| Kubelwagen_Canvas | `{0373FCB583E39633}Prefabs/Vehicles/Wheeled/Kubelwagen/Kubelwagen_Canvas.et` | Voiture legere bachee | 1/1/0 |
| Kubelwagen_MG34 | `{2F09C97004CFD1B5}Prefabs/Vehicles/Wheeled/Kubelwagen/Kubelwagen_MG34.et` | Voiture legere + MG34 | 0/0/0 |
| Opil_Bletz_Armed_Flak38 | `{6CFEE036B08596A6}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_Armed_Flak38.et` | Camion Opel Blitz | 0/0/0 |
| Opil_Bletz_Armed_Flakvierling | `{EBA339690C35F864}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_Armed_Flakvierling.et` | Camion Opel Blitz | 0/0/0 |
| Opil_Bletz_Arsenal | `{1DA54D8491C30502}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_Arsenal.et` | Camion Opel Blitz | 0/0/0 |
| Opil_Bletz_ammo | `{6CD8B6D9848D7877}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_ammo.et` | Camion Opel Blitz | 0/0/0 |
| Opil_Bletz_command | `{34E9C5F482B11D65}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_command.et` | Camion Opel Blitz | 0/1/0 |
| Opil_Bletz_engineer | `{FC9C7C7EFCF8C216}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_engineer.et` | Camion Opel Blitz | 0/1/0 |
| Opil_Bletz_repair | `{55CD8305C5032C21}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_repair.et` | Camion Opel Blitz | 0/1/0 |
| Opil_Bletz_tanker | `{974E023D12A97DC7}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_tanker.et` | Camion Opel Blitz | 0/0/0 |
| Opil_Bletz_transport | `{AEA2E1BD58418A47}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_transport.et` | Camion Opel Blitz | 0/0/0 |
| Opil_Bletz_transport_covered | `{D0A6F804DFFE34A8}Prefabs/Vehicles/Wheeled/Opil_Bletz/Variants/Opil_Bletz_transport_covered.et` | Camion Opel Blitz | 0/0/0 |
| SDKFZ_222_Camo | `{C5BD2985070AABC2}Prefabs/Vehicles/Wheeled/SDKFZ_222/SDKFZ_222_Camo.et` | Voiture blindee legere (canon 20 mm KwK38 + MG) | 0/0/0 |
| SDKFZ_222_Desert | `{EDC4DE87D69BACB7}Prefabs/Vehicles/Wheeled/SDKFZ_222/SDKFZ_222_Desert.et` | Voiture blindee legere (canon 20 mm KwK38 + MG) | 0/0/0 |
| SDKFZ_222_Grey | `{BF2C38CC0B258B42}Prefabs/Vehicles/Wheeled/SDKFZ_222/SDKFZ_222_Grey.et` | Voiture blindee legere (canon 20 mm KwK38 + MG) | 0/0/0 |
| SDKFZ_222_base | `{4EA2A715AE8A7663}Prefabs/Vehicles/Wheeled/SDKFZ_222/SDKFZ_222_base.et` | Voiture blindee legere (canon 20 mm KwK38 + MG) | 1/0/0 |
| SDKFZ_223_base | `{DD871923492033E3}Prefabs/Vehicles/Wheeled/SDKFZ_223/SDKFZ_223_base.et` | Voiture blindee radio | 1/0/0 |
| Trailer_SdAh51_transport | `{6AC81C73AAA103DD}Prefabs/Vehicles/Wheeled/SdAh51_Trailer/Trailer_SdAh51_transport.et` | Remorque | 0/0/0 |

`0/0/0` = slots herites du prefab parent (variantes). Equipage reel : Kubelwagen 1 pilote + passagers ; SdKfz 222 pilote + tourelle (2-3 membres) ; Opel Blitz 1 pilote + cargo ; Sherman 1 pilote + 5 cargo (sieges equipage via tourelle, Weapons/Core/M4A2_Turret_base).
**Artillerie / pieces tractees et fixes** (voir section 7).

**WW2TankCore** : bibliotheque (ammo Tiger I H, sons Pershing/Tiger, parties de BRDM2/UAZ) - **ne contient PAS de prefab de char jouable** (le Tiger I n'a que ses 3 magasins `vehparts` et ses munitions `Prefabs/Weapons/Ammo/TigerH1/*`).

**CIE Tracked Core / CIE Thermal / Chungus Core** : bibliotheques techniques (`Vehicles/Tracked/LTM_Tracked_Vehicles_Base.et`, thermique, base chargeurs) ; aucun vehicule WW2 directement placable.

**WCS_Armaments** : vehicules modernes (UH-1H, Mi-8MT, BTR-70, UAZ, Ural, M998/M1025, S105 rally...) - hors periode. Reperes utiles hors mods : vehicules **vanilla Reforger** M151A2 (`{F649585ABB3706C4}Prefabs/Vehicles/Wheeled/M151A2/M151A2.et`) et M923A1 (`{F1FBD0972FA5FE09}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport.et`) = substituts visuels US (annees 60-80).

## 7. Artillerie, AA, AT, canons, mortiers

**GERVehicles - AT/AA/obusiers (Grey = defaut 1944)** :
- `{5A9635F186D9ED7A}Prefabs/Weapons/Artillery/AntiAir/Flak36/AntiAir_Flak36_Grey.et`
- `{2F525FAE65787BA9}Prefabs/Weapons/Artillery/AntiAir/Flak36/Shielded/AntiAir_Flak36_Grey_Shield.et`
- `{CEB82E16B98A70FE}Prefabs/Weapons/Artillery/AntiAir/Flak38/Autocannon_Flak38_Grey.et`
- `{0738C61CC031AA6B}Prefabs/Weapons/Artillery/AntiAir/Flak38/Deployable/Deployable_Autocannon_Flak38_Grey.et`
- `{CC43A6F94B93E5F1}Prefabs/Weapons/Artillery/AntiAir/Flak38/Trailer_Flak38_transport_Grey.et`
- `{EC38169982271966}Prefabs/Weapons/Artillery/AntiAir/Flak38/VehicleMounted/Autocannon_Flak38_Grey_VehicleMounted.et`
- `{B3449CFECFCD1A63}Prefabs/Weapons/Artillery/AntiAir/Flakvierling/Autocannon_Flakvierling_Grey.et`
- `{A1267B6EC31DE460}Prefabs/Weapons/Artillery/AntiAir/Flakvierling/Autocannon_Flakvierling_Shield_Grey.et`
- `{8F85129E86469D54}Prefabs/Weapons/Artillery/AntiAir/Flakvierling/VehicleMounted/Autocannon_Flakvierling_Shield_Grey_VehicleMounted.et`
- `{04EBE874F22A9287}Prefabs/Weapons/Artillery/AntiAir/SW36/SearchLight_SW36_Grey.et`
- `{4E7ACC1579DF2772}Prefabs/Weapons/Artillery/AntiTank/Pak36/AntiTank_Pak36_Grey.et`
- `{BBE97295CC36DBA3}Prefabs/Weapons/Artillery/AntiTank/Pak36/Trailer_Pak36_Grey.et`
- `{FCF55AA8F7F1FECC}Prefabs/Weapons/Artillery/AntiTank/Pak40/AntiTank_Pak40_Grey.et`
- `{68E4475C4A59CEE4}Prefabs/Weapons/Artillery/AntiTank/Pak40/Trailer_Pak40_Grey.et`
- `{690579024C3EE645}Prefabs/Weapons/Artillery/Howitzer/LeFH18/Howitzer_LeFH18_Grey.et`
- `{F755C948C81DBDCD}Prefabs/Weapons/Artillery/Howitzer/LeFH18/Trailer_LeFH18_Grey.et`
- `{A3397E731682B0A1}Prefabs/Weapons/Artillery/Howitzer/Leig18/Howitzer_Leig18_Grey.et`
- `{711D0D3DEA37B256}Prefabs/Weapons/Artillery/Howitzer/Leig18/Trailer_Leig18_Grey.et`

**GERVehicles - mortiers** :
- `{8BE4C8A97C5FD444}Prefabs/Weapons/Mortars/Grw34/Mortar_Gwr34.et`

**CoreMod - canons US/GER (HeavyWeapons)** :
- `{50DCC1D6553AF327}Prefabs/Weapons/HeavyWeapons/AntiAir/90mm_M1/Cannon_90mm_M1.et`
- `{686089D981124345}Prefabs/Weapons/HeavyWeapons/AntiAir/90mm_M1/Cannon_90mm_M1_Base.et`
- `{87270DE327433790}Prefabs/Weapons/HeavyWeapons/AntiAir/Bofors_L60/AutoCannon_Bofors_L60.et`
- `{369B098A0B33A1FF}Prefabs/Weapons/HeavyWeapons/AntiAir/Bofors_L60/AutoCannon_Bofors_L60_Deployable.et`
- `{A3DEACB80FEAB174}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak36/Cannon_Flak_36_Base.et`
- `{B10E2819C90811B7}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak36/Cannon_Flak_36_Camo.et`
- `{7CC8AA4D52CEADE2}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak36/Cannon_Flak_36_Desert.et`
- `{CB9F3950C5273137}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak36/Cannon_Flak_36_Grey.et`
- `{E14B9CCB7A70FDF1}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Autocannon_Flak38_Base.et`
- `{BF75E449545A2EB5}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Autocannon_Flak38_Camo.et`
- `{C348590FF8FF3B70}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Autocannon_Flak38_Desert.et`
- `{C5E4F50058750E35}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Autocannon_Flak38_Grey.et`
- `{4E12D2C9BFADFCAA}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Deployable/Deployable_Autocannon_Flak38_Camo.et`
- `{40D6BA741AC25FD6}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Deployable/Deployable_Autocannon_Flak38_Desert.et`
- `{3483C380B382DC2A}Prefabs/Weapons/HeavyWeapons/AntiAir/Flak38/Deployable/Deployable_Autocannon_Flak38_Grey.et`
- `{2220B5EAD4AB98DD}Prefabs/Weapons/HeavyWeapons/AntiAir/Flakvierling/Autocannon_Flak38_Flakvierling.et`
- `{3069ADD7C6795731}Prefabs/Weapons/HeavyWeapons/AntiAir/Flakvierling/Autocannon_Flak38_Flakvierling_Camo.et`
- `{7ED133FE318B785A}Prefabs/Weapons/HeavyWeapons/AntiAir/Flakvierling/Autocannon_Flak38_Flakvierling_Desert.et`
- `{DB46BE73514B6EC0}Prefabs/Weapons/HeavyWeapons/AntiAir/M45/HMG_M45_Quadmount.et`
- `{860828BF6A3780B2}Prefabs/Weapons/HeavyWeapons/AntiAir/M45/HMG_M45_Quadmount_Deployable.et`
- `{62E9F65F65D8AC5D}Prefabs/Weapons/HeavyWeapons/Autocannon/KwK_38/Autocannon_KwK_38_Base.et`
- `{87F7D70CF04AB950}Prefabs/Weapons/HeavyWeapons/Autocannon/KwK_38/Autocannon_KwK_38_Camo.et`
- `{FD66C645FC6599D0}Prefabs/Weapons/HeavyWeapons/Autocannon/KwK_38/Autocannon_KwK_38_Grey.et`
- `{7C75FEC5CC061861}Prefabs/Weapons/HeavyWeapons/Autocannon/KwK_38/Autocannon_KwK_38_Tan.et`
- `{99D9C3108EED5DC4}Prefabs/Weapons/HeavyWeapons/Cannon/105mm_M2A1/Cannon_M2A1.et`
- `{BEF70C5309CF577F}Prefabs/Weapons/HeavyWeapons/Cannon/105mm_M2A1/Cannon_M2A1_Base.et`
- `{F1D98AD9507BD5D2}Prefabs/Weapons/HeavyWeapons/Cannon/76mm_M5/Cannon_76mm_M5.et`
- `{AB7D2069AB074103}Prefabs/Weapons/HeavyWeapons/Cannon/76mm_M5/Cannon_76mm_M5_Base.et`
- `{B43C11E807B3E6C2}Prefabs/Weapons/HeavyWeapons/Cannon/LeFH18/Cannon_LeFH18_Base.et`
- `{95223E532B534576}Prefabs/Weapons/HeavyWeapons/Cannon/LeFH18/Cannon_LeFH18_Camo.et`
- `{C903CB676E763CDC}Prefabs/Weapons/HeavyWeapons/Cannon/LeFH18/Cannon_LeFH18_Desert.et`
- `{EFB32F1A277C65F6}Prefabs/Weapons/HeavyWeapons/Cannon/LeFH18/Cannon_LeFH18_Grey.et`
- `{DA1948FF589FCE46}Prefabs/Weapons/HeavyWeapons/Cannon/Leig18/Cannon_Leig18_Base.et`
- `{DA249E6F803E14A7}Prefabs/Weapons/HeavyWeapons/Cannon/Leig18/Cannon_Leig18_Camo.et`
- `{044A61DD3DB6923E}Prefabs/Weapons/HeavyWeapons/Cannon/Leig18/Cannon_Leig18_Desert.et`
- `{A0B58F268C113427}Prefabs/Weapons/HeavyWeapons/Cannon/Leig18/Cannon_Leig18_Grey.et`
- `{4FE2C087C4B0BDE5}Prefabs/Weapons/HeavyWeapons/Cannon/Pak36/Cannon_Pak_36_Base.et`
- `{733D933A1269246D}Prefabs/Weapons/HeavyWeapons/Cannon/Pak36/Cannon_Pak_36_Camo.et`
- `{5E8FE18A4FAE2C58}Prefabs/Weapons/HeavyWeapons/Cannon/Pak36/Cannon_Pak_36_Desert.et`
- `{09AC82731E4604ED}Prefabs/Weapons/HeavyWeapons/Cannon/Pak36/Cannon_Pak_36_Grey.et`
- `{0D2EEAC55594318C}Prefabs/Weapons/HeavyWeapons/Cannon/Pak40/Cannon_Pak_40_Base.et`
- `{A030A6F39406312A}Prefabs/Weapons/HeavyWeapons/Cannon/Pak40/Cannon_Pak_40_Camo.et`
- `{4E0194D8A650E095}Prefabs/Weapons/HeavyWeapons/Cannon/Pak40/Cannon_Pak_40_Desert.et`
- `{DAA1B7BA982911AA}Prefabs/Weapons/HeavyWeapons/Cannon/Pak40/Cannon_Pak_40_Grey.et`
- `{A2651C5EF156E158}Prefabs/Weapons/HeavyWeapons/M2HB/HMG_M2HB_WW2.et`

**USGear - lanceurs** :
- `{FBBB0FC51FC85173}Prefabs/Weapons/Launchers/M1A1/Launcher_M1A1_Bazooka.et`
- `{789C42020B955F1E}Prefabs/Weapons/Launchers/M1A1/Launcher_M1A1_Bazooka_base.et`

**GERGear - lanceurs** :
- `{EADCBEFD2705B515}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust100.et`
- `{15F0473A1454A801}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust60.et`
- `{9F9C81507278D3B8}Prefabs/Weapons/Launchers/Panzerfaust/Launcher_Panzerfaust60_base.et`
- `{BA0ECE0E5883ECE7}Prefabs/Weapons/Launchers/RPzB_43/Launcher_RPzB43_Green.et`
- `{E6972FA7C8CABC7F}Prefabs/Weapons/Launchers/RPzB_43/Launcher_RPzB43_Tan.et`
- `{31EE8B678D8E6195}Prefabs/Weapons/Launchers/RPzB_43/Launcher_RPzB43_base.et`
- `{28B66C43B847AE14}Prefabs/Weapons/Launchers/RPzB_54/Launcher_RPzB54_Green.et`
- `{454330C5B5BF0733}Prefabs/Weapons/Launchers/RPzB_54/Launcher_RPzB54_Tan.et`
- `{D499A8E0BD1C4DD8}Prefabs/Weapons/Launchers/RPzB_54/Launcher_RPzB54_base.et`

Fire missions : `PrefabsEditable/EffectsModules/Mortar/*` (barrages zones, WP) dans CoreMod ; tables balistiques `Configs/Weapons/Ammo/MortarBallisticTableGeneratorConfigs`.

## 8. Autres

**Arsenaux / caisses (32)** : `{87DC7D3BF1DB6908}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Ammunition_HEER.et`; `{E638CA727AC148C1}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Ammunition_HQ_HEER.et`; `{C1DA85CB654C8A62}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Equipment_HEER.et`; `{34D35B4BCABC294A}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Explosives_HEER.et`; `{9D54A046890D24F4}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Launcher_HEER.et`; `{51645317AE0AB785}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Medical_HEER.et`; `{3847B1FD4FECDFBB}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Medical_HQ_HEER.et`; `{1CF4433D7689BBE1}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Mortar_HEER.et`; `{4CD3B6037E68C40C}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_VehicleGadget_HEER.et`; `{8A5CB547B3984B52}Prefabs/Props/Military/Arsenal/AmmoBoxes/HEER/AmmoBoxArsenal_Weapons_HEER.et`; `{DFEFED6DBE7F9F80}Prefabs/Props/Military/Arsenal/AmmoBoxes/US/AmmoBoxArsenal_Ammunition_HQ_US_WW2.et`; `{BB090BD516BA077C}Prefabs/Props/Military/Arsenal/AmmoBoxes/US/AmmoBoxArsenal_Ammunition_US_WW2.et` ...

**Services / spawn US** : `{31820BBB5C327325}Prefabs/Items/Equipment/Radios/Radio_SRC536.et`

**Northcom German Voice Pack** : sons/voix allemandes uniquement (aucun .et/.conf ; 140 fichiers audio) - cf. `NorthcomGermanVoicePack.listing.txt`.

**Radio / protocole** : `Sounds/RadioProtocol/RadioProtocol_Report_2_US.acp` (US) ; voix GER via Northcom.
