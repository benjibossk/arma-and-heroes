# Audit armes — mods installés sur le serveur WW2 (2026-11-25)

Périmètre : uniquement les 25 mods de `D:\Serveur\arma-reforger-ww2\config.json` (pas de recherche Workshop).
Source : `resourceDatabase.rdb` de chaque addon (`profile/addons`) + `inventory/extract/ALL_GUIDS.txt`. Tous les GUID ci-dessous ont été validés contre ALL_GUIDS.

## Ce que fournit chaque mod (armes portatives)

| Mod | Armes |
|---|---|
| Chungus Core (66D6340C1D565C98) | aucune arme (bibliothèque/dépendance) |
| Chungus Bolt Guns (61BD6595183FCEBD) | M40A1 (moderne, 7.62x51), Mosin-Nagant 91/30 (+PU) — **aucune arme US/GER WW2 utile** |
| FH CoreMod | MG34, M1919A4 (+ armes lourdes : Flak 36/38, Pak 36/40, LeFH18, M2HB, Bofors…) |
| FH GER Gear | Kar98k (+Sniper, +Schiessbecher), G43 (+ZF4), STG44 (+optique), FG42, MP38, MP40, MG42, P38, Leuchtpistole 34, Panzerfaust 60/100, RPzB 43/54, M24, M39, Nebel M39/M42, Geballte Ladung |
| FH US Gear | M1 Garand, M1/M1A1/M2 Carbine, M1903A3/A4, Thompson M1A1, M3A1 Grease Gun, BAR M1918, M1919A6, M1911A1, AN-M8, Bazooka M1A1, Mk2, M7AT, Gammon No.82, fumigènes M15/M16 |
| RMS Op Ostfront German Arms | doublons Kar98k, MP40, MG34, MG42, P38, Panzerfaust, Stielhandgranate (variantes RMS) |
| RMS Panzer IV | MG15 / MG34 de tourelle (véhicule) |
| WCS Armaments / SpaceCore, CIE, WW2 Tank Core | armes modernes ou de véhicule — hors sujet |
| Autres (FH Uniforms/Units/Vehicles, Sherman, Tiger, Sturmtiger, SdKfz 251, M3A1, Overlord, Modest, Bicycle, Northcom voix) | pas d'arme portative |

## Mapping catalogue (préférence FH)

| Clé | Prefab | Mod | Statut |
|---|---|---|---|
| US_THOMPSON | `{E59A4E6BBDE7CFBD}Prefabs/Weapons/Machineguns/M1A1/Submachinegun_M1A1.et` | FH US Gear | **rempli** (était null) |
| US_M3_GREASE | `{E14DFCA44B4ADB90}Prefabs/Weapons/Machineguns/M3A1/Submachinegun_M3A1.et` | FH US Gear | **rempli** |
| GER_G43 | `{D0B2A68A3BD9085E}Prefabs/Weapons/Rifles/Gewehr43/Rifle_Gewehr43.et` | FH GER Gear | **rempli** |
| GER_OUTFIT_FJ | — | — | **manquant** : aucun smock Fallschirmjäger installé (seules les bottes `Boots_HEER_JumpBoots_01` existent) |
| US_M1903A3 | `{7CA5C7688880A688}…/M1903A3/Rifle_M1903A3.et` | FH US Gear | nouveau |
| US_M1A1_CARBINE | `{8D7E8742E9F08061}…/M1_Carbine/Rifle_M1A1_Carbine.et` | FH US Gear | nouveau |
| US_M2_CARBINE | `{F34FA45A2DE16C2D}…/M1_Carbine/Rifle_M2_Carbine.et` | FH US Gear | nouveau |
| US_M1919A4 | `{AB3A3ADC8CE3F848}…/MachineGuns/M1919/MG_M1919A4.et` | FH Core | nouveau |
| US_M1919A6 | `{DE2075CB6E5CAE3A}…/Machineguns/M1919/MG_M1919A6.et` | FH US Gear | nouveau |
| US_M7AT_GRENADE | `{F832D80F885FD79A}…/Grenades/M7AT/Grenade_M7AT.et` | FH US Gear | nouveau |
| US_GAMMON | `{3BB958D8E5152FED}…/Grenades/No82/Grenade_No82_600g.et` | FH US Gear | nouveau |
| US_FLAREGUN | `{DBD5C4E1C81D8363}…/Handguns/ANM8/Handgun_ANM8.et` | FH US Gear | nouveau |
| GER_STG44 | `{D9340572C7EF6CB7}…/Rifles/STG44/Rifle_STG44.et` | FH GER Gear | nouveau |
| GER_G43_SCOPE | `{75DDC146073EFD22}…/Gewehr43/Rifle_Gewehr43_Optic.et` | FH GER Gear | nouveau |
| GER_FG42 | `{F6BD5104680D2991}…/FG42G/Machinegun_FG42G.et` | FH GER Gear | nouveau |
| GER_MP38 | `{C81F18A853BE165A}…/MP38/Submachinegun_MP38.et` | FH GER Gear | nouveau |
| GER_MG34 | `{5D838074E66C006C}…/MachineGuns/MG34/MG_MG34.et` | FH Core | nouveau |
| GER_KAR98K_GL | `{AEE8B1A7356E943E}…/Kar98K/Rifle_Kar98K_Scheissbecher.et` | FH GER Gear | nouveau |
| GER_LP34 | `{72B77F14D2073B5D}…/LP34/Handgun_Leichtpistole34.et` | FH GER Gear | nouveau |
| GER_PANZERFAUST100 | `{EADCBEFD2705B515}…/Panzerfaust/Launcher_Panzerfaust100.et` | FH GER Gear | nouveau |
| GER_M39_GRENADE | `{BE26CD5EB5FF1374}…/M39_Eihandgranate/Grenade_M39.et` | FH GER Gear | nouveau |
| GER_GEBALLTE | `{886DA8E3CFF0C54F}…/GeballteLadung/Grenade_GeballteLadung.et` | FH GER Gear | nouveau |

Déjà mappés avant l'audit (inchangés) : M1 Garand, M1 Carbine, BAR, M1903A4, M1911, Bazooka, Mk2, M16 smoke, Kar98k (+ZF), MP40, MG42, P38, Panzerfaust 60, Panzerschreck (RPzB54), M24, Nebel M39.

## Manquant même après l'audit (liste H&G)

- **Luger P08** et **Walther PPK** : absents de tous les mods installés (seul P38).
- **Smock Fallschirmjäger** (GER_OUTFIT_FJ) : absent.
- Chungus : n'apporte rien d'utile aux factions FH (Mosin = soviétique, M40A1 = moderne) ; Chungus Core + Bolt Guns pourraient être retirés du serveur si aucune autre dépendance ne les exige.
- Les variantes RMS Ostfront (K98k, MP40, MG34/42, P38, Panzerfaust) font doublon avec FH ; non utilisées.
