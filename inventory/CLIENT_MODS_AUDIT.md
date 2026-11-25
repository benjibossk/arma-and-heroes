# Audit des mods client (serveurs WW2 rejoints le 2026-11-25)

Source : 166 dossiers créés aujourd'hui dans `Documents/My Games/ArmaReforger/addons` (lecture seule).
Méthode identique à WEAPONS_AUDIT.md / AIRCRAFT_PARA.md : `addon.gproj` (dépendances), `ServerData.json`
(version, gameVersion), noms de prefabs `.et` extraits de `resourceDatabase.rdb`, taille du dossier.
Le dossier de 166 mods contient le pack **WARFARE - WORLD WAR II 670F0D77153F7212** (v3.4.8, 48 dépendances), qui
explique à lui seul la plupart des téléchargements, plus **Operation FUBAR WW2 Factions** et les packs Operation Overlord.

Déjà sur notre serveur (config.json) : Chungus Core/BoltGuns, Finest Hour (Core, GER/US Uniforms/Gear/Units, GER Vehicles),
WW2 Tank Core, M4A2 Sherman, Tiger H1, RMS Sturmtiger, RMS Op Ostfront German Arms, **RMS WW2 PANZER IV 692C3806D8C77463**,
SDKFZ 251, M3A1 HalfTrack, OO US Vehicles, Modest VehicleCore, WW2_Bicycle.

## 1. Fiche par mod

| Mod | ID | Ver. | Taille | Dépendances (hors vanilla 58D0FB3206B6F859) | Contenu (prefabs) |
|---|---|---|---|---|---|
| Chungus SMGs | 60E6F54E174C53C5 | 1.0.51 | 140 Mo | aucune | M1928A1 (drum/stick), M1A1, M3A1 Grease Gun, MP40, PPSh-41, M/45. Fait pour Vietnam '67. |
| Chungus Rifles | 62A711001B8FDEEA | 1.0.42 | 283 Mo | aucune | M1 Garand (clip 8), M1/M2 Carbine, StG44. |
| Chungus WW2 Launchers | 65E86BF91D510331 | 1.0.10 | 34 Mo | aucune | Panzerfaust 60, M1A1 Bazooka. |
| Chungus LMGs | 61344BDC155A5A28 | 1.0.24 | 91 Mo | aucune | BAR M1918A2 (+ version allégée), RPD. |
| Chungus Shotguns | 620E584B1D2C96A4 | 1.0.49 | 123 Mo | Chungus Core | M37 Trench Gun, 870, double canon. gameVersion 1.9.0.37. |
| OO-US Weapons Redone | 6982459DD20FAAAF | 1.0.6 | 141 Mo | OO Core, Chungus BoltGuns, Chungus Rifles | M1911A1, M1903, M1919A4/A6 + trépied déployable, M1 Carbine, M1A1 Para, Liberator. |
| OO GER Weapons Redone | 69713EEF88AEB163 | 1.0.7 | 186 Mo | OO Core, Chungus Core, Chungus BoltGuns | **Luger P08**, P38, Kampfpistole, Kar98k (+ sniper), G43, FG42, MG42. |
| OO Core Mod | 66CAC7A691A9A173 | 1.0.0 | 18 Mo | aucune | Base des deux packs ci-dessus. |
| OO Axis Gear | 5F04139AD86CE1E6 | 1.0.67 | 1461 Mo | SGS Gear Slots | Uniformes Heer/SS/Panzer, **Smock + JumpSuit + Jaeger** (Fallschirmjäger), couvre-casques splinter/plane/oak, grades. |
| OO US Gear | 5EE4E278D8EF2466 | 1.0.79 | 1270 Mo | Prontos Parachutes, SGS Gear Slots | Tenues GI. |
| OO US Gear V2 | 68420A16F02A8C16 | 1.0.58 | 715 Mo | SGS Gear Slots, Prontos Parachutes | Airborne : JumpSuit M42, bottes M35, sangles, sac parachute T-10. |
| US WW2 Overhaul | 6960FDEDB2542EAE | 0.0.21 | 4 Mo | OO-US Weapons, USGearV2, Chungus SMGs/Rifles/LMGs/Launchers | Remplace les loadouts US vanilla (IA) par du WW2. Pas d'asset propre. |
| Morks WW2 German Overhaul | 69384BDFBFA50990 | 0.1.26 | 26 Mo | Chungus Rifles/SMGs/Launchers, OO Axis Gear, RMS Ostfront, OO GER Weapons | Loadouts GER (StG, G43, **Para** riflemen/sniper), emplacements MG42. |
| Morks WW2 Vehicle Overhaul | 6A34127682A2A835 | 0.4.4 | 3 Mo | M4A2 Sherman, M3A1, OO US Vehicles, Chungus x4 | 12 variantes Sherman (.50, tourelle). Patch de config. |
| WW2 French Weapon Pack | 69923DEAF3F5E81E | 1.0.1 | 144 Mo | Chungus BoltGuns, Chungus LMGs | Berthier, MAS36 CR39, MAS38, Chauchat, Ruby. Modèles Sketchfab. |
| WW2 Resistance | 6A0451DAB70EC001 | 0.0.1 | <1 Mo | USGearV2, OO Axis Gear, Chungus, OO GER | Variantes FIA en résistants (riflemen seulement). |
| WW2 KubelWagen | 655E23F881127F8C | 1.0.5 | 26 Mo | aucune | Kübelwagen (le même modèle est dans OO German Vehicles 65AE1353B9C5BC41, avec MG). |
| Overlord US Vehicles (nouveau) | 6AD34498D47530CB | 1.0.1 | 578 Mo | aucune | Refonte du pack 65AD797399E3CF39 (WC63 avec affût). Doublon. |
| FIE-M4 Tank | 64B3018236DBE9A8 | 1.0.21 | 277 Mo | **Forged in Europe 5ADD31817243EAD5 (502 Mo)** | M4A2 75/76. |
| FIE-PanzerIV Tank | 6246D76D2C35F6D4 | 0.0.22 | 399 Mo | Forged in Europe | Pz IV Ausf. A/E/J, intérieur, râteliers de munitions. |
| RMS WW2 WarBirds II | 69E3578A86951C05 | 1.0.8 | 132 Mo | RMS Fixed-Wing Flight Core 6AAA520FE626344E, WCS Armaments 629B2BA37EFFD577 | **Ju 87 Stuka (bombes, canon), Ju 88** pilotables. Flight Core est marqué WIP. |
| WARFARE - WWII | 670F0D77153F7212 | 3.4.8 | 323 Mo | 48 mods (dont C-47, Aviation Toolkit, PropellerFlightCore, WarBirds II, Normandy...) | Scénario complet + **P51_Mustang** pilotable. |
| Parachute Framework | 65930CB4CD0237B2 | 1.2.0 | 6 Mo | aucune | Sac `Backpack_Kolobok_Parachute` + voile `DeployedParachuteMK4`. |
| BUSHWAR Parachutes | 66027627A36B8623 | 1.0.3 | 28 Mo | Parachute Framework, ZeliksCharacter | Sac + voile au look WW2. |
| Prontos Parachutes | 68996556700D5CA1 | 1.0.2 | 19 Mo | aucune | T-10 à ouverture automatique (sac + voile). Requis par les OO US Gear. |
| AeroplaneHavens C-47 Dakota | 66A2A0B8531C9D2A | 1.1.28 | 368 Mo | **Aviation-Toolkit 69B307004DF5C417** (JSBSim, gameVersion 1.9.0.37), **USAAF Bomber Attire 64F8C1AB4CFD207A** (147 Mo) | C-47 pilotable, `ParaCabin_Green/Red` (feu de saut). |
| WW2 US Plane props | 923645AE7ADAAABC | 1.0.4 | 432 Mo | aucune | B-17 (formation) + P-51 statiques. |
| WW2 German Plane Props | 4614D892F9E64A83 | 1.0.5 | 15 Mo | aucune | Bf 109 + He 111 statiques. |
| WW2 AA Searchlight | 69254840FFB76B49 | 1.0.1 | 4 Mo | aucune | Projecteur antiaérien statique. |
| Keep Gun When Uncon | 6088A3044B7ECBFD | 1.0.1 | <1 Mo | aucune | Garde l'arme à l'inconscience. |
| No Weapon Drop Unconscious | 60612C225328522E | 1.0.6 | <1 Mo | aucune | Même chose ; prendre un seul des deux (ou TWS-NoAutoWeaponDrop). |
| Normandy | 616D67C41B1DF93D | 0.0.39 | 526 Mo | WW2Assets 647804D31EC300F0 (**1787 Mo**), iRON79 Compositions 60A68ED7293C691A (513 Mo) | Carte Normandie. Total environ 2,8 Go. |
| WW2 Arland Map | 6072D68D8DCD0C12 | 0.0.34 | 158 Mo | aucune | Arland en version WW2. |
| Operation FUBAR WW2 Factions | 66FD25011BDDFB61 | 0.0.39 | 30 Mo | 22 mods (Chungus x4, BUSHWAR, Parachute Framework, Axis Gear, US Gear, FIE x5, Forged in Europe, SVT-40...) | Factions de conversion complète (Pz IV A/E/J via FIE, Kübel). |
| Operation FUBAR Launchers | 6714E842BCD5D889 | 0.0.1 | 1 Mo | aucune | **Panzerschreck**. |

Absents de tout le lot : **Walther PPK, Bren, Sten** (aucun prefab). L'arme française la plus proche est le pack WW2 French Weapon Pack.

## 2. Lacunes de notre serveur

| Lacune | Où la trouver | Verdict |
|---|---|---|
| Luger P08 | OO GER Weapons Redone (aussi FIE-GermanEquipment, qui tire Forged in Europe) | **OO GER** : 186 Mo + OO Core ; Chungus Core/BoltGuns sont déjà chez nous. |
| Walther PPK | aucun | Toujours une lacune. |
| Smock Fallschirmjäger | OO Axis Gear (`Smock`, `JumpSuit`, `Jaeger`, couvre-casques) | Le contenu est bon, mais le mod fait **1,46 Go** et fait doublon avec les uniformes GER de FH. |
| Panzer IV avec tourelle attachée | RMS WW2 PANZER IV, déjà chez nous (tourelles D/F/F2/G/H + Jagdpanzer) ; FIE-PanzerIV utilisé par FUBAR | FIE tire Forged in Europe (502 Mo) et un deuxième écosystème de factions. On reste sur RMS. |
| Chasseurs WW2 pilotables | WarBirds II (Stuka/Ju 88, Flight Core WIP) ; P-51 dans WARFARE (PropellerFlightCore) | Aucun chasseur allié autonome. Le Stuka est à tester. |
| Panzerschreck | FUBAR Launchers (1 Mo) | Ajouter. |
| Parachute | Prontos (T-10 auto), Parachute Framework + BUSHWAR | Voir section 3. |

## 3. C-47 + parachute sur les serveurs rejoints

- Le C-47 AeroplaneHavens **ne dépend d'aucun mod de parachute**. Il demande seulement **Aviation-Toolkit 69B307004DF5C417**
  (JSBSim, publié pour gameVersion **1.9.0.37**) et **USAAF Bomber Attire 64F8C1AB4CFD207A**.
- Sur ces serveurs, le parachute vient d'ailleurs :
  - **Prontos Parachutes 68996556700D5CA1**, tiré par OO US Gear et V2 (sac T-10 porté par les paras US) ;
  - **Parachute Framework 65930CB4CD0237B2 + BUSHWAR**, tiré par FUBAR.

  Le joueur saute du C-47 (sortie de compartiment) et la voile se déploie.
- **Grey Mans Chute Framework 69EEAA01F4A11001 ne figure dans aucun des 166 mods.** Notre erreur `Addon '69EEAA01F4A11001' not found`
  vient du fait qu'on référence ce mod : le dépôt Workshop ne le sert pas (ou plus) au serveur dédié. Ce n'est pas lié au C-47.
  **Correctif** : retirer 69EEAA01F4A11001 et utiliser **Prontos Parachutes** (autonome, 19 Mo) ou Parachute Framework (6 Mo).
- Point à vérifier : WARFARE fixe la version du jeu. Aviation-Toolkit et ChungusShotguns déclarent 1.9.0.37. Il faut aligner notre serveur dédié sur cette version.

## 4. Recommandation classée

**Ajouter**
1. Prontos Parachutes 68996556700D5CA1 : remplace Grey Mans, sans dépendance.
2. AeroplaneHavens C-47 66A2A0B8531C9D2A, Aviation-Toolkit 69B307004DF5C417 et USAAF Bomber Attire 64F8C1AB4CFD207A : environ 520 Mo, avion para prouvé en dédié.
3. OO Core 66CAC7A691A9A173 et OO GER Weapons Redone 69713EEF88AEB163 : Luger P08, P38, FG42, Kampfpistole.
4. Operation FUBAR Launchers 6714E842BCD5D889 : Panzerschreck.
5. Chungus SMGs 60E6F54E174C53C5 et Chungus Rifles 62A711001B8FDEEA : socle de Garand, Carbine, M1A1, M3, MP40, StG44. Ils sont requis par la plupart des packs WW2.
6. Chungus WW2 Launchers 65E86BF91D510331 et Chungus LMGs 61344BDC155A5A28 : Bazooka, Panzerfaust 60, BAR.
7. No Weapon Drop Unconscious 60612C225328522E (un seul des deux).

**Peut-être**
- OO-US Weapons Redone 6982459DD20FAAAF (M1919, M1903, M1911) : FH US couvre déjà en partie.
- OO Axis Gear 5F04139AD86CE1E6 : seule source du smock FJ, mais 1,46 Go et doublon avec les uniformes FH.
- RMS WarBirds II 69E3578A86951C05 + RMS FWFC 6AAA520FE626344E : Stuka pilotable (WIP), WCS Armaments est déjà requis par nos chars.
- WW2 AA Searchlight 69254840FFB76B49 et WW2 German Plane Props 4614D892F9E64A83 (15 Mo) : décor.
- WW2 French Weapon Pack 69923DEAF3F5E81E (résistance française). WW2 Arland Map 6072D68D8DCD0C12 (158 Mo) si on reste sur Arland.

**Ne pas prendre**
- US WW2 Overhaul et Morks German/Vehicle Overhaul : patches de loadouts IA pour la faction vanilla US, en conflit avec les factions FH.
- WW2 Resistance (0.0.1).
- FIE-M4 et FIE-PanzerIV : écosystème Forged in Europe, doublon de RMS et M4A2.
- FUBAR Factions : un troisième système de factions.
- WARFARE : le scénario entier.
- Overlord_Us_Vehicles 6AD34498D47530CB : doublon de 65AD797399E3CF39.
- WW2 KubelWagen : doublon.
- Chungus Shotguns : facultatif.
- Normandy : environ 2,8 Go avec ses dépendances, seulement si on change de carte.
- US Plane props : 432 Mo pour du décor.
- Parachute Framework + BUSHWAR : Prontos suffit.
- Keep Gun When Uncon : doublon.

## 5. Entrées config prêtes (non appliquées)

```json
{ "modId": "68996556700D5CA1", "name": "Prontos Parachutes" },
{ "modId": "69B307004DF5C417", "name": "Aviation-Toolkit" },
{ "modId": "64F8C1AB4CFD207A", "name": "USAAF Bomber Attire" },
{ "modId": "66A2A0B8531C9D2A", "name": "AeroplaneHavens C47 Dakota" },
{ "modId": "66CAC7A691A9A173", "name": "Operation Overlord Core Mod" },
{ "modId": "69713EEF88AEB163", "name": "OO_GERWeapons_Redone" },
{ "modId": "6714E842BCD5D889", "name": "Operation FUBAR Launchers" },
{ "modId": "60E6F54E174C53C5", "name": "Chungus SMGs" },
{ "modId": "62A711001B8FDEEA", "name": "Chungus Rifles" },
{ "modId": "65E86BF91D510331", "name": "Chungus WW2 Launchers" },
{ "modId": "61344BDC155A5A28", "name": "Chungus LMGs" },
{ "modId": "60612C225328522E", "name": "No Weapon Drop Unconscious" }
```
Optionnel : `{ "modId": "5F04139AD86CE1E6", "name": "Operation Overlord Axis Gear" }` + `{ "modId": "688CA726E38860F3", "name": "SGS Gear Slots" }`,
`{ "modId": "69E3578A86951C05", "name": "RMS WW2 WarBirds II" }` + `{ "modId": "6AAA520FE626344E", "name": "RMS-FIXEDWINGFLIGHTCORE" }`.
À retirer : `69EEAA01F4A11001` (Grey Mans Chute).
