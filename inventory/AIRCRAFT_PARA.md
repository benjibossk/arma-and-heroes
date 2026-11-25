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
