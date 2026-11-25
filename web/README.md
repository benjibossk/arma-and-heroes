# Arma & Heroes — site web & API (V1)

Go (Gin + GORM) + MariaDB, front vanilla JS dans `static/` (servi en volume, modifiable sans rebuild).
Contrats BattleConfig/BattleResult : voir `../PLAN.md` §3-4.

## Variables d'environnement (`.env`, cf. `.env.example`)
| Variable | Rôle |
|---|---|
| `F44_SERVER_KEY` | clé Bearer de l'orchestrateur / du mod (claim, heartbeat, result) |
| `F44_DB_PASSWORD`, `F44_DB_ROOT_PASSWORD` | MariaDB (utilisés par compose) |
| `F44_ADMIN_USER`, `F44_ADMIN_PASSWORD` | compte admin créé au 1er démarrage s'il n'existe pas |
| `F44_PUBLIC_URL` | URL publique (redirect OAuth, cookie Secure) |
| `F44_TICK_SEC` | période de la boucle (arrivées), défaut 30 s |
| `F44_INCOME_MIN` | intervalle de ravitaillement, défaut 60 min |
| `F44_TRAVEL_MIN_PER_KM` | durée de trajet, défaut 3 min/km (MOT/REC ×2/3, ARM ×5/4) |
| `DISCORD_CLIENT_ID/SECRET` | optionnel : active « Connexion via Discord » (redirect `<URL>/api/v1/auth/discord/callback`) |

## API `/api/v1`
- Public : `GET /map`, `GET /events`, `GET /catalog`, `GET /battles?status=pending[,running…]`, `GET /battles/{id}` (BattleConfig §3 exact), `GET /battles/{id}/detail`, `GET /players`, `GET /players/{id}`
- Auth : `POST /auth/login {name,password}`, `POST /auth/logout`, `GET /auth/me`, `GET /auth/discord`; `POST /me/faction|identity|password`
- Général : `POST /teams {type,cityId}`, `POST /teams/{n}/move {destCityId}`, `DELETE /teams/{n}`
- Serveur (Bearer `F44_SERVER_KEY`) : `POST /battles/{id}/claim {serverInstance}` (renvoie la config), `POST /battles/{id}/heartbeat` (corps JSON libre = stats live), `POST /battles/{id}/result` (BattleResult §4)
- Admin : `POST /admin/war/reset`, `/admin/tick`, `/admin/arrive-now`, `PUT /admin/cities/{id}`, `PUT /admin/factions/{US|GER}`, `GET|POST /admin/players`, `PUT /admin/players/{id}`, `POST /admin/battles/{id}/cancel`

`{id}` de bataille : `b_000123` ou `123`.

## Règles implémentées
- Revenu : 10/ville, +30 usine, +20 capitale. Achat d'AT uniquement en capitale/usine.
- Arrivée sur ville alliée : stationne (fusion avec une AT du même type/général). Ville ennemie : bataille `pending` (défenseurs = AT présentes, sinon garnison). Bataille déjà ouverte : renfort.
- Résultat : pertes appliquées, AT à 0 détruite ; attaquant vainqueur prend la ville, défenseur battu recule vers une ville alliée adjacente ; attaquant battu reste chez lui. XP (+100 au vainqueur), crédits = XP/10, joueurs créés par identityId.
- Victoire : les deux capitales ou ≥ 70 % des villes.

## Déploiement (VM)
```
scp -r web/* web/.env ben@192.168.1.34:~/front44/
ssh ben@192.168.1.34 "cd ~/front44 && docker compose up --build -d"
```
Réseau externe : `cfa-net` (Caddy). Bloc Caddyfile :
```
heroes.collectifxxl.fr {
	reverse_proxy f44-app:8080
}
```
puis `docker exec caddy caddy reload --config /etc/caddy/Caddyfile --adapter caddyfile`.
