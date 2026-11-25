package main

// P2-G combat badges (GAP_PLAN.md): one badge slot per soldier, chosen in the barracks.
// A badge is unlocked by a combat ribbon; its level (1 bronze .. 3 gold) follows the highest tier of that ribbon.
// The battle payload carries playerLoadouts[id].badge / badgeLevel; the mod applies the effect (F44_Badges.c).

import (
	"sync"

	"github.com/gin-gonic/gin"
)

type BadgeDef struct {
	Key    string `json:"key"`
	Name   string `json:"name"`
	Ribbon string `json:"ribbon"` // ribbons.json id that unlocks it
	Effect string `json:"effect"` // per level
}

var badgeDefs = []BadgeDef{
	{"FIRST_AID", "First Aid", "heal", "+1 field dressing per level at each deploy"},
	{"COMBAT_MEDIC", "Combat Medic", "revive", "medical items used 15% faster per level"},
	{"HEAVY_SET", "Heavy Set", "rifle", "5% less damage taken per level"},
}

// SoldierBadge : badge worn by a soldier (one slot)
type SoldierBadge struct {
	SoldierID uint   `gorm:"primaryKey;autoIncrement:false"`
	Badge     string `gorm:"size:24"`
}

var badgeMigrate sync.Once

func badgeDB() {
	badgeMigrate.Do(func() { db.AutoMigrate(&SoldierBadge{}) })
}

func badgeDef(key string) *BadgeDef {
	for i := range badgeDefs {
		if badgeDefs[i].Key == key {
			return &badgeDefs[i]
		}
	}
	return nil
}

// badgeLevel : 0 (locked) .. 3, from the highest awarded tier of the badge's ribbon
func badgeLevel(soldierID uint, d *BadgeDef) int {
	var t struct{ Max int }
	db.Model(&SoldierRibbon{}).Select("MAX(tier) AS max").Where("soldier_id = ? AND ribbon_id = ?", soldierID, d.Ribbon).Scan(&t)
	if t.Max > 3 {
		return 3
	}
	return t.Max
}

// soldierBadge : worn badge and its current level ("" , 0 when none or locked)
func soldierBadge(soldierID uint) (string, int) {
	badgeDB()
	var b SoldierBadge
	if db.Where("soldier_id = ?", soldierID).First(&b).Error != nil {
		return "", 0
	}
	d := badgeDef(b.Badge)
	if d == nil {
		return "", 0
	}
	lv := badgeLevel(soldierID, d)
	if lv == 0 {
		return "", 0
	}
	return d.Key, lv
}

// GET /api/soldiers/:id/badges
func getBadges(c *gin.Context) {
	s := mySoldier(c)
	if s == nil {
		return
	}
	worn, _ := soldierBadge(s.ID)
	out := []gin.H{}
	for i := range badgeDefs {
		d := &badgeDefs[i]
		out = append(out, gin.H{"key": d.Key, "name": d.Name, "ribbon": d.Ribbon, "effect": d.Effect, "level": badgeLevel(s.ID, d)})
	}
	c.JSON(200, gin.H{"badges": out, "worn": worn})
}

// PUT /api/soldiers/:id/badge {badge: "" | KEY}
func putBadge(c *gin.Context) {
	s := mySoldier(c)
	if s == nil {
		return
	}
	var in struct {
		Badge string `json:"badge"`
	}
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "invalid request")
		return
	}
	badgeDB()
	if in.Badge == "" {
		db.Where("soldier_id = ?", s.ID).Delete(&SoldierBadge{})
		c.JSON(200, gin.H{"worn": ""})
		return
	}
	d := badgeDef(in.Badge)
	if d == nil || badgeLevel(s.ID, d) == 0 {
		errJ(c, 400, "badge locked: earn its ribbon first")
		return
	}
	db.Save(&SoldierBadge{SoldierID: s.ID, Badge: d.Key})
	c.JSON(200, gin.H{"worn": d.Key})
}
