package main

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
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
	a.GET("/battles/:id/live", getLive)
	a.GET("/players", leaderboard)
	a.GET("/players/:id", getPlayer)
	// auth
	a.POST("/auth/login", login)
	a.POST("/auth/logout", logout)
	a.GET("/auth/me", me)
	a.GET("/auth/config", func(c *gin.Context) { c.JSON(200, gin.H{"discord": discordConfigured()}) })
	a.GET("/auth/discord", discordStart)
	a.GET("/auth/discord/callback", discordCallback)
	a.POST("/me/faction", requireUser, setFaction)
	// soldiers, arsenal, loadouts
	a.GET("/store", getStore)
	a.GET("/me/soldiers", requireUser, listSoldiers)
	a.POST("/me/soldiers", requireUser, createSoldier)
	a.POST("/soldiers/:id/activate", requireUser, activateSoldier)
	a.GET("/soldiers/:id/ribbons", getRibbons)
	a.GET("/soldiers/:id/loadout", requireUser, getLoadout)
	a.PUT("/soldiers/:id/loadout", requireUser, putLoadout)
	a.POST("/soldiers/:id/buy", requireUser, buyItem)
	a.GET("/weapons/stats", getWeaponStats)
	a.GET("/vehicles/stats", getVehicleStats)
	a.GET("/unlocks", getUnlocks)
	a.POST("/unlocks/:key/buy", requireUser, buyUnlock)
	a.GET("/battles/:id/loadout/:identityId", requireServer, getBattleLoadout)
	// account linking
	a.POST("/link/request", requireServer, linkRequest)
	a.POST("/link/confirm", requireUser, linkConfirm)
	a.GET("/link/qr", linkQR)
	a.GET("/players/by-identity/:identityId", requireServer, playerByIdentity)
	// général
	a.GET("/teams/options", vehicleOptions)
	registerCampaignRoutes(a)
	g := a.Group("/teams", requireGeneral)
	g.POST("/:id/vehicle", configureVehicle)
	g.POST("", createTeam)
	g.POST("/:id/move", moveTeam)
	g.DELETE("/:id", dissolveTeam)
	g.POST("/:id/resupply", resupplyTeam)
	// serveur (orchestrateur / mod)
	s := a.Group("/battles/:id", requireServer)
	s.POST("/claim", claimBattle)
	s.POST("/heartbeat", heartbeat)
	s.POST("/result", postResult)
	s.POST("/live", postLive)
	// admin
	ad := a.Group("/admin", requireAdmin)
	ad.POST("/war/reset", resetWar)
	ad.GET("/ai", adminGetAI)
	ad.PUT("/ai", adminPutAI)
	ad.POST("/tick", func(c *gin.Context) { gameMu.Lock(); doTick(true); gameMu.Unlock(); c.JSON(200, gin.H{"ok": true}) })
	ad.POST("/arrive-now", arriveNow)
	ad.PUT("/cities/:id", editCity)
	ad.PUT("/factions/:code", editFaction)
	ad.GET("/players", adminPlayers)
	ad.POST("/players/:id/unlink", adminUnlink)
	ad.PUT("/players/:id", adminEditPlayer)
	ad.POST("/battles/:id/cancel", cancelBattle)

	r.NoRoute(serveStatic)
	return r
}

func serveStatic(c *gin.Context) {
	p := filepath.Clean(c.Request.URL.Path)
	if strings.HasPrefix(p, "/api/") {
		c.JSON(404, gin.H{"error": "not found"})
		return
	}
	f := filepath.Join(cfg.StaticDir, p)
	if st, err := os.Stat(f); err == nil && !st.IsDir() {
		c.Header("Cache-Control", staticCache(p))
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

// ---------- carte ----------

type TeamView struct {
	AssaultTeam
	TeamID    string         `json:"teamId"`
	VehList   []VehicleCount `json:"vehicles"`
	VehOpts   []VehicleOpt   `json:"vehicleOpts"`
	OwnerName string         `json:"ownerName"`
	BattleRef string         `json:"battleId"`
	*TeamStrength
}

func teamView(t *AssaultTeam, names map[uint]string, hideDest bool) TeamView {
	v := TeamView{AssaultTeam: *t, TeamID: t.TeamID(), VehList: t.GetVehicles(), VehOpts: t.GetVehicleOpts(), OwnerName: names[t.OwnerID]}
	if t.BattleID > 0 {
		v.BattleRef = "b_" + pad6(int(t.BattleID))
	}
	if !hideDest {
		st := teamStrength(t)
		v.TeamStrength = &st
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
		errJ(c, 404, "no war")
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
		errJ(c, 400, "invalid request")
		return
	}
	spec, ok := atSpecs[in.Type]
	if !ok {
		errJ(c, 400, "unknown assault team type")
		return
	}
	gameMu.Lock()
	defer gameMu.Unlock()
	w := currentWar()
	if w == nil || w.Status != "active" {
		errJ(c, 400, "no active war")
		return
	}
	if msg := teamCapReached(u, w.ID); msg != "" {
		errJ(c, 400, msg)
		return
	}
	fac := u.Faction
	if u.Role == "admin" && in.Faction != "" {
		fac = in.Faction
	}
	city := getCity(w.ID, in.CityID)
	if city == nil || city.Owner != fac {
		errJ(c, 400, "the city must belong to your faction")
		return
	}
	if !city.IsFactory && !city.IsCapital {
		errJ(c, 400, "teams can only be raised in a capital or factory city")
		return
	}
	if activeBattleAt(w.ID, city.ID) != nil {
		errJ(c, 400, "city is under attack")
		return
	}
	var f Faction
	if db.Where("war_id = ? AND code = ?", w.ID, fac).First(&f).Error != nil || !payTeamCost(&f, city, spec.Cost) {
		errJ(c, 400, "not enough resources")
		return
	}
	t := newTeam(w.ID, in.Type, fac, u.ID, city.ID)
	db.Create(&t)
	t.DisplayName = teamDisplayName(t.Faction, t.Type, t.ID)
	db.Model(&AssaultTeam{}).Where("id = ?", t.ID).Update("display_name", t.DisplayName)
	logEvent(w.ID, "order", fac, fmt.Sprintf("%s raises %s (%s) at %s", u.Name, t.DisplayName, spec.Label, city.Name))
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
		errJ(c, 404, "assault team not found")
		return
	}
	if !ownsTeam(u, &t) {
		errJ(c, 403, "this assault team is not yours")
		return
	}
	if t.Status != "idle" {
		errJ(c, 400, "assault team not available ("+t.Status+")")
		return
	}
	rd := roadBetween(w.ID, t.CityID, in.DestCityID)
	if rd == nil {
		errJ(c, 400, "no direct road to this city")
		return
	}
	if dc := getCity(w.ID, in.DestCityID); dc != nil && dc.Owner != t.Faction {
		if st := teamStrength(&t); !st.CanAttack {
			errJ(c, 400, fmt.Sprintf("assault team too weak to attack (%d%% < %d%%): resupply it first", st.Pct, teamRules.MinAttackPct))
			return
		}
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
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s orders %s to march on %s (arrival %s UTC)", u.Name, t.Label(), dest.Name, arr.Format("15:04")))
	c.JSON(200, gin.H{"ok": true, "arrivalAt": arr})
}

func dissolveTeam(c *gin.Context) {
	u := user(c)
	gameMu.Lock()
	defer gameMu.Unlock()
	w := currentWar()
	var t AssaultTeam
	if w == nil || db.Where("war_id = ?", w.ID).First(&t, c.Param("id")).Error != nil {
		errJ(c, 404, "assault team not found")
		return
	}
	if !ownsTeam(u, &t) {
		errJ(c, 403, "this assault team is not yours")
		return
	}
	if t.Status != "idle" {
		errJ(c, 400, "only an idle assault team can be disbanded")
		return
	}
	spec := atSpecs[t.Type]
	refund := spec.Cost / 2 * t.Manpower / max(spec.Manpower, 1)
	db.Model(&Faction{}).Where("war_id = ? AND code = ?", w.ID, t.Faction).Update("resources", gormExprAdd(refund))
	db.Model(&t).Update("status", "dissolved")
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s disbands %s (+%d resources)", u.Name, t.Label(), refund))
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
		errJ(c, 404, "battle not found")
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
		"serverInstance": b.ServerInstance, "createdAt": b.CreatedAt, "claimedAt": b.ClaimedAt, "heartbeatAt": b.HeartbeatAt, "finishedAt": b.FinishedAt,
		"readyAt": battleReadyAt(b), "autoResolveAt": battleAutoResolveAt(b)}
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
		errJ(c, 409, "battle already claimed ("+b.Status+")")
		return
	}
	now := time.Now().UTC()
	if !battleClaimable(b, now) {
		errJ(c, 409, "battle in prep window until "+battleReadyAt(b).Format(time.RFC3339))
		return
	}
	res := db.Model(&Battle{}).Where("id = ? AND status = ?", b.ID, "pending").
		Updates(map[string]interface{}{"status": "starting", "server_instance": in.ServerInstance, "claimed_at": now, "heartbeat_at": now})
	if res.RowsAffected == 0 {
		errJ(c, 409, "battle already claimed")
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
		errJ(c, 409, "battle not active ("+b.Status+")")
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
		errJ(c, 400, "invalid JSON: "+err.Error())
		return
	}
	gameMu.Lock()
	defer gameMu.Unlock()
	b := loadBattle(c)
	if b == nil {
		return
	}
	if r.BattleID != "" && parseBattleID(r.BattleID) != b.ID {
		errJ(c, 400, "battleId mismatch")
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
		errJ(c, 404, "player not found")
		return
	}
	var hist []PlayerBattle
	db.Where("player_id = ?", p.ID).Order("id desc").Limit(50).Find(&hist)
	linked := p.IdentityID != ""
	if linked {
		p.IdentityID = "linked"
	}
	c.JSON(200, gin.H{"player": p, "rank": rankFor(p.XP), "history": hist, "linked": linked})
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
		errJ(c, 404, "city not found")
		return
	}
	var in map[string]interface{}
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "invalid request")
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
	logEvent(w.ID, "admin", "", "City edited by admin: "+ci.Name)
	db.Where("war_id = ? AND id = ?", w.ID, ci.ID).First(&ci)
	c.JSON(200, ci)
}

func editFaction(c *gin.Context) {
	w := currentWar()
	var in struct {
		Resources int `json:"resources"`
	}
	if w == nil || c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "invalid request")
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

func adminEditPlayer(c *gin.Context) {
	var p Player
	if db.First(&p, c.Param("id")).Error != nil {
		errJ(c, 404, "player not found")
		return
	}
	var in struct {
		Role, Faction *string
		XP, Credits   *int
	}
	_ = c.ShouldBindJSON(&in)
	upd := map[string]interface{}{}
	if in.Role != nil {
		if *in.Role != "admin" && *in.Role != "general" && *in.Role != "soldier" {
			errJ(c, 400, "invalid role")
			return
		}
		upd["role"] = *in.Role
	}
	if in.Faction != nil {
		upd["faction"] = *in.Faction
	}
	if in.XP != nil {
		upd["xp"] = *in.XP
	}
	if in.Credits != nil {
		upd["credits"] = *in.Credits
	}
	db.Model(&p).Updates(upd)
	if w := currentWar(); w != nil {
		logEvent(w.ID, "admin", "", fmt.Sprintf("Admin %s updated player %s", user(c).Name, p.Name))
	}
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
		errJ(c, 400, "battle already closed")
		return
	}
	db.Model(&AssaultTeam{}).Where("battle_id = ?", b.ID).Updates(map[string]interface{}{"status": "idle", "battle_id": 0, "dest_city_id": ""})
	db.Model(b).Update("status", "cancelled")
	logEvent(b.WarID, "admin", "", "Battle cancelled: "+b.BattleID())
	c.JSON(200, gin.H{"ok": true})
}

// staticCache: images/fonts are cached a week (revalidated after via Last-Modified),
// css/js an hour, pages always revalidated.
func staticCache(p string) string {
	switch strings.ToLower(filepath.Ext(p)) {
	case ".png", ".jpg", ".jpeg", ".webp", ".gif", ".svg", ".ico", ".woff", ".woff2", ".ttf":
		return "public, max-age=604800, stale-while-revalidate=86400"
	case ".css", ".js", ".json":
		return "public, max-age=3600"
	default:
		return "no-cache"
	}
}
