package main

// Ribbons: per-soldier stat counters -> bronze/silver/gold tiers -> one-shot XP/credit rewards
// and item gates. Tuning in ribbons.json (see docs/RIBBONS.md).

import (
	_ "embed"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm/clause"
)

//go:embed ribbons.json
var ribbonsRaw []byte

type RibbonTier struct {
	At      int `json:"at"`
	XP      int `json:"xp"`
	Credits int `json:"credits"`
}

type RibbonDef struct {
	ID       string       `json:"id"`
	Name     string       `json:"name"`
	Category string       `json:"category"`
	Stat     string       `json:"stat"`
	Tiers    []RibbonTier `json:"tiers"`
	Unlocks  []struct {
		Item string `json:"item"`
		Tier int    `json:"tier"`
	} `json:"unlocks"`
}

var ribbonCfg struct {
	TierNames    []string    `json:"tierNames"`
	AIKillFactor float64     `json:"aiKillFactor"`
	Ribbons      []RibbonDef `json:"ribbons"`
}

type ribbonGate struct {
	R    *RibbonDef
	Tier int
}

var itemRibbonGate = map[string]ribbonGate{}

// SoldierStat : one counter per (soldier, stat key), e.g. "kills.rifle", "healGiven"
type SoldierStat struct {
	ID        uint   `gorm:"primaryKey"`
	SoldierID uint   `gorm:"uniqueIndex:ux_soldier_stat"`
	Stat      string `gorm:"size:64;uniqueIndex:ux_soldier_stat"`
	Value     int
}

// SoldierRibbon : a tier awarded once (unique index = idempotent award)
type SoldierRibbon struct {
	ID        uint   `gorm:"primaryKey"`
	SoldierID uint   `gorm:"uniqueIndex:ux_soldier_ribbon"`
	RibbonID  string `gorm:"size:32;uniqueIndex:ux_soldier_ribbon"`
	Tier      int    `gorm:"uniqueIndex:ux_soldier_ribbon"`
	BattleID  uint
	CreatedAt time.Time
}

func loadRibbons() {
	raw := ribbonsRaw
	if b, err := os.ReadFile("ribbons.json"); err == nil {
		raw = b
		log.Println("ribbons: using ./ribbons.json override")
	}
	if err := json.Unmarshal(raw, &ribbonCfg); err != nil {
		log.Fatal("ribbons.json: ", err)
	}
	for i := range ribbonCfg.Ribbons {
		r := &ribbonCfg.Ribbons[i]
		for _, u := range r.Unlocks {
			if items[u.Item] == nil {
				log.Println("ribbons: unknown item", u.Item)
				continue
			}
			itemRibbonGate[u.Item] = ribbonGate{r, u.Tier}
		}
	}
}

func tierName(n int) string {
	if n >= 1 && n <= len(ribbonCfg.TierNames) {
		return ribbonCfg.TierNames[n-1]
	}
	return "Tier " + strconv.Itoa(n)
}

// soldierStats : counters, with legacy Soldier.CatKills merged in as kills.<cat>
func soldierStats(s *Soldier) map[string]int {
	m := map[string]int{}
	var rows []SoldierStat
	db.Where("soldier_id = ?", s.ID).Find(&rows)
	for _, r := range rows {
		m[r.Stat] = r.Value
	}
	for cat, n := range s.GetCatKills() {
		if k := "kills." + cat; m[k] < n {
			m[k] = n
		}
	}
	return m
}

func ribbonTier(r *RibbonDef, v int) int {
	t := 0
	for i, x := range r.Tiers {
		if v >= x.At {
			t = i + 1
		}
	}
	return t
}

// battleCounters : per-battle deltas from a player's result line (docs/RIBBONS.md contract)
func battleCounters(rp *ResPlayer) map[string]int {
	d := map[string]int{}
	add := func(k string, v int) {
		if v > 0 {
			d[k] += v
		}
	}
	if len(rp.WeaponStats) > 0 {
		for cat, st := range rp.WeaponStats {
			add("kills."+cat, st.Kills)
		}
	} else {
		for cat, n := range rp.KillsByCategory {
			add("kills."+cat, n)
		}
	}
	if f := ribbonCfg.AIKillFactor; f < 1 && rp.PlayerKillsByCategory != nil { // optional: weigh AI kills down
		for cat, n := range rp.KillsByCategory {
			ai := n - rp.PlayerKillsByCategory[cat]
			if ai > 0 {
				d["kills."+cat] -= ai - int(float64(ai)*f)
			}
		}
	}
	add("captures", rp.Captures)
	add("defends", rp.Defends)
	add("assists", rp.Assists)
	add("vehicleKills", rp.VehicleKills)
	add("headshots", rp.Headshots)
	add("multiKills", rp.MultiKills)
	add("resupplyGiven", rp.ResupplyGiven)
	add("healGiven", rp.HealGiven)
	add("revives", rp.Revives)
	add("repairs", rp.Repairs)
	add("spots", rp.Spots)
	add("transportPassengers", rp.TransportPassengers)
	for k, v := range rp.Stats { // free-form extra counters
		if !strings.ContainsAny(k, " '\"") && len(k) <= 64 {
			add(k, v)
		}
	}
	d["battles"] = 1
	return d
}

// applyRibbons : add counters, award new tiers once, return bonus xp/credits (caller credits the soldier)
func applyRibbons(s *Soldier, battleID uint, rp *ResPlayer) (xp, credits int, awarded []string) {
	before := soldierStats(s)
	delta := battleCounters(rp)
	for k, v := range delta {
		if v <= 0 {
			continue
		}
		db.Clauses(clause.OnConflict{
			Columns:   []clause.Column{{Name: "soldier_id"}, {Name: "stat"}},
			DoUpdates: clause.Assignments(map[string]interface{}{"value": gormExprCol("value", v)}),
		}).Create(&SoldierStat{SoldierID: s.ID, Stat: k, Value: before[k] + v})
	}
	after := soldierStats(s)
	for i := range ribbonCfg.Ribbons {
		r := &ribbonCfg.Ribbons[i]
		for t := 1; t <= ribbonTier(r, after[r.Stat]); t++ {
			res := db.Clauses(clause.OnConflict{DoNothing: true}).Create(&SoldierRibbon{SoldierID: s.ID, RibbonID: r.ID, Tier: t, BattleID: battleID, CreatedAt: time.Now().UTC()})
			if res.Error == nil && res.RowsAffected > 0 {
				xp += r.Tiers[t-1].XP
				credits += r.Tiers[t-1].Credits
				awarded = append(awarded, r.Name+" "+tierName(t))
			}
		}
	}
	return
}

// ribbonLock : "" if the soldier meets the item's ribbon gate (ribbons.json, else legacy catalog field)
func ribbonLock(it *Item, s *Soldier) string {
	if g, ok := itemRibbonGate[it.Key]; ok {
		if ribbonTier(g.R, soldierStats(s)[g.R.Stat]) < g.Tier {
			return fmt.Sprintf("requires ribbon: %s %s (%d)", g.R.Name, tierName(g.Tier), g.R.Tiers[g.Tier-1].At)
		}
		return ""
	}
	if it.Ribbon != nil && soldierStats(s)["kills."+it.Ribbon.Category] < it.Ribbon.Kills {
		return fmt.Sprintf("requires ribbon: %d %s kills", it.Ribbon.Kills, it.Ribbon.Category)
	}
	return ""
}

// GET /soldiers/:id/ribbons (public: soldiers are visible to all)
func getRibbons(c *gin.Context) {
	var s Soldier
	if db.First(&s, c.Param("id")).Error != nil {
		errJ(c, 404, "unknown soldier")
		return
	}
	st := soldierStats(&s)
	var got []SoldierRibbon
	db.Where("soldier_id = ?", s.ID).Find(&got)
	when := map[string]time.Time{}
	for _, g := range got {
		when[g.RibbonID+"#"+strconv.Itoa(g.Tier)] = g.CreatedAt
	}
	out := []gin.H{}
	for i := range ribbonCfg.Ribbons {
		r := &ribbonCfg.Ribbons[i]
		v := st[r.Stat]
		t := ribbonTier(r, v)
		tiers := []gin.H{}
		for j, x := range r.Tiers {
			h := gin.H{"tier": j + 1, "name": tierName(j + 1), "at": x.At, "xp": x.XP, "credits": x.Credits}
			if w, ok := when[r.ID+"#"+strconv.Itoa(j+1)]; ok {
				h["awardedAt"] = w
			}
			tiers = append(tiers, h)
		}
		unl := []gin.H{}
		for _, u := range r.Unlocks {
			if it := items[u.Item]; it != nil && (it.Faction == "" || it.Faction == s.Faction) {
				unl = append(unl, gin.H{"item": u.Item, "label": it.Label, "tier": u.Tier, "tierName": tierName(u.Tier), "unlocked": t >= u.Tier})
			}
		}
		next := 0
		if t < len(r.Tiers) {
			next = r.Tiers[t].At
		}
		out = append(out, gin.H{"id": r.ID, "name": r.Name, "category": r.Category, "stat": r.Stat, "value": v,
			"tier": t, "tierName": tierName(t), "next": next, "tiers": tiers, "unlocks": unl})
	}
	c.JSON(200, gin.H{"soldierId": s.ID, "tierNames": ribbonCfg.TierNames, "stats": st, "ribbons": out})
}
