# Avions & parachutistes — recherche + design (2026-11-25)

Statut : recherche uniquement, rien installé. Version/taille/licence de chaque mod = **à vérifier sur la page Workshop avant ajout** (non lues individuellement ; source = index de recherche du Workshop).
Référence H&G : classe Paratrooper (spawn par parachute depuis l'avion au-dessus du secteur, 12 spawns/joueur, 1 avion de transport actif max, air spawn 15 s) ; Pilot = chasseurs/attaque au sol, gate « Fighter Squadron » (HG_MECHANICS.md l.25-27, 82, 103, 115). Avions = « skip pour l'instant » (l.177).

## 1. Existant dans les mods installés
Scan des 28 `resourceDatabase.rdb` (profile/addons) sur parach/C47/Dakota/Ju52/Plane/Aircraft/P51/Bf109 :
**aucun avion, aucun parachute**. Seule occurrence : `Ammo_Flare_40mm_StarParachute_White.et` (FH US Gear, fusée éclairante). Vanilla Reforger : pas de composant parachute ni d'avion à voilure fixe (api_search « Parachute » : aucun résultat ; vanilla = hélicos seulement).

## 2. Workshop — candidats
### Parachutes
| Rang | Mod | ID | Notes |
|---|---|---|---|
| 1 | Grey Mans Chute Framework | 69EEAA01F4A11001 | **Chute libre HALO scriptée + voile pilotable**, aucune dépendance. Colle au besoin « ouverture manuelle ». |
| 2 | Prontos Parachutes | 68996556700D5CA1 | T-10 static line, autonome (plus besoin de Parachute Framework). |
| 3 | Static Line Parachutes (JDripstein) | 6A2F4BAC4D08F9F4 | Saut d'un « stick » depuis hélico en mouvement ; dépend de Prontos. Montre que le saut d'un véhicule en vol marche en MP. |
| 4 | BUSHWAR Parachutes | 66027627A36B8623 | Look WW2, static (ouverture auto) — visuel réutilisable. |
| - | Parachute Framework (Alphaegen) | 65930CB4CD0237B2 | Base historique, voiles contrôlables ; plusieurs dérivés (kuka, PLG, OGR, Daras, MFF, SOA V2, ParachuteAutoDeploy V3). |
| - | JMParaAI | 6A8C8CA472C34B77 | Avion IA (C-17/Il-112) traverse la carte et largue des paras IA au point cliqué ; avion non pilotable/non abattable. **Modèle de script à étudier** pour notre trajectoire. |

### Avions
| Rang | Mod | ID | Notes |
|---|---|---|---|
| 1 | Douglas C-47 Skytrain (CrackrDev) | 690B26AF12774741 | Prop **statique** prévu pour opérations para (ne vole pas seul) → parfait comme coque d'avion scripté. |
| 2 | AeroplaneHavens C47 Dakota | 66A2A0B8531C9D2A | Pilotable (JSBSim), orienté reconstitution ; stabilité serveur dédié à tester. |
| 3 | RMS WW2 WarBirds | 69E3578A86951C05 | Stuka + P-51 (+ à venir), même auteur que nos mods RMS (Panzer IV, Sturmtiger) ; pilotabilité à confirmer. |
| - | RMS Fixed Wing Flight Core | 6AAA520FE626344E | Modèle de vol warbirds, WIP. |
| - | Pingus WW2 P-51D & Bf109G | CA5F1717E5100000 | **IA seulement**, vol cinématique — patrouilles/CAS d'ambiance. |
| - | WW2 Gliders | 661ACB871721738A | Planeurs (Horsa/Waco ?) — piste pour assaut aéroporté. |
Pas de Ju 52, Fw 190, Spitfire Reforger trouvés.

## 3. Design paras (clarification : spawn DANS l'avion, saut libre, ouverture manuelle)
1. **Avion de transport scripté** (non pilotable) : prefab coque C-47 (CrackrDev, ou mesh simple si pas de mod) + composant serveur qui déplace l'entité sur une trajectoire (entrée hors zone → passage au-dessus du secteur choisi → boucle ou sortie), altitude ~300-400 m, vitesse ~60 m/s. Mouvement cinématique (SetOrigin/SetTransform par tick serveur, interpolé client), pas de physique de vol → stable en dédié et pas besoin d'IA pilote. 1 avion actif par camp (règle H&G).
2. **Spawn** : point de spawn « Para » = slots/compartiments passagers (`SCR_BaseCompartmentManagerComponent`, sièges cargo) de l'avion ; le joueur apparaît assis dedans (comme spawn dans un véhicule de RespawnSystem). Délai 15 s, 12 tickets para.
3. **Saut** : action « Sauter » (= sortie de compartiment) ; la sortie d'un véhicule en mouvement est déjà gérée par le moteur (preuve : Static Line Parachutes depuis hélico). Feu vert lumineux / ordre au-dessus de la zone (option : interdit hors zone).
4. **Chute libre + ouverture manuelle** : Grey Mans Chute Framework (freefall + voile pilotable) ; sinon notre composant : en chute (vitesse verticale > seuil, hauteur > 15 m) action « Ouvrir parachute » → attache une entité voile, drag fort, direction limitée. Sans ouverture → dégâts de chute vanilla (mort). Atterrissage : détache la voile, petite animation/ralenti.
5. **Largage de caisses** : même avion, action officier/général « Largage ravitaillement » (coût WF) → caisse munitions/médic. avec voile, descente cinématique, atterrit sur point choisi.
6. **Avions de combat** (plus tard) : assets financés par le général, classe Pilot, gate Fighter Squadron. Court terme : Pingus P-51/Bf109 IA en CAS/patrouille scriptée (mitraillage d'une zone), car aucun chasseur WW2 joueur fiable n'existe encore ; suivre RMS WarBirds + Flight Core.

## 4. Shortlist recommandée
1. **Grey Mans Chute Framework** (ouverture manuelle, 0 dépendance) — ou Prontos si on préfère static line.
2. **Douglas C-47 Skytrain (CrackrDev)** comme coque scriptée.
3. Optionnel : Pingus P-51D/Bf109G (IA d'ambiance/CAS) ; surveiller RMS WW2 WarBirds.
À vérifier avant ajout : licence de réutilisation (dépendance OK, pas de repack), tailles, test serveur dédié 20+ joueurs.

## 5. Repli sans nouveau mod
Avion = absent ou mesh placeholder invisible ; « spawn haut » : joueur spawné à 300 m au-dessus du point choisi sur la carte de déploiement, chute libre + notre composant parachute scripté (voile = mesh simple ou aucune, drag + action d'ouverture). Caisses : spawn en altitude + descente scriptée + fumigène. Tout faisable en script A&H Core, sans dépendance.

Sources : pages Workshop citées (reforger.armaplatform.com/workshop/<ID>).

## 6. Mécaniques H&G (wiki PDFs, HG_MECHANICS.md §11) → faisabilité
Légende : **OK** = faisable maintenant (vanilla + nos scripts) · **MOD** = nécessite le mod cité · **SKIP**.
| Mécanique | Statut | Moyen |
|---|---|---|
| Équipe PARA = soldats + ressources « avion de transport » (ratio 4:1) | OK | backend war map (teams.json / payload bataille) |
| Équipe PARA commandée par rang 15 / général | OK | règle backend |
| C-47 scripté en boucle au-dessus du secteur | MOD C-47 CrackrDev 690B26AF12774741 (coque statique) ; repli OK = mesh placeholder déplacé par script | composant serveur trajectoire |
| Joueur spawn assis dans l'avion | OK (compartiments cargo + spawn véhicule) une fois la coque dispo | dépend de la coque C-47 |
| Saut quand il veut (sortie compartiment en vol) | OK | moteur gère la sortie en mouvement |
| Chute libre + ouverture manuelle de la voile | MOD Grey Mans Chute 69EEAA01F4A11001 ; repli OK = composant drag maison | |
| Ju 52 côté GER | SKIP (aucun mod) → C-47 repeint/neutre ou même coque | |
| Chasseurs pilotés par joueurs (3 classes × 2 camps) | MOD RMS WW2 WarBirds 69E3578A86951C05 (P-51, Stuka) — pilotabilité à confirmer ; Bf109/Fw190/P-38/P-40 : SKIP (inexistants) | |
| Chasseurs IA (patrouille / mitraillage scripté) | MOD Pingus P-51D/Bf109G CA5F1717E5100000 | « soutien aérien » acheté par le général |
| 1 bombe par sortie, réarmement chronométré sur aérodrome | MOD (dépend de l'avion) ; timer = OK script | |
| Prix de spawn par sortie, achat par rang | OK | progression.json |
| Avion de reco 2 places (mitrailleur arrière) | SKIP (aucun mod) | |
| Modèle de dégâts / DCA | non décrit par la source ; DCA = rôle light_armored_aa existant | |

## 7. Proposition catalogue / équipes (NE PAS appliquer tel quel — à valider)
teams.json :
- `allowedRoles.PARA`: `["transport_plane"]` ; `vehicleRoles`: `US_C47`/`GER_C47`: `transport_plane` ; `vehicleShare.PARA`: 0.2.
- Nouveau type `AIR` : `allowedRoles.AIR`: `["fighter","fighter_ai"]`, `vehicleShare.AIR`: 0.8 ; `vehicleRoles`: `US_P51`: `fighter`, `GER_BF109_AI`/`US_P51_AI`: `fighter_ai`.
- Gate : PARA commandable rang 15 ou général ; AIR seulement si le camp tient une ville avec aérodrome (option).
progression.json :
- `trees` += `"para"`, `"pilot"`.
- unlocks para (rang/prix) : `US_OUTFIT_PARA` → tree `para` ; `US_M1A1_CARBINE` para r2 300 ; `US_THOMPSON` para r4 600 ; `GER_FG42` para r8 1500 (si dispo) ; `GER_MP40` para r2 300 ; `US_CHUTE_RESERVE` para r6 400 (voile de secours = 2e ouverture).
- vehicles : `US_P51` {pilot, r12, 5000} (RMS) ; `GER_STUKA` {pilot, r9, 4000} (RMS) ; `US_P51_AI`/`GER_BF109_AI` {soutien général, coût WF 200/sortie} ; `US_C47`/`GER_C47` non achetables (ressource d'équipe).
- `warFunds.teamCost` += `"AIR": 900` ; PARA reste 450.

## 8. Vérification + inspection des mods (2026-11-25, serveur test isolé port 2010, rien en prod)
| Mod | Version / taille / MAJ | Licence | Dépendances | Contenu vérifié (pak) |
|---|---|---|---|---|
| Grey Mans Chute Framework | 0.2.8 / 211 Mo / 2026-10-02 | APL-SA | aucune | `GMS_FreeFallComponent` (chute libre pilotable, roulis Q/E, **Espace = ouverture manuelle** après 1 s), canopées V2 (4 couleurs) + static line (`GMS_StaticLineComponent`), dégâts d'atterrissage, prefabs de test (`GMS_HaloParadropTest.et`…). Moddé `Character_Base.et` (ajoute le composant). |
| Prontos Parachutes | 1.0.2 / 20 Mo / 2026-08-18 | APL-SA | aucune | T-10 (sac + voile), `Pronto_ParachuteComponent`, slot de chargement ; moddé `Character_Base` + `DefaultPlayerController`. |
| Static Line Parachutes | 1.4.0 / 20 Mo / 2026-11-15 | APL-SA | **aucune** (n'utilise plus Prontos) | Fork MacPara_* (T-10, largage cargo) ; mêmes overrides Character_Base/PlayerController → **incompatible en pratique avec Prontos** (doublon). |
| JMParaAI | 1.0.6 / 52 Mo / 2026-10-17 | APL | aucune | Avions scriptés C-17/Il-112 (`JMPara_Plane`, `JMPara_JumpRun`), sacs RamAir/ronds, IA paras, action GM. Scripts réutilisables comme modèle (APL = dérivé autorisé). |
| C-47 Skytrain (CrackrDev) | 1.1.1 / 45 Mo / 2026-04-17 | APL | GM FX, You can sit, Op Overlord GMFX Pack | **NON TÉLÉCHARGEABLE** : backend « Addon has dependencies deleted from workshop » (GMFX Pack) → serveur refuse de démarrer. À écarter. |
| AeroplaneHavens C47 Dakota | 1.1.28 / 369 Mo / 2026-11-22 | **APL-ND** | USAAF Bomber Attire, Aviation-Toolkit | Prefab `AeroplaneHaven-C47.et` : **30 CargoCompartmentSlot + 2 pilotes**, vol JSBSim (`FGFDMExec`), `TK_AIPilotComponent` + autopilote (IA pilote possible), lumières cabine rouge/verte para, `C47GetOutAction` = sortie en vol par la porte cargo (sans limite 15 km/h, option « toujours paradrop »), marche dans l'allée. ND = utilisable en dépendance, pas de modification/repack. |
| Pingus P-51D / Bf109G | 1.0.66 / 2 Mo / 2026-06-19 | APL | **10 deps** (C-130J 434 Mo, WCS_Armaments 424 Mo, US/GER plane props, Pegasus, MFD, Propeller Flight Core, Grey Chute…) ≈ 1,6 Go | Vol cinématique IA (`CASF_AutopilotComponent`, `CASF_GunComponent`), prefabs `_Fly`/`_CAS` : ambiance/CAS seulement, non pilotable. |
| RMS WW2 WarBirds II | 1.0.8 / 133 Mo / 2026-11-07 | Custom | WCS_Armaments, RMS Fixed Wing Flight Core | Stuka + **Ju-88** (bombes, mitrailleurs MG-15) ; plus de P-51 dans cette version. Modèle de vol = RMS Flight Core (WIP). |

**Conflits** : FH Core moddé aussi `Character_Base.et` et `DefaultPlayerController.et` → les overrides se chaînent (composants ajoutés) ; à tester mais pas de collision de script détectée. Ne pas empiler Grey + Prontos + Static Line (3 systèmes parachute sur le même personnage).

**Test** : serveur dédié (FH Core + Grey Chute + AH C47 + JMParaAI, scénario vanilla Conflict) → **compilation scripts OK, 0 erreur SCRIPT, monde chargé** (warnings seulement). Test en vol (perso placé dans l'avion, saut, chute, ouverture) **non réalisé** : impossible sans client ni scénario de test dédié — à faire en Workbench (prefab `GMS_HaloParadropTest.et` + C47 AH en altitude).

**Recommandation mise à jour** : parachute = **Grey Mans Chute Framework** (seul). Avion = **AeroplaneHavens C47** (sièges cargo + sortie en vol + IA pilote) en dépendance, déplacé par notre script ou son autopilote ; CrackrDev écarté. Pingus trop lourd en dépendances ; JMParaAI = source d'inspiration script (pas en dépendance).

## 9. Implémentation (2026-11-25, F44_ParaDrop.c, mod 1.2.0 draft)
- Intégration **souple** : aucun lien gproj vers C47/Aviation-Toolkit/Grey Chute ; prefab C-47 `{A2B54744389B4105}` chargé par nom, pilote IA vanilla + waypoint Move → `TK_AIPilotComponent` orbite le centre du layout. Mods ajoutés au serveur WW2 (config.json + template).
- Test serveur isolé (port 2010, F44_Test) : C-47 spawné à 350 m AGL, pilote assis, point « AIR DROP » enregistré, bot assis en cargo (siège 2, 29 libres), avion en orbite ~1200 m autour du centre à 72 m/s.
- Reste à tester avec client : apparition du point dans le menu, spawn joueur en siège, saut porte cargo, chute libre + ouverture Espace, refus classe non-PARA. Catalogue : `US_PARA`/`GER_PARA` sans prefab → à remplir (sinon personne n'est PARA).
