# ARMA & HEROES — jeu persistant WW2 pour Arma Reforger

> Nom : "Heroes & Generals" est une marque de TLM Partners -> ne PAS l'utiliser (Workshop, site).
> Nom affiché : **Arma & Heroes**. Domaine : heroes.collectifxxl.fr. Préfixe code interne inchangé : `F44_`.

## 1. Boucle de jeu
1. **Carte de guerre** (site web) : villes reliées par routes de ravitaillement. Factions US / GER (URSS plus tard).
2. **Ressources** par faction, revenu par tick (1 tick = 1 h) selon villes tenues + villes "usine".
3. **Généraux** (joueurs promus) achètent des **Escouades d'assaut** (AT) avec les ressources :
   - `INF` (infanterie : fusilier, MG, médic, AT, officier), `PARA` (paras, spawn aérien),
   - `ARM` (blindés : 2-4 chars + équipages), `REC` (recon : sniper, jeep, éclaireurs), `MOT` (camions/half-tracks).
   - Une AT = type + `manpower` (tickets) + liste véhicules + niveau d'équipement.
4. Le général **déplace** l'AT vers une ville adjacente (temps de trajet). Arrivée sur une ville ennemie
   (ou contestée) => **Bataille** créée.
5. **Bataille** = une partie Reforger : carte/scénario associés à la ville, les AT engagées des 2 camps
   définissent spawns, classes, véhicules et tickets. Plusieurs AT par camp possibles (renforts).
6. Fin de bataille (tickets à 0, tous secteurs pris, ou timer) => résultat remonté : ville prise ou non,
   pertes réelles (manpower/véhicules consommés) appliquées aux AT, XP/crédits aux joueurs.
7. Victoire de guerre : capitale adverse prise ou 70 % des villes ; puis reset (nouvelle guerre).

## 2. Architecture
```
[Site web Arma & Heroes]  (Go + MariaDB, docker sur VM Debian, Caddy, heroes.collectifxxl.fr)
   - carte de guerre, généraux, AT, ordres, profils joueurs, historique
   - API REST  /api/v1/...
        |  (1) bataille créée -> file "pending"
[Orchestrateur]  (Python, sur le PC serveur Windows, D:\Serveur\ww2-heroes\orchestrator)
   - poll API, choisit une instance libre (pool ports 2002/2003/2004), écrit config.json
     (scenarioId de la ville + missionHeader {battleId, apiUrl, token}), lance ArmaReforgerServer, suit PID
        |  (2)
[Mod Reforger F44_Core]  (Workbench addon, D:\...\ArmaReforgerWorkbench\addons\F44_Core)
   - au démarrage : GET /api/v1/battles/{id}  -> BattleConfig (JSON ci-dessous)
   - configure : factions, spawns, arsenal par classe, véhicules (compteurs), tickets, secteurs
   - pendant : heartbeat + kills ; fin : POST /api/v1/battles/{id}/result ; puis shutdown
        |  (3)
   site applique le résultat -> carte de guerre mise à jour ; orchestrateur arrête l'instance.
```
Fallback hors-ligne : si l'API est injoignable, le mod lit `$profile:F44/battle.json`.

## 3. Contrat BattleConfig (GET /api/v1/battles/{id})
```json
{
  "battleId": "b_000123", "warId": 4, "cityId": "sainte-marie",
  "mode": "ASSAULT",              // ASSAULT (secteurs séquentiels) | SKIRMISH (secteurs libres)
  "scenario": "{GUID}Missions/F44_SainteMarie.conf",
  "timeLimitMin": 45,
  "attacker": "US", "defender": "GER",
  "sides": {
    "US":  { "teams": [ { "teamId": "at_17", "type": "INF", "manpower": 120, "equipLevel": 1,
                         "classes": ["RIFLEMAN","MG","MEDIC","AT","OFFICER"],
                         "vehicles": [ {"key":"US_JEEP","count":2} ] } ] },
    "GER": { "teams": [ { "teamId": "at_9",  "type": "ARM", "manpower": 60, "equipLevel": 1,
                         "classes": ["CREW","RIFLEMAN"],
                         "vehicles": [ {"key":"GER_SDKFZ222","count":2} ] } ] }
  },
  "playerUnlocks": { "<identityId>": ["US_THOMPSON"] },  // optionnel, V3
  "aiLimit": 48,                  // optionnel : budget IA global (min avec l'attribut serveur)
  // par camp, optionnels : "aiEnabled": true, "aiMaxAlive": 24, "aiPerPlayerScale": 1.0
  //   -> bots vivants visés = max(0, aiMaxAlive - joueursCamp * aiPerPlayerScale)
  "playerLoadouts": { "<identityId>": { "soldierId":"s_42", "class":"RIFLEMAN",
                      "items":["US_M1_GARAND"], "points":120 } }   // optionnel
}
```
- Retardataires : `GET /api/v1/battles/{id}/loadout/{identityId}` -> même objet que `playerLoadouts[id]`.
- Catalogue côté mod : clés `<SIDE>_AIGROUP` (prefab de groupe IA vide de la faction) ; champ
  optionnel `category` par arme (rifle|smg|lmg|sniper|at|explosive) pour les rubans.
- `tickets` d'un camp = somme des `manpower`. 1 mort = -1 ; 1 véhicule détruit = -coût véhicule.
- `classes` / `vehicles.key` -> table de correspondance **Catalog** (`catalog.json`, issu de l'inventaire)
  qui mappe une clé logique vers prefab(s) Reforger. Le site ne connaît que les clés.

## 4. Contrat BattleResult (POST /api/v1/battles/{id}/result, Bearer token)
```json
{ "battleId":"b_000123", "winner":"US", "reason":"SECTORS|TICKETS|TIME",
  "durationSec": 2140,
  "losses": { "at_17": {"manpower":64, "vehicles":{"US_JEEP":1}}, "at_9": {"manpower":60,"vehicles":{"GER_SDKFZ222":2}} },
  "players": [ {"identityId":"...","name":"...","side":"US","teamId":"at_17","kills":7,"deaths":3,"captures":2,"xp":340,
       "killsByCategory":{"rifle":4,"smg":1,"at":2}} ] }
```
- Les morts de bots comptent dans `losses.<teamId>.manpower` (1 ticket chacun).

## 5. Cartes / scénarios
Chaque ville = 1 scénario (`.conf`) sur un terrain, avec 3-5 secteurs (SCR_* capture areas custom F44)
+ zones de spawn attaquant/défenseur + spawns véhicules.
Terrains candidats (mods à jour) : WW2 Arland (6072D68D8DCD0C12), Montferrier1944 (6ADB27ABF0E4CC92),
Utah Beach (698C2A44E9972692, WIP), Everon/Arland vanilla habillés WW2.
V0 : 3 villes sur WW2 Arland (village, ferme/carrefour, ville).

## 6. Mods (stack serveur) — cf. D:\Serveur\arma-reforger-ww2\config.json
Finest Hour (8 modules, APL), Chungus Core/Bolt Guns, CIE Tracked/Thermal, WW2 Tank Core, M4A2 Sherman,
Northcom voices. Pas d'URSS (aucune faction WW2 soviétique maintenue).
**Permission** : demander l'accord des auteurs Finest Hour (salami._. / tracesnow) avant publication Workshop.
F44_Core ne redistribue AUCUN asset : uniquement dépendances + scripts + scénarios.

## 7. Phases
- **V0** Mod F44_Core : bataille jouable depuis `battle.json` local (spawns, classes, véhicules, tickets, secteurs ASSAULT), 1 carte.
- **V1** Site : carte de guerre, AT, ordres, ressources, création des batailles, API contrats §3-4, comptes (Discord/Steam ou login simple).
- **V2** Orchestrateur : pool d'instances, lancement/arrêt auto, remontée des résultats.
- **V3** Progression joueurs (grades, crédits, déblocages), plusieurs terrains, URSS si un mod apparaît.

## 8. Emplacements
- Plan & docs : `D:\Serveur\ww2-heroes\`  (inventaire : `inventory\`, catalogue : `catalog.json`)
- Site : `D:\Serveur\ww2-heroes\web` -> VM `~/front44`, conteneurs `f44-app`, `f44-db`, réseau Caddy
- Orchestrateur : `D:\Serveur\ww2-heroes\orchestrator`
- Mod : `C:\Users\benbo\Documents\My Games\ArmaReforgerWorkbench\addons\F44_Core`
- Instances serveur : `D:\Serveur\arma-reforger-ww2` (instance 1), suivantes `-ww2-2`, `-ww2-3`

