# Arma & Heroes — website & API (V1)

Go (Gin + GORM) + MariaDB, vanilla JS front-end in `static/` (mounted as a volume: editable without rebuild), mobile-first.
BattleConfig / BattleResult contracts: see `../PLAN.md` §3-4.

## Environment variables (`.env`, see `.env.example`)
| Variable | Purpose |
|---|---|
| `F44_SERVER_KEY` | Bearer key for the orchestrator / mod (claim, heartbeat, result, link/request, by-identity) |
| `F44_DB_PASSWORD`, `F44_DB_ROOT_PASSWORD` | MariaDB (used by compose) |
| `F44_ADMIN_USER`, `F44_ADMIN_PASSWORD` | bootstrap admin, the only account with a local password |
| `F44_PUBLIC_URL` | public URL (link/QR URLs) |
| `F44_TICK_SEC` | game loop period (arrivals), default 30 s |
| `F44_INCOME_MIN` | supply interval, default 60 min |
| `F44_TRAVEL_MIN_PER_KM` | travel time, default 3 min/km (MOT/REC x2/3, ARM x5/4) |
| `DISCORD_CLIENT_ID`, `DISCORD_CLIENT_SECRET`, `DISCORD_REDIRECT_URI` | Discord OAuth2 (scope `identify`). Redirect: `https://heroes.collectifxxl.fr/api/v1/auth/discord/callback`. Until all three are set, Discord login returns HTTP 503 with a clear message. |

## Authentication
- Players: Discord OAuth2 only (anti-CSRF `state` cookie, session cookie HttpOnly + Secure + SameSite=Lax, 30 days).
- Admin: local password login (`POST /auth/login`) restricted to `F44_ADMIN_USER`. Admins can promote any Discord account to `general` or `admin`.
- Players can never type their Reforger identityId. Linking:
  1. The mod calls `POST /link/request` (server key) `{identityId, playerName}` and gets `{code:"AH-XXXXXX", expiresAt, alreadyLinked, url, qrUrl}` (or `{alreadyLinked:true, discordName, faction, rank}`).
  2. It shows the code and the QR image `GET /link/qr?code=AH-XXXXXX[&size=512]` (PNG of `https://heroes.collectifxxl.fr/link?code=...`).
  3. The player opens `/link?code=...` on a phone, logs in with Discord (the code is kept through the OAuth `next` parameter) and confirms (`POST /link/confirm`).
  - Codes: 6 chars from `ABCDEFGHJKLMNPQRSTUVWXYZ23456789`, 10 min TTL, single use, stored as SHA-256 hashes; input is case/dash/space insensitive.
  - Rate limit 5 attempts / 10 min per account and per IP. Refused if the identity is linked elsewhere or the account is already linked. Unlink: admin only, logged.
- `GET /players/by-identity/{identityId}` (server key) -> `{linked, discordName, faction, rank}` so the mod can force the faction.
- Battle results only credit XP/stats to linked identities (others still count in team losses).

## API `/api/v1`
- Public: `GET /map`, `/events`, `/catalog`, `/battles?status=pending[,running...]`, `/battles/{id}` (exact BattleConfig §3), `/battles/{id}/detail`, `/players`, `/players/{id}`, `/link/qr`
- Session: `POST /auth/logout`, `GET /auth/me`, `GET /auth/discord?next=/path`, `POST /me/faction`, `POST /link/confirm {code}`
- General: `POST /teams {type,cityId}`, `POST /teams/{n}/move {destCityId}`, `DELETE /teams/{n}`
- Server key: `POST /battles/{id}/claim {serverInstance}`, `/heartbeat` (free JSON body = live stats), `/result` (BattleResult §4), `POST /link/request`, `GET /players/by-identity/{id}`
- Admin: `POST /admin/war/reset`, `/admin/tick`, `/admin/arrive-now`, `PUT /admin/cities/{id}`, `PUT /admin/factions/{US|GER}`, `GET /admin/players`, `PUT /admin/players/{id} {role,faction,xp,credits}`, `POST /admin/players/{id}/unlink`, `POST /admin/battles/{id}/cancel`

AI bots: each side of BattleConfig carries `"ai": {"enabled", "maxAlive", "perPlayerScale"}`; defaults and per-type `maxAlive` (INF 24, PARA 16, ARM 8, REC 8, MOT 20; the side takes the highest of its teams) are editable via `GET/PUT /admin/ai`. In BattleResult, `losses.manpower` includes bots; optional `aiKills` / `aiDeaths` per side are displayed only.

Battle `{id}`: `b_000123` or `123`.

## Game rules
- Income: 10/city, +30 factory, +20 capital. Teams can only be raised in a capital or factory city.
- Arriving on a friendly city: the team garrisons it (merges with a team of the same type/general). Enemy city: `pending` battle (defenders = teams present, otherwise garrison). Existing battle: reinforcement.
- Result: losses applied, team at 0 destroyed; winning attacker takes the city, beaten defender retreats to an adjacent friendly city; beaten attacker stays home. XP (+100 to the winner), credits = XP/10.
- Victory: both capitals or >= 70 % of cities.

## Deployment (VM)
```
scp -r web/* web/.env ben@192.168.1.34:~/front44/
ssh ben@192.168.1.34 "cd ~/front44 && docker compose up --build -d 2>&1 | tail -30"
```
External network: `cfa-net` (Caddy). Caddyfile block:
```
heroes.collectifxxl.fr {
	reverse_proxy f44-app:8080
}
```
then `docker exec caddy caddy reload --config /etc/caddy/Caddyfile --adapter caddyfile`.
