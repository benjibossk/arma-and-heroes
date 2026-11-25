package main

// Progression ("the grind"): battle rewards, rank curve, unlock rules, war funds.
// Numbers live in progression.json (see docs/PROGRESSION.md).

import (
	_ "embed"
	"encoding/json"
	"log"
	"os"
	"sort"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
)

//go:embed progression.json
var progressionRaw []byte

type RewardTable struct {
	PlayerKill      int `json:"playerKill"`
	AIKill          int `json:"aiKill"`
	Assist          int `json:"assist"`
	VehicleKill     int `json:"vehicleKill"`
	Capture         int `json:"capture"`
	Defend          int `json:"defend"`
	ObjectivePerMin int `json:"objectivePerMin"`
	Win             int `json:"win"`
	Loss            int `json:"loss"`
	Participation   int `json:"participation"`
	FirstWinOfDay   int `json:"firstWinOfDay"`
	SquadBonusPct   int `json:"squadBonusPct"`
}

type UnlockRule struct {
	Label   string `json:"label,omitempty"`
	Faction string `json:"faction,omitempty"`
	Tree    string `json:"tree"`
	Rank    int    `json:"rank"`
	Price   int    `json:"price"`
}

type Progression struct {
	XP       RewardTable `json:"xp"`
	Credits  RewardTable `json:"credits"`
	AntiFarm struct {
		AIKillTiers []struct {
			UpTo   int     `json:"upTo"`
			Factor float64 `json:"factor"`
		} `json:"aiKillTiers"`
		ObjectiveMaxMin      int  `json:"objectiveMaxMin"`
		XPCapPerBattle       int  `json:"xpCapPerBattle"`
		CreditsCapPerBattle  int  `json:"creditsCapPerBattle"`
		MinPresenceSec       int  `json:"minPresenceSec"`
		IdleRequiresActivity bool `json:"idleRequiresActivity"`
	} `json:"antiFarm"`
	Ranks []struct {
		XP  int    `json:"xp"`
		US  string `json:"us"`
		GER string `json:"ger"`
	} `json:"ranks"`
	Trees    []string              `json:"trees"`
	Salary   SalaryRule            `json:"salary"`
	Unlocks  map[string]UnlockRule `json:"unlocks"`
	Vehicles map[string]UnlockRule `json:"vehicles"`
	WarFunds struct {
		Win          int            `json:"win"`
		Loss         int            `json:"loss"`
		PerCapture   int            `json:"perCapture"`
		PerPlayer    int            `json:"perPlayer"`
		CityCaptured int            `json:"cityCaptured"`
		TeamCost     map[string]int `json:"teamCost"`
	} `json:"warFunds"`
}

var prog Progression
var itemTree = map[string]string{}

const licensePrefix = "LIC_"

// loadProgression : call after loadItems(). Overrides ranks, item prices/ranks, adds vehicle licenses.
func loadProgression() {
	raw := progressionRaw
	if b, err := os.ReadFile("progression.json"); err == nil {
		raw = b
		log.Println("progression: using ./progression.json override")
	}
	if err := json.Unmarshal(raw, &prog); err != nil {
		log.Fatal("progression.json: ", err)
	}
	if len(prog.Ranks) > 0 {
		ranks = ranks[:0]
		for _, r := range prog.Ranks {
			ranks = append(ranks, struct {
				XP   int
				Name string
			}{r.XP, r.US})
		}
	}
	for k, u := range prog.Unlocks {
		it := items[k]
		if it == nil {
			log.Println("progression: unknown item", k)
			continue
		}
		it.Price = u.Price
		it.RankRequired = rankNamePtr(u.Rank)
		itemTree[k] = u.Tree
	}
	for k, v := range prog.Vehicles {
		key := licensePrefix + k
		items[key] = &Item{Key: key, Label: v.Label + " licence", Faction: v.Faction, Slot: "license", Category: "vehicle",
			Classes: soldierClasses, Price: v.Price, RankRequired: rankNamePtr(v.Rank)}
		itemTree[key] = v.Tree
	}
	for t, cost := range prog.WarFunds.TeamCost {
		if s, ok := atSpecs[t]; ok {
			s.Cost = cost
			atSpecs[t] = s
		}
	}
}

func rankNamePtr(n int) *string {
	if n <= 1 || n > len(ranks) {
		return nil
	}
	s := ranks[n-1].Name
	return &s
}

// rankNum : 1-based rank for an XP total
func rankNum(xp int) int { return rankIdxFor(xp) + 1 }

func rankLabel(n int, faction string) string {
	if n < 1 || n > len(prog.Ranks) {
		return ""
	}
	if faction == "GER" {
		return prog.Ranks[n-1].GER
	}
	return prog.Ranks[n-1].US
}

func itemRank(it *Item) int {
	if it.RankRequired == nil {
		return 1
	}
	return rankIndex(*it.RankRequired) + 1
}

// ---------- battle rewards ----------

type Reward struct {
	XP      int            `json:"xp"`
	Credits int            `json:"credits"`
	Lines   map[string]int `json:"lines"` // xp per source, for the result screen
	Idle    bool           `json:"idle,omitempty"`
}

// aiKillValue : effective AI kills after diminishing tiers
func aiKillValue(n int) float64 {
	v, prev := 0.0, 0
	for _, t := range prog.AntiFarm.AIKillTiers {
		if n <= prev {
			break
		}
		c := n
		if c > t.UpTo {
			c = t.UpTo
		}
		v += float64(c-prev) * t.Factor
		prev = t.UpTo
	}
	return v
}

func computeReward(rp *ResPlayer, won, firstWin bool) Reward {
	af := prog.AntiFarm
	pk, ak := rp.PlayerKills, rp.AIKillCount
	if pk == 0 && ak == 0 { // mod does not split yet: count everything as AI kills (conservative)
		ak = rp.Kills
	}
	active := pk+ak+rp.Assists+rp.Captures+rp.Defends+rp.Deaths+rp.VehicleKills > 0 || rp.ObjectiveSec > 0
	present := rp.PresenceSec == 0 || rp.PresenceSec >= af.MinPresenceSec
	if !present || (af.IdleRequiresActivity && !active) {
		return Reward{Lines: map[string]int{}, Idle: true}
	}
	objMin := rp.ObjectiveSec / 60
	if objMin > af.ObjectiveMaxMin {
		objMin = af.ObjectiveMaxMin
	}
	aiv := aiKillValue(ak)
	calc := func(t RewardTable, lines map[string]int) int {
		add := func(k string, v int) int {
			if lines != nil && v != 0 {
				lines[k] = v
			}
			return v
		}
		s := add("playerKills", pk*t.PlayerKill) + add("aiKills", int(aiv*float64(t.AIKill))) + add("assists", rp.Assists*t.Assist) +
			add("vehicleKills", rp.VehicleKills*t.VehicleKill) + add("captures", rp.Captures*t.Capture) + add("defends", rp.Defends*t.Defend) +
			add("objective", objMin*t.ObjectivePerMin) + add("participation", t.Participation)
		if won {
			s += add("win", t.Win)
		} else {
			s += add("loss", t.Loss)
		}
		if rp.PresenceSec > 0 && rp.SquadSec*2 >= rp.PresenceSec {
			s += add("squad", s*t.SquadBonusPct/100)
		}
		if won && firstWin {
			s += add("firstWinOfDay", t.FirstWinOfDay) // outside the cap on purpose
		}
		return s
	}
	r := Reward{Lines: map[string]int{}}
	r.XP = calc(prog.XP, r.Lines)
	r.Credits = calc(prog.Credits, nil)
	bonusXP, bonusCr := 0, 0
	if won && firstWin {
		bonusXP, bonusCr = prog.XP.FirstWinOfDay, prog.Credits.FirstWinOfDay
	}
	if af.XPCapPerBattle > 0 && r.XP-bonusXP > af.XPCapPerBattle {
		r.XP = af.XPCapPerBattle + bonusXP
	}
	if af.CreditsCapPerBattle > 0 && r.Credits-bonusCr > af.CreditsCapPerBattle {
		r.Credits = af.CreditsCapPerBattle + bonusCr
	}
	return r
}

func hasWonToday(playerID uint) bool {
	var n int64
	day := time.Now().UTC().Truncate(24 * time.Hour)
	db.Model(&PlayerBattle{}).Where("player_id = ? AND won = ? AND created_at >= ?", playerID, true, day).Count(&n)
	return n > 0
}

// applyWarFunds : team resources earned from a battle (caller holds gameMu)
func applyWarFunds(b *Battle, r *BattleResult) {
	wf := prog.WarFunds
	players, caps := map[string]int{}, map[string]int{}
	for _, rp := range r.Players {
		players[rp.Side]++
		caps[rp.Side] += rp.Captures
	}
	for _, side := range []string{b.Attacker, b.Defender} {
		v := wf.Loss
		if side == r.Winner {
			v = wf.Win
			if side == b.Attacker && b.RoadID == 0 { // no city taken in a road skirmish
				v += wf.CityCaptured
			}
		}
		v += players[side]*wf.PerPlayer + caps[side]*wf.PerCapture
		if v > 0 {
			db.Model(&Faction{}).Where("war_id = ? AND code = ?", b.WarID, side).Update("resources", gormExprAdd(v))
		}
	}
	applyKillFunds(b, r) // P2-E (killfunds.go)
}

// ---------- unlocks API ----------

// unlockSoldier : ?soldierId, else active soldier, else first soldier of the player
func unlockSoldier(c *gin.Context, u *Player) *Soldier {
	var s Soldier
	if id, _ := strconv.Atoi(c.Query("soldierId")); id != 0 {
		if db.Where("id = ? AND player_id = ?", id, u.ID).First(&s).Error == nil {
			return &s
		}
		return nil
	}
	if u.ActiveSoldierID != 0 && db.Where("id = ? AND player_id = ?", u.ActiveSoldierID, u.ID).First(&s).Error == nil {
		return &s
	}
	if db.Where("player_id = ?", u.ID).Order("id").First(&s).Error == nil {
		return &s
	}
	return nil
}

func getUnlocks(c *gin.Context) {
	u := user(c)
	var s *Soldier
	if u != nil {
		s = unlockSoldier(c, u)
	}
	var owned map[string]bool
	out := gin.H{"trees": prog.Trees, "earn": gin.H{"xp": prog.XP, "credits": prog.Credits, "antiFarm": prog.AntiFarm}}
	rk := []gin.H{}
	for i, r := range prog.Ranks {
		rk = append(rk, gin.H{"n": i + 1, "xp": r.XP, "us": r.US, "ger": r.GER})
	}
	out["ranks"] = rk
	if s != nil {
		owned = ownedKeys(s.ID)
		n := rankNum(s.XP)
		next := -1
		if n < len(prog.Ranks) {
			next = prog.Ranks[n].XP
		}
		out["soldier"] = gin.H{"id": s.ID, "faction": s.Faction, "class": s.Class, "xp": s.XP, "credits": s.Credits,
			"rank": n, "rankName": rankLabel(n, s.Faction), "rankXP": prog.Ranks[n-1].XP, "nextXP": next}
	}
	keys := make([]string, 0, len(items))
	for k := range items {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool {
		a, b := items[keys[i]], items[keys[j]]
		if itemRank(a) != itemRank(b) {
			return itemRank(a) < itemRank(b)
		}
		if a.Price != b.Price {
			return a.Price < b.Price
		}
		return keys[i] < keys[j]
	})
	list := []gin.H{}
	for _, k := range keys {
		it := items[k]
		n := itemRank(it)
		v := gin.H{"key": k, "label": it.Label, "faction": it.Faction, "slot": it.Slot, "category": it.Category, "classes": it.Classes,
			"weight": it.Weight, "price": it.Price, "rank": n, "rankName": rankLabel(n, it.Faction), "tree": itemTree[k], "ribbon": it.Ribbon,
			"starter": it.Starter}
		if s != nil {
			v["price"] = unlockPrice(it, s) // P3 cross-faction surcharge
			switch reason := unlockState(it, s); {
			case owned[k]:
				v["state"] = "owned"
			case reason != "":
				v["state"], v["reason"] = "locked", reason
			case s.Credits < unlockPrice(it, s):
				v["state"], v["reason"] = "locked", "not enough credits"
			default:
				v["state"] = "buyable"
			}
		}
		list = append(list, v)
	}
	out["items"] = list
	c.JSON(200, out)
}

// buyUnlock : POST /unlocks/:key/buy — idempotent (owned -> 200 already:true)
func buyUnlock(c *gin.Context) {
	u := user(c)
	s := unlockSoldier(c, u)
	if s == nil {
		errJ(c, 404, "no soldier: create one in the barracks first")
		return
	}
	key := c.Param("key")
	it := items[key]
	if it == nil {
		errJ(c, 404, "unknown item")
		return
	}
	if ownedKeys(s.ID)[key] {
		c.JSON(200, gin.H{"ok": true, "already": true, "credits": s.Credits})
		return
	}
	if r := unlockState(it, s); r != "" {
		errJ(c, 400, r)
		return
	}
	price := unlockPrice(it, s)
	res := db.Model(&Soldier{}).Where("id = ? AND credits >= ?", s.ID, price).Update("credits", gormExprCol("credits", -price))
	if res.RowsAffected == 0 {
		errJ(c, 400, "not enough credits")
		return
	}
	if err := db.Create(&SoldierItem{SoldierID: s.ID, ItemKey: key, CreatedAt: time.Now().UTC()}).Error; err != nil {
		// concurrent duplicate buy: refund, stay idempotent
		db.Model(&Soldier{}).Where("id = ?", s.ID).Update("credits", gormExprCol("credits", price))
		c.JSON(200, gin.H{"ok": true, "already": true, "credits": s.Credits})
		return
	}
	c.JSON(200, gin.H{"ok": true, "credits": s.Credits - price, "item": key})
}

// seedStarterKits : existing soldiers get any starter item they lack (safe to rerun)
func seedStarterKits() {
	var ss []Soldier
	db.Find(&ss)
	for i := range ss {
		owned := ownedKeys(ss[i].ID)
		for k, it := range items {
			if it.Starter && !owned[k] && itemFits(it, &ss[i]) {
				db.Create(&SoldierItem{SoldierID: ss[i].ID, ItemKey: k, CreatedAt: time.Now().UTC()})
			}
		}
	}
}
