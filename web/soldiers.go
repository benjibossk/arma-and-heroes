package main

import (
	_ "embed"
	"encoding/json"
	"fmt"
	"log"
	"sort"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
)

//go:embed catalog.json
var catalogRaw []byte

type Ribbon struct {
	Category string `json:"category"`
	Kills    int    `json:"kills"`
}

type Item struct {
	Key          string   `json:"key"`
	Prefab       *string  `json:"prefab"`
	Label        string   `json:"label"`
	Faction      string   `json:"faction"`
	Slot         string   `json:"slot"` // primary | secondary | equipment | outfit
	Category     string   `json:"category"`
	Classes      []string `json:"classes"`
	Price        int      `json:"price"`
	Weight       int      `json:"weight"`
	RankRequired *string  `json:"rankRequired"`
	Ribbon       *Ribbon  `json:"ribbon"`
	Starter      bool     `json:"starter"`
}

var items = map[string]*Item{}

func loadItems() {
	var c struct {
		Items map[string]*Item `json:"items"`
	}
	if err := json.Unmarshal(catalogRaw, &c); err != nil {
		log.Fatal("catalog.json: ", err)
	}
	for k, it := range c.Items {
		it.Key = k
		items[k] = it
	}
}

var soldierClasses = []string{"INF", "PARA", "REC", "CREW"}

func budgetFor(class string) int {
	if class == "INF" {
		return 10
	}
	return 6
}

func rankIndex(name string) int {
	for i, r := range ranks {
		if r.Name == name {
			return i
		}
	}
	return 0
}

func rankIdxFor(xp int) int { return rankIndex(rankFor(xp)) }

type Loadout struct {
	Primary   string   `json:"primary"`
	Secondary string   `json:"secondary"`
	Equipment []string `json:"equipment"` // up to 4
	Outfit    string   `json:"outfit"`
}

func (l Loadout) Keys() []string {
	out := []string{}
	for _, k := range append([]string{l.Primary, l.Secondary, l.Outfit}, l.Equipment...) {
		if k != "" {
			out = append(out, k)
		}
	}
	return out
}

type Soldier struct {
	ID        uint      `gorm:"primaryKey" json:"id"`
	PlayerID  uint      `gorm:"index" json:"playerId"`
	Faction   string    `gorm:"size:8" json:"faction"`
	Class     string    `gorm:"size:8" json:"class"`
	XP        int       `json:"xp"`
	Credits   int       `json:"credits"`
	Kills     int       `json:"kills"`
	Deaths    int       `json:"deaths"`
	CatKills  string    `gorm:"type:text" json:"-"` // {"rifle":12,...}
	Loadout   string    `gorm:"type:text" json:"-"`
	CreatedAt time.Time `json:"createdAt"`
}

type SoldierItem struct {
	ID        uint   `gorm:"primaryKey"`
	SoldierID uint   `gorm:"uniqueIndex:ux_soldier_item"`
	ItemKey   string `gorm:"size:64;uniqueIndex:ux_soldier_item"`
	CreatedAt time.Time
}

func (s *Soldier) GetCatKills() map[string]int {
	m := map[string]int{}
	_ = json.Unmarshal([]byte(s.CatKills), &m)
	return m
}

func (s *Soldier) GetLoadout() Loadout {
	var l Loadout
	_ = json.Unmarshal([]byte(s.Loadout), &l)
	if l.Equipment == nil {
		l.Equipment = []string{}
	}
	return l
}

func ownedKeys(sid uint) map[string]bool {
	var si []SoldierItem
	db.Where("soldier_id = ?", sid).Find(&si)
	m := map[string]bool{}
	for _, x := range si {
		m[x.ItemKey] = true
	}
	return m
}

func classOK(it *Item, class string) bool {
	if len(it.Classes) == 0 {
		return true
	}
	for _, c := range it.Classes {
		if c == class {
			return true
		}
	}
	return false
}

func itemFits(it *Item, s *Soldier) bool {
	return (it.Faction == s.Faction || it.Faction == "ANY") && classOK(it, s.Class)
}

// unlockState : "" if purchasable, else the reason
func unlockState(it *Item, s *Soldier) string {
	if !itemFits(it, s) {
		return "not available for this class/faction"
	}
	if it.RankRequired != nil && rankIdxFor(s.XP) < rankIndex(*it.RankRequired) {
		return "requires rank " + *it.RankRequired
	}
	if it.Ribbon != nil && s.GetCatKills()[it.Ribbon.Category] < it.Ribbon.Kills {
		return fmt.Sprintf("requires ribbon: %d %s kills", it.Ribbon.Kills, it.Ribbon.Category)
	}
	return ""
}

// grantStarters : give free starter items and a default loadout
func grantStarters(s *Soldier) {
	l := Loadout{Equipment: []string{}}
	keys := make([]string, 0, len(items))
	for k := range items {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	for _, k := range keys {
		it := items[k]
		if !it.Starter || !itemFits(it, s) {
			continue
		}
		db.Create(&SoldierItem{SoldierID: s.ID, ItemKey: k, CreatedAt: time.Now().UTC()})
		switch it.Slot {
		case "primary":
			if l.Primary == "" {
				l.Primary = k
			}
		case "secondary":
			if l.Secondary == "" {
				l.Secondary = k
			}
		case "outfit":
			if l.Outfit == "" {
				l.Outfit = k
			}
		}
	}
	if l.Primary == "" { // e.g. CREW: cheapest fitting primary for free
		best := ""
		for _, k := range keys {
			if it := items[k]; it.Slot == "primary" && itemFits(it, s) && it.RankRequired == nil && (best == "" || it.Price < items[best].Price) {
				best = k
			}
		}
		if best != "" {
			db.Create(&SoldierItem{SoldierID: s.ID, ItemKey: best, CreatedAt: time.Now().UTC()})
			l.Primary = best
		}
	}
	b, _ := json.Marshal(l)
	db.Model(s).Update("loadout", string(b))
	s.Loadout = string(b)
}

func validateLoadout(s *Soldier, l Loadout) (int, string) {
	owned := ownedKeys(s.ID)
	if len(l.Equipment) > 4 {
		return 0, "at most 4 equipment slots"
	}
	check := func(k, slot string) string {
		if k == "" {
			return ""
		}
		it := items[k]
		if it == nil {
			return "unknown item " + k
		}
		if it.Slot != slot {
			return k + " does not go in the " + slot + " slot"
		}
		if !owned[k] {
			return k + " is not owned"
		}
		if !itemFits(it, s) {
			return k + " is not compatible with this class/faction"
		}
		return ""
	}
	for _, p := range [][2]string{{l.Primary, "primary"}, {l.Secondary, "secondary"}, {l.Outfit, "outfit"}} {
		if e := check(p[0], p[1]); e != "" {
			return 0, e
		}
	}
	for _, k := range l.Equipment {
		if e := check(k, "equipment"); e != "" {
			return 0, e
		}
	}
	pts := 0
	for _, k := range l.Keys() {
		pts += items[k].Weight
	}
	if pts > budgetFor(s.Class) {
		return pts, fmt.Sprintf("loadout uses %d points, budget is %d", pts, budgetFor(s.Class))
	}
	return pts, ""
}

func loadoutPoints(l Loadout) int {
	p := 0
	for _, k := range l.Keys() {
		if it := items[k]; it != nil {
			p += it.Weight
		}
	}
	return p
}

func maxSoldiers(p *Player) int {
	n := 2 + p.XP/5000
	if n > 6 {
		n = 6
	}
	return n
}

func soldierView(s *Soldier) gin.H {
	l := s.GetLoadout()
	owned := []string{}
	for k := range ownedKeys(s.ID) {
		owned = append(owned, k)
	}
	sort.Strings(owned)
	return gin.H{"id": s.ID, "faction": s.Faction, "class": s.Class, "xp": s.XP, "rank": rankFor(s.XP), "credits": s.Credits,
		"kills": s.Kills, "deaths": s.Deaths, "ribbons": s.GetCatKills(), "loadout": l, "points": loadoutPoints(l), "budget": budgetFor(s.Class), "owned": owned}
}

// ---------- handlers ----------

func mySoldier(c *gin.Context) *Soldier {
	var s Soldier
	id, _ := strconv.Atoi(c.Param("id"))
	if id == 0 || db.Where("id = ? AND player_id = ?", id, user(c).ID).First(&s).Error != nil {
		errJ(c, 404, "soldier not found")
		return nil
	}
	return &s
}

func listSoldiers(c *gin.Context) {
	u := user(c)
	var ss []Soldier
	db.Where("player_id = ?", u.ID).Order("id").Find(&ss)
	out := []gin.H{}
	for i := range ss {
		v := soldierView(&ss[i])
		v["active"] = ss[i].ID == u.ActiveSoldierID
		out = append(out, v)
	}
	c.JSON(200, gin.H{"soldiers": out, "maxSoldiers": maxSoldiers(u), "classes": soldierClasses})
}

func createSoldier(c *gin.Context) {
	u := user(c)
	var in struct{ Faction, Class string }
	_ = c.ShouldBindJSON(&in)
	if in.Faction != "US" && in.Faction != "GER" {
		errJ(c, 400, "invalid faction")
		return
	}
	ok := false
	for _, x := range soldierClasses {
		ok = ok || x == in.Class
	}
	if !ok {
		errJ(c, 400, "invalid class")
		return
	}
	var total, inFac int64
	db.Model(&Soldier{}).Where("player_id = ?", u.ID).Count(&total)
	db.Model(&Soldier{}).Where("player_id = ? AND faction = ?", u.ID, in.Faction).Count(&inFac)
	// the first soldier of each faction is always free; extra slots come with progression
	if inFac > 0 && int(total) >= maxSoldiers(u) {
		errJ(c, 400, fmt.Sprintf("no free soldier slot (%d max, earn XP to unlock more)", maxSoldiers(u)))
		return
	}
	s := Soldier{PlayerID: u.ID, Faction: in.Faction, Class: in.Class, CreatedAt: time.Now().UTC(), CatKills: "{}"}
	db.Create(&s)
	grantStarters(&s)
	if u.ActiveSoldierID == 0 || (u.Faction == in.Faction && inFac == 0) {
		db.Model(u).Update("active_soldier_id", s.ID)
	}
	if u.Faction == "" {
		db.Model(u).Update("faction", in.Faction)
	}
	c.JSON(200, soldierView(&s))
}

func activateSoldier(c *gin.Context) {
	if s := mySoldier(c); s != nil {
		db.Model(user(c)).Update("active_soldier_id", s.ID)
		c.JSON(200, gin.H{"ok": true})
	}
}

func getLoadout(c *gin.Context) {
	if s := mySoldier(c); s != nil {
		l := s.GetLoadout()
		c.JSON(200, gin.H{"loadout": l, "points": loadoutPoints(l), "budget": budgetFor(s.Class)})
	}
}

func putLoadout(c *gin.Context) {
	s := mySoldier(c)
	if s == nil {
		return
	}
	var l Loadout
	if c.ShouldBindJSON(&l) != nil {
		errJ(c, 400, "invalid request")
		return
	}
	if l.Equipment == nil {
		l.Equipment = []string{}
	}
	pts, e := validateLoadout(s, l)
	if e != "" {
		errJ(c, 400, e)
		return
	}
	b, _ := json.Marshal(l)
	db.Model(s).Update("loadout", string(b))
	c.JSON(200, gin.H{"loadout": l, "points": pts, "budget": budgetFor(s.Class)})
}

func getStore(c *gin.Context) {
	var s *Soldier
	if u := user(c); u != nil && c.Query("soldierId") != "" {
		var x Soldier
		if db.Where("id = ? AND player_id = ?", c.Query("soldierId"), u.ID).First(&x).Error == nil {
			s = &x
		}
	}
	var owned map[string]bool
	if s != nil {
		owned = ownedKeys(s.ID)
	}
	out := []gin.H{}
	keys := make([]string, 0, len(items))
	for k := range items {
		keys = append(keys, k)
	}
	sort.Strings(keys)
	for _, k := range keys {
		it := items[k]
		if s != nil && !itemFits(it, s) {
			continue
		}
		v := gin.H{"key": k, "label": it.Label, "faction": it.Faction, "slot": it.Slot, "category": it.Category, "classes": it.Classes,
			"price": it.Price, "weight": it.Weight, "rankRequired": it.RankRequired, "ribbon": it.Ribbon}
		if s != nil {
			v["owned"] = owned[k]
			v["locked"] = unlockState(it, s)
		}
		out = append(out, v)
	}
	c.JSON(200, out)
}

func buyItem(c *gin.Context) {
	s := mySoldier(c)
	if s == nil {
		return
	}
	var in struct {
		Item string `json:"item"`
	}
	_ = c.ShouldBindJSON(&in)
	it := items[in.Item]
	if it == nil {
		errJ(c, 404, "unknown item")
		return
	}
	if ownedKeys(s.ID)[in.Item] {
		errJ(c, 400, "already owned")
		return
	}
	if r := unlockState(it, s); r != "" {
		errJ(c, 400, r)
		return
	}
	res := db.Model(&Soldier{}).Where("id = ? AND credits >= ?", s.ID, it.Price).Update("credits", gormExprCol("credits", -it.Price))
	if res.RowsAffected == 0 {
		errJ(c, 400, "not enough credits")
		return
	}
	db.Create(&SoldierItem{SoldierID: s.ID, ItemKey: in.Item, CreatedAt: time.Now().UTC()})
	c.JSON(200, gin.H{"ok": true, "credits": s.Credits - it.Price})
}

// ---------- battle integration ----------

type PlayerLoadout struct {
	SoldierID uint     `json:"soldierId"`
	Class     string   `json:"class"`
	Rank      string   `json:"rank"`
	Items     []string `json:"items"`
	Points    int      `json:"points"`
}

// activeSoldierFor : the player's active soldier if in one of the sides, else their first soldier of a battle side
func activeSoldierFor(p *Player, sides []string) *Soldier {
	var s Soldier
	if p.ActiveSoldierID != 0 && db.Where("id = ? AND faction IN ?", p.ActiveSoldierID, sides).First(&s).Error == nil {
		return &s
	}
	if db.Where("player_id = ? AND faction IN ?", p.ID, sides).Order("id").First(&s).Error == nil {
		return &s
	}
	return nil
}

func playerLoadout(p *Player, sides []string) *PlayerLoadout {
	s := activeSoldierFor(p, sides)
	if s == nil {
		return nil
	}
	l := s.GetLoadout()
	return &PlayerLoadout{SoldierID: s.ID, Class: s.Class, Rank: rankFor(s.XP), Items: l.Keys(), Points: loadoutPoints(l)}
}

func battleLoadouts(b *Battle) map[string]PlayerLoadout {
	out := map[string]PlayerLoadout{}
	var ps []Player
	db.Where("identity_id <> ''").Find(&ps)
	for i := range ps {
		if pl := playerLoadout(&ps[i], []string{b.Attacker, b.Defender}); pl != nil {
			out[ps[i].IdentityID] = *pl
		}
	}
	return out
}

func getBattleLoadout(c *gin.Context) {
	b := loadBattle(c)
	if b == nil {
		return
	}
	var p Player
	if db.Where("identity_id = ?", c.Param("identityId")).First(&p).Error != nil {
		c.JSON(200, gin.H{"linked": false})
		return
	}
	pl := playerLoadout(&p, []string{b.Attacker, b.Defender})
	c.JSON(200, gin.H{"linked": true, "faction": p.Faction, "loadout": pl})
}

// creditSoldier : apply battle stats to the soldier used in the battle
func creditSoldier(p *Player, b *Battle, rp *ResPlayer, xp int) {
	var s *Soldier
	if rp.SoldierID != 0 {
		var x Soldier
		if db.Where("id = ? AND player_id = ?", rp.SoldierID, p.ID).First(&x).Error == nil {
			s = &x
		}
	}
	if s == nil {
		s = activeSoldierFor(p, []string{rp.Side})
	}
	if s == nil {
		return
	}
	ck := s.GetCatKills()
	for cat, st := range rp.WeaponStats {
		ck[cat] += st.Kills
	}
	cb, _ := json.Marshal(ck)
	db.Model(s).Updates(map[string]interface{}{"xp": s.XP + xp, "credits": s.Credits + xp/10, "kills": s.Kills + rp.Kills,
		"deaths": s.Deaths + rp.Deaths, "cat_kills": string(cb)})
}
