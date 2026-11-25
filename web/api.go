package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"net/url"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"golang.org/x/crypto/bcrypt"
)

func setupRouter() *gin.Engine {
	gin.SetMode(gin.ReleaseMode)
	r := gin.New()
	r.Use(gin.Recovery())
	r.GET("/healthz", func(c *gin.Context) { c.String(200, "ok") })
	a := r.Group("/api/v1")
	a.Use(loadUser)
	// public
	a.GET("/map", getMap)
	a.GET("/events", getEvents)
	a.GET("/catalog", func(c *gin.Context) { c.JSON(200, atSpecs) })
	a.GET("/battles", listBattles)
	a.GET("/battles/:id", getBattleConfig)
	a.GET("/battles/:id/detail", getBattleDetail)
	a.GET("/players", leaderboard)
	a.GET("/players/:id", getPlayer)
	// auth
	a.POST("/auth/login", login)
	a.POST("/auth/logout", logout)
	a.GET("/auth/me", me)
	a.GET("/auth/config", func(c *gin.Context) { c.JSON(200, gin.H{"discord": cfg.DiscordID != ""}) })
	a.GET("/auth/discord", discordStart)
	a.GET("/auth/discord/callback", discordCallback)
	u := a.Group("/me", requireUser)
	u.POST("/faction", setFaction)
	u.POST("/identity", setIdentity)
	u.POST("/password", setPassword)
	// général
	g := a.Group("/teams", requireGeneral)
	g.POST("", createTeam)
	g.POST("/:id/move", moveTeam)
	g.DELETE("/:id", dissolveTeam)
	// serveur (orchestrateur / mod)
	s := a.Group("/battles/:id", requireServer)
	s.POST("/claim", claimBattle)
	s.POST("/heartbeat", heartbeat)
	s.POST("/result", postResult)
	// admin
	ad := a.Group("/admin", requireAdmin)
	ad.POST("/war/reset", resetWar)
	ad.POST("/tick", func(c *gin.Context) { gameMu.Lock(); doTick(true); gameMu.Unlock(); c.JSON(200, gin.H{"ok": true}) })
	ad.POST("/arrive-now", arriveNow)
	ad.PUT("/cities/:id", editCity)
	ad.PUT("/factions/:code", editFaction)
	ad.GET("/players", adminPlayers)
	ad.POST("/players", adminCreatePlayer)
	ad.PUT("/players/:id", adminEditPlayer)
	ad.POST("/battles/:id/cancel", cancelBattle)

	r.NoRoute(serveStatic)
	return r
}

func serveStatic(c *gin.Context) {
	p := filepath.Clean(c.Request.URL.Path)
	if strings.HasPrefix(p, "/api/") {
		c.JSON(404, gin.H{"error": "introuvable"})
		return
	}
	f := filepath.Join(cfg.StaticDir, p)
	if st, err := os.Stat(f); err == nil && !st.IsDir() {
		c.File(f)
		return
	}
	if st, err := os.Stat(f + ".html"); err == nil && !st.IsDir() {
		c.File(f + ".html")
		return
	}
	c.File(filepath.Join(cfg.StaticDir, "index.html"))
}

func errJ(c *gin.Context, code int, msg string) { c.AbortWithStatusJSON(code, gin.H{"error": msg}) }

// ---------- auth ----------

func loadUser(c *gin.Context) {
	tok, _ := c.Cookie("f44_session")
	if tok != "" {
		var s Session
		if db.Where("token = ? AND expires > ?", tok, time.Now().UTC()).First(&s).Error == nil {
			var p Player
			if db.First(&p, s.PlayerID).Error == nil {
				c.Set("user", &p)
			}
		}
	}
	c.Next()
}

func user(c *gin.Context) *Player {
	if v, ok := c.Get("user"); ok {
		return v.(*Player)
	}
	return nil
}

func requireUser(c *gin.Context) {
	if user(c) == nil {
		errJ(c, 401, "connexion requise")
	}
}

func requireGeneral(c *gin.Context) {
	if u := user(c); u == nil || !u.IsGeneral() {
		errJ(c, 403, "réservé aux généraux")
	}
}

func requireAdmin(c *gin.Context) {
	if u := user(c); u == nil || u.Role != "admin" {
		errJ(c, 403, "réservé aux administrateurs")
	}
}

func requireServer(c *gin.Context) {
	h := c.GetHeader("Authorization")
	if cfg.ServerKey == "" || h != "Bearer "+cfg.ServerKey {
		errJ(c, 401, "clé serveur invalide")
	}
}

func startSession(c *gin.Context, p *Player) {
	tok := randToken(24)
	db.Create(&Session{Token: tok, PlayerID: p.ID, Expires: time.Now().UTC().Add(30 * 24 * time.Hour)})
	secure := strings.HasPrefix(cfg.PublicURL, "https")
	c.SetSameSite(http.SameSiteLaxMode)
	c.SetCookie("f44_session", tok, 30*24*3600, "/", "", secure, true)
}

func login(c *gin.Context) {
	var in struct{ Name, Password string }
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "requête invalide")
		return
	}
	var p Player
	if db.Where("name = ?", in.Name).First(&p).Error != nil || p.PassHash == "" ||
		bcrypt.CompareHashAndPassword([]byte(p.PassHash), []byte(in.Password)) != nil {
		errJ(c, 401, "identifiants incorrects")
		return
	}
	startSession(c, &p)
	c.JSON(200, p)
}

func logout(c *gin.Context) {
	if tok, _ := c.Cookie("f44_session"); tok != "" {
		db.Where("token = ?", tok).Delete(&Session{})
	}
	c.SetCookie("f44_session", "", -1, "/", "", false, true)
	c.JSON(200, gin.H{"ok": true})
}

func me(c *gin.Context) {
	u := user(c)
	if u == nil {
		c.JSON(200, gin.H{"user": nil})
		return
	}
	c.JSON(200, gin.H{"user": u, "rank": rankFor(u.XP), "isGeneral": u.IsGeneral()})
}

func setFaction(c *gin.Context) {
	u := user(c)
	var in struct{ Faction string }
	_ = c.ShouldBindJSON(&in)
	if in.Faction != "US" && in.Faction != "GER" {
		errJ(c, 400, "faction invalide")
		return
	}
	if u.Faction != "" && u.Role != "admin" {
		errJ(c, 400, "faction déjà choisie (contactez un admin)")
		return
	}
	db.Model(u).Update("faction", in.Faction)
	c.JSON(200, gin.H{"ok": true})
}

func setIdentity(c *gin.Context) {
	u := user(c)
	var in struct{ IdentityID string }
	_ = c.ShouldBindJSON(&in)
	in.IdentityID = strings.TrimSpace(in.IdentityID)
	if len(in.IdentityID) < 8 {
		errJ(c, 400, "identityId invalide")
		return
	}
	var other Player
	if db.Where("identity_id = ? AND id <> ?", in.IdentityID, u.ID).First(&other).Error == nil {
		if other.PassHash != "" || other.DiscordID != "" {
			errJ(c, 409, "identityId déjà lié à un autre compte")
			return
		}
		// compte créé automatiquement par un résultat de bataille : on fusionne ses stats
		db.Model(u).Updates(map[string]interface{}{"xp": u.XP + other.XP, "credits": u.Credits + other.Credits, "kills": u.Kills + other.Kills,
			"deaths": u.Deaths + other.Deaths, "captures": u.Captures + other.Captures, "battles": u.Battles + other.Battles})
		db.Model(&PlayerBattle{}).Where("player_id = ?", other.ID).Update("player_id", u.ID)
		db.Delete(&other)
	}
	db.Model(u).Update("identity_id", in.IdentityID)
	c.JSON(200, gin.H{"ok": true})
}

func setPassword(c *gin.Context) {
	u := user(c)
	var in struct{ Password string }
	_ = c.ShouldBindJSON(&in)
	if len(in.Password) < 6 {
		errJ(c, 400, "mot de passe trop court")
		return
	}
	h, _ := bcrypt.GenerateFromPassword([]byte(in.Password), bcrypt.DefaultCost)
	db.Model(u).Update("pass_hash", string(h))
	c.JSON(200, gin.H{"ok": true})
}

func discordRedirect() string { return strings.TrimRight(cfg.PublicURL, "/") + "/api/v1/auth/discord/callback" }

func discordStart(c *gin.Context) {
	if cfg.DiscordID == "" {
		errJ(c, 404, "Discord non configuré")
		return
	}
	st := randToken(12)
	c.SetCookie("f44_oauth", st, 600, "/", "", false, true)
	q := url.Values{"client_id": {cfg.DiscordID}, "redirect_uri": {discordRedirect()}, "response_type": {"code"}, "scope": {"identify"}, "state": {st}}
	c.Redirect(302, "https://discord.com/oauth2/authorize?"+q.Encode())
}

func discordCallback(c *gin.Context) {
	st, _ := c.Cookie("f44_oauth")
	if cfg.DiscordID == "" || st == "" || st != c.Query("state") {
		errJ(c, 400, "état OAuth invalide")
		return
	}
	form := url.Values{"client_id": {cfg.DiscordID}, "client_secret": {cfg.DiscordSecret}, "grant_type": {"authorization_code"},
		"code": {c.Query("code")}, "redirect_uri": {discordRedirect()}}
	resp, err := http.PostForm("https://discord.com/api/oauth2/token", form)
	if err != nil {
		errJ(c, 502, "Discord injoignable")
		return
	}
	var tk struct {
		AccessToken string `json:"access_token"`
	}
	_ = json.NewDecoder(resp.Body).Decode(&tk)
	resp.Body.Close()
	if tk.AccessToken == "" {
		errJ(c, 401, "échec OAuth Discord")
		return
	}
	req, _ := http.NewRequest("GET", "https://discord.com/api/users/@me", nil)
	req.Header.Set("Authorization", "Bearer "+tk.AccessToken)
	resp, err = http.DefaultClient.Do(req)
	if err != nil {
		errJ(c, 502, "Discord injoignable")
		return
	}
	var du struct {
		ID         string `json:"id"`
		Username   string `json:"username"`
		GlobalName string `json:"global_name"`
	}
	_ = json.NewDecoder(resp.Body).Decode(&du)
	resp.Body.Close()
	if du.ID == "" {
		errJ(c, 401, "profil Discord introuvable")
		return
	}
	var p Player
	if db.Where("discord_id = ?", du.ID).First(&p).Error != nil {
		name := du.GlobalName
		if name == "" {
			name = du.Username
		}
		var n int64
		db.Model(&Player{}).Where("name = ?", name).Count(&n)
		if n > 0 {
			name = name + "#" + du.ID[len(du.ID)-4:]
		}
		p = Player{Name: name, DiscordID: du.ID, Role: "soldier"}
		db.Create(&p)
	}
	startSession(c, &p)
	c.Redirect(302, "/profile.html")
}

// ---------- carte ----------

type TeamView struct {
	AssaultTeam
	TeamID    string         `json:"teamId"`
	VehList   []VehicleCount `json:"vehicles"`
	OwnerName string         `json:"ownerName"`
	BattleRef string         `json:"battleId"`
}

func teamView(t *AssaultTeam, names map[uint]string, hideDest bool) TeamView {
	v := TeamView{AssaultTeam: *t, TeamID: t.TeamID(), VehList: t.GetVehicles(), OwnerName: names[t.OwnerID]}
	if t.BattleID > 0 {
		v.BattleRef = "b_" + pad6(int(t.BattleID))
	}
	if hideDest {
		v.DestCityID, v.ArrivalAt, v.DepartAt = "", nil, nil
		v.OwnerName = ""
	}
	return v
}

func getMap(c *gin.Context) {
	w := currentWar()
	if w == nil {
		errJ(c, 404, "aucune guerre")
		return
	}
	var factions []Faction
	var cities []City
	var roads []Road
	var teams []AssaultTeam
	var battles []Battle
	db.Where("war_id = ?", w.ID).Find(&factions)
	db.Where("war_id = ?", w.ID).Find(&cities)
	db.Where("war_id = ?", w.ID).Find(&roads)
	db.Where("war_id = ? AND status IN ?", w.ID, []string{"idle", "moving", "in_battle"}).Find(&teams)
	db.Where("war_id = ? AND status IN ?", w.ID, []string{"pending", "starting", "running"}).Find(&battles)
	names := map[uint]string{}
	var ps []Player
	db.Select("id, name").Where("role IN ?", []string{"general", "admin"}).Find(&ps)
	for _, p := range ps {
		names[p.ID] = p.Name
	}
	u := user(c)
	viewer := ""
	if u != nil && u.Role != "admin" {
		viewer = u.Faction
	}
	// brouillard : une AT ennemie n'est visible que si elle est dans/à côté d'une ville tenue par le spectateur, ou en bataille
	owner := map[string]string{}
	for _, ci := range cities {
		owner[ci.ID] = ci.Owner
	}
	adj := map[string]bool{}
	for _, r := range roads {
		if owner[r.CityA] == viewer {
			adj[r.CityB] = true
		}
		if owner[r.CityB] == viewer {
			adj[r.CityA] = true
		}
	}
	tv := []TeamView{}
	for i := range teams {
		t := &teams[i]
		enemy := viewer != "" && t.Faction != viewer
		if enemy && t.Status != "in_battle" && owner[t.CityID] != viewer && !adj[t.CityID] {
			continue
		}
		tv = append(tv, teamView(t, names, enemy))
	}
	bv := []gin.H{}
	for i := range battles {
		bv = append(bv, gin.H{"battleId": battles[i].BattleID(), "cityId": battles[i].CityID, "status": battles[i].Status, "attacker": battles[i].Attacker, "defender": battles[i].Defender})
	}
	facts := gin.H{}
	for _, f := range factions {
		if viewer == "" || viewer == f.Code {
			facts[f.Code] = f
		} else {
			facts[f.Code] = gin.H{"code": f.Code, "name": f.Name}
		}
	}
	income := map[string]int{}
	for i := range cities {
		income[cities[i].Owner] += cityIncome(&cities[i])
	}
	next := w.LastIncomeAt.Add(time.Duration(cfg.IncomeMin) * time.Minute)
	c.JSON(200, gin.H{"war": w, "factions": facts, "income": income, "nextIncomeAt": next, "cities": cities, "roads": roads,
		"teams": tv, "battles": bv, "catalog": atSpecs, "travelMinPerKm": cfg.TravelMinPerKm, "serverTime": time.Now().UTC()})
}

func getEvents(c *gin.Context) {
	w := currentWar()
	if w == nil {
		c.JSON(200, []Event{})
		return
	}
	lim, _ := strconv.Atoi(c.DefaultQuery("limit", "50"))
	if lim <= 0 || lim > 500 {
		lim = 50
	}
	var ev []Event
	q := db.Where("war_id = ?", w.ID)
	u := user(c)
	// les ordres et revenus de l'adversaire ne sont pas publics
	if u == nil || u.Role != "admin" {
		f := ""
		if u != nil {
			f = u.Faction
		}
		q = q.Where("type NOT IN ? OR faction = ?", []string{"order", "income"}, f)
	}
	q.Order("id desc").Limit(lim).Find(&ev)
	c.JSON(200, ev)
}

// ---------- ordres ----------

func ownsTeam(u *Player, t *AssaultTeam) bool {
	return u.Role == "admin" || (t.Faction == u.Faction && (t.OwnerID == u.ID || t.OwnerID == 0))
}

func createTeam(c *gin.Context) {
	u := user(c)
	var in struct {
		Type    string `json:"type"`
		CityID  string `json:"cityId"`
		Faction string `json:"faction"`
	}
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "requête invalide")
		return
	}
	spec, ok := atSpecs[in.Type]
	if !ok {
		errJ(c, 400, "type d'AT inconnu")
		return
	}
	gameMu.Lock()
	defer gameMu.Unlock()
	w := currentWar()
	if w == nil || w.Status != "active" {
		errJ(c, 400, "aucune guerre active")
		return
	}
	fac := u.Faction
	if u.Role == "admin" && in.Faction != "" {
		fac = in.Faction
	}
	city := getCity(w.ID, in.CityID)
	if city == nil || city.Owner != fac {
		errJ(c, 400, "la ville doit appartenir à votre faction")
		return
	}
	if !city.IsFactory && !city.IsCapital {
		errJ(c, 400, "achat possible uniquement dans une capitale ou une ville-usine")
		return
	}
	if activeBattleAt(w.ID, city.ID) != nil {
		errJ(c, 400, "ville en cours de bataille")
		return
	}
	var f Faction
	if db.Where("war_id = ? AND code = ?", w.ID, fac).First(&f).Error != nil || f.Resources < spec.Cost {
		errJ(c, 400, "ressources insuffisantes")
		return
	}
	db.Model(&f).Update("resources", f.Resources-spec.Cost)
	t := newTeam(w.ID, in.Type, fac, u.ID, city.ID)
	db.Create(&t)
	logEvent(w.ID, "order", fac, fmt.Sprintf("%s lève %s (%s) à %s", u.Name, t.TeamID(), spec.Label, city.Name))
	c.JSON(200, teamView(&t, map[uint]string{u.ID: u.Name}, false))
}

func moveTeam(c *gin.Context) {
	u := user(c)
	var in struct {
		DestCityID string `json:"destCityId"`
	}
	_ = c.ShouldBindJSON(&in)
	gameMu.Lock()
	defer gameMu.Unlock()
	w := currentWar()
	var t AssaultTeam
	if w == nil || db.Where("war_id = ?", w.ID).First(&t, c.Param("id")).Error != nil {
		errJ(c, 404, "AT introuvable")
		return
	}
	if !ownsTeam(u, &t) {
		errJ(c, 403, "cette AT ne vous appartient pas")
		return
	}
	if t.Status != "idle" {
		errJ(c, 400, "l'AT n'est pas disponible ("+t.Status+")")
		return
	}
	rd := roadBetween(w.ID, t.CityID, in.DestCityID)
	if rd == nil {
		errJ(c, 400, "aucune route directe vers cette ville")
		return
	}
	dur := travelDuration(rd.Distance)
	if t.Type == "MOT" || t.Type == "REC" {
		dur = dur * 2 / 3
	} else if t.Type == "ARM" {
		dur = dur * 5 / 4
	}
	now := time.Now().UTC()
	arr := now.Add(dur)
	db.Model(&t).Updates(map[string]interface{}{"status": "moving", "dest_city_id": in.DestCityID, "depart_at": now, "arrival_at": arr})
	dest := getCity(w.ID, in.DestCityID)
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s ordonne à %s de rejoindre %s (arrivée %s UTC)", u.Name, t.TeamID(), dest.Name, arr.Format("15:04")))
	c.JSON(200, gin.H{"ok": true, "arrivalAt": arr})
}

func dissolveTeam(c *gin.Context) {
	u := user(c)
	gameMu.Lock()
	defer gameMu.Unlock()
	w := currentWar()
	var t AssaultTeam
	if w == nil || db.Where("war_id = ?", w.ID).First(&t, c.Param("id")).Error != nil {
		errJ(c, 404, "AT introuvable")
		return
	}
	if !ownsTeam(u, &t) {
		errJ(c, 403, "cette AT ne vous appartient pas")
		return
	}
	if t.Status != "idle" {
		errJ(c, 400, "seule une AT au repos peut être dissoute")
		return
	}
	spec := atSpecs[t.Type]
	refund := spec.Cost / 2 * t.Manpower / max(spec.Manpower, 1)
	db.Model(&Faction{}).Where("war_id = ? AND code = ?", w.ID, t.Faction).Update("resources", gormExprAdd(refund))
	db.Model(&t).Update("status", "dissolved")
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s dissout %s (+%d ressources)", u.Name, t.TeamID(), refund))
	c.JSON(200, gin.H{"ok": true, "refund": refund})
}

// ---------- batailles ----------

func parseBattleID(s string) uint {
	s = strings.TrimPrefix(s, "b_")
	n, _ := strconv.Atoi(s)
	return uint(n)
}

func loadBattle(c *gin.Context) *Battle {
	var b Battle
	id := parseBattleID(c.Param("id"))
	if id == 0 || db.First(&b, id).Error != nil {
		errJ(c, 404, "bataille introuvable")
		return nil
	}
	return &b
}

func battleSummary(b *Battle, names map[string]string) gin.H {
	sides := b.GetSides()
	mp := map[string]int{}
	for side, ids := range sides {
		for _, id := range ids {
			if t := teamByTeamID(id); t != nil {
				mp[side] += t.Manpower
			}
		}
	}
	if b.GarrisonMP > 0 {
		mp[b.Defender] += b.GarrisonMP
	}
	return gin.H{"battleId": b.BattleID(), "warId": b.WarID, "status": b.Status, "cityId": b.CityID, "cityName": names[b.CityID],
		"mode": b.Mode, "attacker": b.Attacker, "defender": b.Defender, "winner": b.Winner, "sides": sides, "tickets": mp,
		"serverInstance": b.ServerInstance, "createdAt": b.CreatedAt, "claimedAt": b.ClaimedAt, "heartbeatAt": b.HeartbeatAt, "finishedAt": b.FinishedAt}
}

func cityNames() map[string]string {
	var cs []City
	db.Select("id, name").Find(&cs)
	m := map[string]string{}
	for _, c := range cs {
		m[c.ID] = c.Name
	}
	return m
}

func listBattles(c *gin.Context) {
	q := db.Order("id desc")
	if st := c.Query("status"); st != "" {
		q = q.Where("status IN ?", strings.Split(st, ","))
	}
	lim, _ := strconv.Atoi(c.DefaultQuery("limit", "100"))
	if lim <= 0 || lim > 500 {
		lim = 100
	}
	var bs []Battle
	q.Limit(lim).Find(&bs)
	names := cityNames()
	out := []gin.H{}
	for i := range bs {
		out = append(out, battleSummary(&bs[i], names))
	}
	c.JSON(200, out)
}

func getBattleConfig(c *gin.Context) {
	if b := loadBattle(c); b != nil {
		c.JSON(200, buildConfig(b))
	}
}

func getBattleDetail(c *gin.Context) {
	b := loadBattle(c)
	if b == nil {
		return
	}
	out := battleSummary(b, cityNames())
	out["config"] = buildConfig(b)
	if b.Result != "" {
		out["result"] = json.RawMessage(b.Result)
	}
	if b.LastStats != "" {
		out["live"] = json.RawMessage(b.LastStats)
	}
	c.JSON(200, out)
}

func claimBattle(c *gin.Context) {
	var in struct {
		ServerInstance string `json:"serverInstance"`
	}
	_ = c.ShouldBindJSON(&in)
	gameMu.Lock()
	defer gameMu.Unlock()
	b := loadBattle(c)
	if b == nil {
		return
	}
	if b.Status != "pending" {
		errJ(c, 409, "bataille déjà prise ("+b.Status+")")
		return
	}
	now := time.Now().UTC()
	res := db.Model(&Battle{}).Where("id = ? AND status = ?", b.ID, "pending").
		Updates(map[string]interface{}{"status": "starting", "server_instance": in.ServerInstance, "claimed_at": now, "heartbeat_at": now})
	if res.RowsAffected == 0 {
		errJ(c, 409, "bataille déjà prise")
		return
	}
	b.Status, b.ServerInstance = "starting", in.ServerInstance
	c.JSON(200, buildConfig(b))
}

func heartbeat(c *gin.Context) {
	b := loadBattle(c)
	if b == nil {
		return
	}
	if b.Status != "starting" && b.Status != "running" {
		errJ(c, 409, "bataille non active ("+b.Status+")")
		return
	}
	body, _ := c.GetRawData()
	upd := map[string]interface{}{"status": "running", "heartbeat_at": time.Now().UTC()}
	if len(body) > 0 && json.Valid(body) && len(body) < 60000 {
		upd["last_stats"] = string(body)
	}
	db.Model(b).Updates(upd)
	c.JSON(200, gin.H{"ok": true, "status": "running"})
}

func postResult(c *gin.Context) {
	var r BattleResult
	if err := c.ShouldBindJSON(&r); err != nil {
		errJ(c, 400, "JSON invalide : "+err.Error())
		return
	}
	gameMu.Lock()
	defer gameMu.Unlock()
	b := loadBattle(c)
	if b == nil {
		return
	}
	if r.BattleID != "" && parseBattleID(r.BattleID) != b.ID {
		errJ(c, 400, "battleId ne correspond pas")
		return
	}
	if err := applyResult(b, &r); err != nil {
		errJ(c, 409, err.Error())
		return
	}
	c.JSON(200, gin.H{"ok": true})
}

// ---------- classement / profils ----------

func leaderboard(c *gin.Context) {
	var ps []Player
	q := db.Order("xp desc").Limit(100)
	if f := c.Query("faction"); f != "" {
		q = q.Where("faction = ?", f)
	}
	q.Find(&ps)
	out := []gin.H{}
	for _, p := range ps {
		out = append(out, gin.H{"id": p.ID, "name": p.Name, "faction": p.Faction, "role": p.Role, "xp": p.XP, "rank": rankFor(p.XP),
			"kills": p.Kills, "deaths": p.Deaths, "captures": p.Captures, "battles": p.Battles})
	}
	c.JSON(200, out)
}

func getPlayer(c *gin.Context) {
	var p Player
	if db.First(&p, c.Param("id")).Error != nil {
		errJ(c, 404, "joueur introuvable")
		return
	}
	var hist []PlayerBattle
	db.Where("player_id = ?", p.ID).Order("id desc").Limit(50).Find(&hist)
	c.JSON(200, gin.H{"player": p, "rank": rankFor(p.XP), "history": hist})
}

// ---------- admin ----------

func resetWar(c *gin.Context) {
	gameMu.Lock()
	defer gameMu.Unlock()
	if w := currentWar(); w != nil {
		now := time.Now().UTC()
		db.Model(&Battle{}).Where("war_id = ? AND status IN ?", w.ID, []string{"pending", "starting", "running"}).Update("status", "cancelled")
		db.Model(&AssaultTeam{}).Where("war_id = ? AND status IN ?", w.ID, []string{"idle", "moving", "in_battle"}).Update("status", "dissolved")
		if w.Status == "active" {
			db.Model(w).Updates(map[string]interface{}{"status": "finished", "ended_at": now})
		}
	}
	w := seedWar()
	c.JSON(200, w)
}

func arriveNow(c *gin.Context) {
	gameMu.Lock()
	defer gameMu.Unlock()
	now := time.Now().UTC()
	db.Model(&AssaultTeam{}).Where("status = ?", "moving").Update("arrival_at", now.Add(-time.Second))
	doTick(false)
	c.JSON(200, gin.H{"ok": true})
}

func editCity(c *gin.Context) {
	w := currentWar()
	var ci City
	if w == nil || db.Where("war_id = ? AND id = ?", w.ID, c.Param("id")).First(&ci).Error != nil {
		errJ(c, 404, "ville introuvable")
		return
	}
	var in map[string]interface{}
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "requête invalide")
		return
	}
	allowed := map[string]string{"name": "name", "x": "x", "y": "y", "owner": "owner", "isCapital": "is_capital", "isFactory": "is_factory",
		"scenarioId": "scenario_id", "terrain": "terrain", "mode": "mode", "garrison": "garrison"}
	upd := map[string]interface{}{}
	for k, v := range in {
		if col, ok := allowed[k]; ok {
			upd[col] = v
		}
	}
	gameMu.Lock()
	db.Model(&City{}).Where("war_id = ? AND id = ?", w.ID, ci.ID).Updates(upd)
	gameMu.Unlock()
	logEvent(w.ID, "admin", "", "Ville modifiée par l'admin : "+ci.Name)
	db.Where("war_id = ? AND id = ?", w.ID, ci.ID).First(&ci)
	c.JSON(200, ci)
}

func editFaction(c *gin.Context) {
	w := currentWar()
	var in struct {
		Resources int `json:"resources"`
	}
	if w == nil || c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "requête invalide")
		return
	}
	db.Model(&Faction{}).Where("war_id = ? AND code = ?", w.ID, c.Param("code")).Update("resources", in.Resources)
	c.JSON(200, gin.H{"ok": true})
}

func adminPlayers(c *gin.Context) {
	var ps []Player
	db.Order("name").Find(&ps)
	c.JSON(200, ps)
}

func adminCreatePlayer(c *gin.Context) {
	var in struct{ Name, Password, Role, Faction, IdentityID string }
	if c.ShouldBindJSON(&in) != nil || in.Name == "" || len(in.Password) < 6 {
		errJ(c, 400, "nom et mot de passe (6+) requis")
		return
	}
	if in.Role == "" {
		in.Role = "soldier"
	}
	h, _ := bcrypt.GenerateFromPassword([]byte(in.Password), bcrypt.DefaultCost)
	p := Player{Name: in.Name, PassHash: string(h), Role: in.Role, Faction: in.Faction, IdentityID: in.IdentityID}
	if err := db.Create(&p).Error; err != nil {
		errJ(c, 409, "nom déjà pris")
		return
	}
	c.JSON(200, p)
}

func adminEditPlayer(c *gin.Context) {
	var p Player
	if db.First(&p, c.Param("id")).Error != nil {
		errJ(c, 404, "joueur introuvable")
		return
	}
	var in struct {
		Role, Faction, IdentityID, Password *string
		XP, Credits                         *int
	}
	_ = c.ShouldBindJSON(&in)
	upd := map[string]interface{}{}
	if in.Role != nil {
		upd["role"] = *in.Role
	}
	if in.Faction != nil {
		upd["faction"] = *in.Faction
	}
	if in.IdentityID != nil {
		upd["identity_id"] = *in.IdentityID
	}
	if in.XP != nil {
		upd["xp"] = *in.XP
	}
	if in.Credits != nil {
		upd["credits"] = *in.Credits
	}
	if in.Password != nil && len(*in.Password) >= 6 {
		h, _ := bcrypt.GenerateFromPassword([]byte(*in.Password), bcrypt.DefaultCost)
		upd["pass_hash"] = string(h)
	}
	db.Model(&p).Updates(upd)
	c.JSON(200, gin.H{"ok": true})
}

func cancelBattle(c *gin.Context) {
	gameMu.Lock()
	defer gameMu.Unlock()
	b := loadBattle(c)
	if b == nil {
		return
	}
	if b.Status == "finished" || b.Status == "cancelled" {
		errJ(c, 400, "bataille déjà close")
		return
	}
	db.Model(&AssaultTeam{}).Where("battle_id = ?", b.ID).Updates(map[string]interface{}{"status": "idle", "battle_id": 0, "dest_city_id": ""})
	db.Model(b).Update("status", "cancelled")
	logEvent(b.WarID, "admin", "", "Bataille annulée : "+b.BattleID())
	c.JSON(200, gin.H{"ok": true})
}
