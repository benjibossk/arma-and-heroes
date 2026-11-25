package main

import (
	"crypto/rand"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"math/big"
	"net/http"
	"net/url"
	"regexp"
	"strings"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
	qrcode "github.com/skip2/go-qrcode"
	"golang.org/x/crypto/bcrypt"
)

// ---------- sessions & roles ----------

func loadUser(c *gin.Context) {
	if tok, _ := c.Cookie("f44_session"); tok != "" {
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
		errJ(c, 401, "login required")
	}
}

func requireGeneral(c *gin.Context) {
	if u := user(c); u == nil || !u.IsGeneral() {
		errJ(c, 403, "generals only")
	}
}

func requireAdmin(c *gin.Context) {
	if u := user(c); u == nil || u.Role != "admin" {
		errJ(c, 403, "admins only")
	}
}

func requireServer(c *gin.Context) {
	if cfg.ServerKey == "" || c.GetHeader("Authorization") != "Bearer "+cfg.ServerKey {
		errJ(c, 401, "invalid server key")
	}
}

func setCookie(c *gin.Context, name, val string, maxAge int) {
	http.SetCookie(c.Writer, &http.Cookie{Name: name, Value: val, Path: "/", MaxAge: maxAge, HttpOnly: true, Secure: true, SameSite: http.SameSiteLaxMode})
}

func startSession(c *gin.Context, p *Player) {
	tok := randToken(24)
	db.Create(&Session{Token: tok, PlayerID: p.ID, Expires: time.Now().UTC().Add(30 * 24 * time.Hour)})
	setCookie(c, "f44_session", tok, 30*24*3600)
}

// login : local password login, restricted to the bootstrap admin account
func login(c *gin.Context) {
	var in struct{ Name, Password string }
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "invalid request")
		return
	}
	var p Player
	if in.Name != cfg.AdminUser || db.Where("name = ? AND role = ?", in.Name, "admin").First(&p).Error != nil || p.PassHash == "" ||
		bcrypt.CompareHashAndPassword([]byte(p.PassHash), []byte(in.Password)) != nil {
		errJ(c, 401, "invalid credentials")
		return
	}
	startSession(c, &p)
	c.JSON(200, p)
}

func logout(c *gin.Context) {
	if tok, _ := c.Cookie("f44_session"); tok != "" {
		db.Where("token = ?", tok).Delete(&Session{})
	}
	setCookie(c, "f44_session", "", -1)
	c.JSON(200, gin.H{"ok": true})
}

func me(c *gin.Context) {
	u := user(c)
	if u == nil {
		c.JSON(200, gin.H{"user": nil})
		return
	}
	c.JSON(200, gin.H{"user": u, "rank": rankFor(u.XP), "isGeneral": u.IsGeneral(), "linked": u.IdentityID != ""})
}

func setFaction(c *gin.Context) {
	u := user(c)
	var in struct{ Faction string }
	_ = c.ShouldBindJSON(&in)
	if in.Faction != "US" && in.Faction != "GER" {
		errJ(c, 400, "invalid faction")
		return
	}
	if u.Faction != "" && u.Role != "admin" {
		errJ(c, 400, "faction already chosen (ask an admin)")
		return
	}
	db.Model(u).Update("faction", in.Faction)
	c.JSON(200, gin.H{"ok": true})
}

// ---------- Discord OAuth2 ----------

func discordConfigured() bool {
	return cfg.DiscordID != "" && cfg.DiscordSecret != "" && cfg.DiscordRedir != ""
}

const notConfigured = "Discord login is not configured on this server (DISCORD_CLIENT_ID / DISCORD_CLIENT_SECRET / DISCORD_REDIRECT_URI missing)"

// safeNext : only same-site relative paths
func safeNext(n string) string {
	if !strings.HasPrefix(n, "/") || strings.HasPrefix(n, "//") || strings.ContainsAny(n, "\\\r\n|") {
		return "/"
	}
	return n
}

func discordStart(c *gin.Context) {
	if !discordConfigured() {
		errJ(c, 503, notConfigured)
		return
	}
	st := randToken(16)
	setCookie(c, "f44_oauth", st+"|"+url.QueryEscape(safeNext(c.DefaultQuery("next", "/"))), 600)
	q := url.Values{"client_id": {cfg.DiscordID}, "redirect_uri": {cfg.DiscordRedir}, "response_type": {"code"}, "scope": {"identify"}, "state": {st}, "prompt": {"none"}}
	c.Redirect(302, "https://discord.com/oauth2/authorize?"+q.Encode())
}

func discordCallback(c *gin.Context) {
	if !discordConfigured() {
		errJ(c, 503, notConfigured)
		return
	}
	ck, _ := c.Cookie("f44_oauth")
	parts := strings.SplitN(ck, "|", 2)
	setCookie(c, "f44_oauth", "", -1)
	if len(parts) != 2 || parts[0] == "" || parts[0] != c.Query("state") {
		errJ(c, 400, "invalid OAuth state, please retry")
		return
	}
	next, _ := url.QueryUnescape(parts[1])
	next = safeNext(next)
	form := url.Values{"client_id": {cfg.DiscordID}, "client_secret": {cfg.DiscordSecret}, "grant_type": {"authorization_code"},
		"code": {c.Query("code")}, "redirect_uri": {cfg.DiscordRedir}}
	cl := &http.Client{Timeout: 10 * time.Second}
	resp, err := cl.PostForm("https://discord.com/api/oauth2/token", form)
	if err != nil {
		errJ(c, 502, "Discord unreachable")
		return
	}
	var tk struct {
		AccessToken string `json:"access_token"`
	}
	_ = json.NewDecoder(resp.Body).Decode(&tk)
	resp.Body.Close()
	if tk.AccessToken == "" {
		errJ(c, 401, "Discord OAuth failed")
		return
	}
	req, _ := http.NewRequest("GET", "https://discord.com/api/users/@me", nil)
	req.Header.Set("Authorization", "Bearer "+tk.AccessToken)
	resp, err = cl.Do(req)
	if err != nil {
		errJ(c, 502, "Discord unreachable")
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
		errJ(c, 401, "Discord profile not found")
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
	ensureBaseSoldiers(&p)
	startSession(c, &p)
	c.Redirect(302, next)
}

// ---------- game account linking ----------

const codeAlphabet = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789" // no 0/O/1/I
const codeLen = 6

var codeRe = regexp.MustCompile(`^[A-HJ-NP-Z2-9]{6}$`)

func hashCode(body string) string { h := sha256.Sum256([]byte("f44link:" + body)); return hex.EncodeToString(h[:]) }

// normCode : case-insensitive, ignores dashes/spaces and an optional AH prefix
func normCode(s string) string {
	s = strings.ToUpper(s)
	var b strings.Builder
	for _, r := range s {
		if (r >= 'A' && r <= 'Z') || (r >= '0' && r <= '9') {
			b.WriteRune(r)
		}
	}
	out := b.String()
	if len(out) == codeLen+2 && strings.HasPrefix(out, "AH") {
		out = out[2:]
	}
	return out
}

func newCodeBody() string {
	b := make([]byte, codeLen)
	for i := range b {
		n, _ := rand.Int(rand.Reader, big.NewInt(int64(len(codeAlphabet))))
		b[i] = codeAlphabet[n.Int64()]
	}
	return string(b)
}

func linkURL(code string) string {
	return strings.TrimRight(cfg.PublicURL, "/") + "/link?code=" + code
}

func linkRequest(c *gin.Context) {
	var in struct {
		IdentityID string `json:"identityId"`
		PlayerName string `json:"playerName"`
	}
	if c.ShouldBindJSON(&in) != nil || len(strings.TrimSpace(in.IdentityID)) < 8 || len(in.IdentityID) > 64 {
		errJ(c, 400, "identityId required")
		return
	}
	in.IdentityID = strings.TrimSpace(in.IdentityID)
	var p Player
	if db.Where("identity_id = ?", in.IdentityID).First(&p).Error == nil {
		c.JSON(200, gin.H{"alreadyLinked": true, "discordName": p.Name, "faction": p.Faction, "rank": rankFor(p.XP)})
		return
	}
	now := time.Now().UTC()
	// any previous unused code for this identity is invalidated
	db.Model(&LinkCode{}).Where("identity_id = ? AND used_at IS NULL", in.IdentityID).Update("expires_at", now)
	var body string
	for i := 0; i < 10; i++ {
		body = newCodeBody()
		lc := LinkCode{CodeHash: hashCode(body), IdentityID: in.IdentityID, PlayerName: in.PlayerName, ExpiresAt: now.Add(10 * time.Minute), CreatedAt: now}
		if db.Create(&lc).Error == nil {
			code := "AH-" + body
			c.JSON(200, gin.H{"code": code, "expiresAt": lc.ExpiresAt, "alreadyLinked": false, "url": linkURL(code),
				"qrUrl": strings.TrimRight(cfg.PublicURL, "/") + "/api/v1/link/qr?code=" + code})
			return
		}
	}
	errJ(c, 500, "could not generate a code")
}

// rate limiting: 5 attempts / 10 min per account and per IP
var (
	rlMu   sync.Mutex
	rlHits = map[string][]time.Time{}
)

func rateHit(key string) bool {
	rlMu.Lock()
	defer rlMu.Unlock()
	now := time.Now()
	kept := rlHits[key][:0]
	for _, t := range rlHits[key] {
		if now.Sub(t) < 10*time.Minute {
			kept = append(kept, t)
		}
	}
	if len(kept) >= 5 {
		rlHits[key] = kept
		return false
	}
	rlHits[key] = append(kept, now)
	return true
}

func linkConfirm(c *gin.Context) {
	u := user(c)
	var in struct {
		Code string `json:"code"`
	}
	_ = c.ShouldBindJSON(&in)
	// both counters are always incremented
	okU, okIP := rateHit(fmt.Sprintf("u:%d", u.ID)), rateHit("ip:"+c.ClientIP())
	if !okU || !okIP {
		errJ(c, 429, "too many attempts, wait 10 minutes")
		return
	}
	if u.IdentityID != "" {
		errJ(c, 409, "your account is already linked to a game identity (ask an admin to unlink it)")
		return
	}
	body := normCode(in.Code)
	if !codeRe.MatchString(body) {
		errJ(c, 400, "invalid code format (expected AH-XXXXXX)")
		return
	}
	gameMu.Lock()
	defer gameMu.Unlock()
	now := time.Now().UTC()
	var lc LinkCode
	if db.Where("code_hash = ? AND used_at IS NULL AND expires_at > ?", hashCode(body), now).First(&lc).Error != nil {
		errJ(c, 404, "unknown or expired code: rejoin the server to get a new one")
		return
	}
	var other Player
	if db.Where("identity_id = ?", lc.IdentityID).First(&other).Error == nil {
		errJ(c, 409, "this game identity is already linked to another account")
		return
	}
	res := db.Model(&LinkCode{}).Where("id = ? AND used_at IS NULL", lc.ID).Update("used_at", now)
	if res.RowsAffected == 0 {
		errJ(c, 404, "code already used")
		return
	}
	db.Model(u).Update("identity_id", lc.IdentityID)
	if w := currentWar(); w != nil {
		logEvent(w.ID, "link", u.Faction, fmt.Sprintf("%s linked game profile %q", u.Name, lc.PlayerName))
	}
	c.JSON(200, gin.H{"ok": true, "playerName": lc.PlayerName})
}

func linkQR(c *gin.Context) {
	body := normCode(c.Query("code"))
	if !codeRe.MatchString(body) {
		errJ(c, 400, "invalid code")
		return
	}
	size := 256
	if c.Query("size") == "512" {
		size = 512
	}
	png, err := qrcode.Encode(linkURL("AH-"+body), qrcode.Medium, size)
	if err != nil {
		errJ(c, 500, "QR generation failed")
		return
	}
	c.Header("Cache-Control", "no-store")
	c.Data(200, "image/png", png)
}

func playerByIdentity(c *gin.Context) {
	var p Player
	if db.Where("identity_id = ?", c.Param("identityId")).First(&p).Error != nil {
		c.JSON(200, gin.H{"linked": false})
		return
	}
	c.JSON(200, gin.H{"linked": true, "discordName": p.Name, "faction": p.Faction, "rank": rankFor(p.XP), "role": p.Role})
}

func adminUnlink(c *gin.Context) {
	var p Player
	if db.First(&p, c.Param("id")).Error != nil {
		errJ(c, 404, "player not found")
		return
	}
	old := p.IdentityID
	db.Model(&p).Update("identity_id", "")
	if w := currentWar(); w != nil {
		logEvent(w.ID, "admin", "", fmt.Sprintf("Admin %s unlinked game identity %s from %s", user(c).Name, old, p.Name))
	}
	c.JSON(200, gin.H{"ok": true})
}
