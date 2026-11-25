package main

// P3 — soldier cosmetics, weapon upgrades, cross-faction surcharge (docs/CAMPAIGN.md).
//   - Cosmetics: one uniform camo + one face preset per soldier. Camos are bought once with
//     soldier credits (some rank-gated); faces are free. Pure visual, read by the mod.
//   - Weapon upgrades: an owned primary/secondary weapon can be worked up 3 levels
//     (sights / action / match ammo). Level n needs rank n+1; price grows with the
//     weapon's own price. Effects are applied mod-side from the exported levels.
//   - Cross-faction: enemy primary/secondary weapons can be bought from rank 3 at +50%.

import (
	"encoding/json"
	"sort"
	"time"

	"github.com/gin-gonic/gin"
)

const (
	crossFactionMinRank = 3
	crossFactionPct     = 50
	weaponUpgradeMax    = 3
)

// ---------- cross-faction ----------

func isCrossFaction(it *Item, s *Soldier) bool {
	return it.Faction != s.Faction && it.Faction != "ANY" && (it.Slot == "primary" || it.Slot == "secondary")
}

// itemUsable : own-faction fit, or an enemy weapon the soldier's rank allows
func itemUsable(it *Item, s *Soldier) bool {
	if itemFits(it, s) {
		return true
	}
	return isCrossFaction(it, s) && classOK(it, s.Class) && rankNum(s.XP) >= crossFactionMinRank
}

func unlockPrice(it *Item, s *Soldier) int {
	if s != nil && isCrossFaction(it, s) {
		return it.Price * (100 + crossFactionPct) / 100
	}
	return it.Price
}

// ---------- cosmetics ----------

type Cosmetic struct {
	Key   string `json:"key"`
	Label string `json:"label"`
	Price int    `json:"price"`
	Rank  int    `json:"rank"`
}

var camos = map[string]Cosmetic{
	"standard": {"standard", "Standard issue", 0, 1},
	"field":    {"field", "Field-worn", 150, 1},
	"mud":      {"mud", "Mud-stained", 250, 2},
	"winter":   {"winter", "Winter whites", 400, 3},
	"veteran":  {"veteran", "Veteran's patched", 800, 5},
}

const facePresets = 8 // face_1 .. face_8, free

type SoldierCosmetic struct {
	SoldierID uint      `gorm:"primaryKey" json:"soldierId"`
	Camo      string    `gorm:"size:32" json:"camo"`
	Face      int       `json:"face"`
	Owned     string    `gorm:"type:text" json:"-"` // []string camo keys
	UpdatedAt time.Time `json:"updatedAt"`
}

func (c *SoldierCosmetic) owned() map[string]bool {
	var l []string
	_ = json.Unmarshal([]byte(c.Owned), &l)
	m := map[string]bool{"standard": true}
	for _, k := range l {
		m[k] = true
	}
	return m
}

func cosmeticFor(sid uint) *SoldierCosmetic {
	c := SoldierCosmetic{SoldierID: sid, Camo: "standard", Face: 1, Owned: "[]"}
	db.First(&c, "soldier_id = ?", sid)
	return &c
}

// camoPurchase : price to pay (0 if owned) or an error message
func camoPurchase(key string, owned map[string]bool, rank int) (int, string) {
	cm, ok := camos[key]
	if !ok {
		return 0, "unknown camo"
	}
	if owned[key] {
		return 0, ""
	}
	if rank < cm.Rank {
		return 0, "requires a higher rank"
	}
	return cm.Price, ""
}

func getCosmetics(c *gin.Context) {
	var s Soldier
	if db.First(&s, c.Param("id")).Error != nil {
		errJ(c, 404, "soldier not found")
		return
	}
	cm := cosmeticFor(s.ID)
	c.JSON(200, gin.H{"camo": cm.Camo, "face": cm.Face, "owned": cm.owned(), "camos": camos, "faces": facePresets})
}

func setCosmetics(c *gin.Context) {
	s := mySoldier(c)
	if s == nil {
		return
	}
	var in struct {
		Camo string `json:"camo"`
		Face int    `json:"face"`
	}
	_ = c.ShouldBindJSON(&in)
	cm := cosmeticFor(s.ID)
	if in.Face != 0 {
		if in.Face < 1 || in.Face > facePresets {
			errJ(c, 400, "unknown face preset")
			return
		}
		cm.Face = in.Face
	}
	if in.Camo != "" {
		own := cm.owned()
		price, msg := camoPurchase(in.Camo, own, rankNum(s.XP))
		if msg != "" {
			errJ(c, 400, msg)
			return
		}
		if price > 0 {
			res := db.Model(&Soldier{}).Where("id = ? AND credits >= ?", s.ID, price).Update("credits", gormExprCol("credits", -price))
			if res.RowsAffected == 0 {
				errJ(c, 400, "not enough credits")
				return
			}
			own[in.Camo] = true
			var l []string
			for k := range own {
				l = append(l, k)
			}
			sort.Strings(l)
			b, _ := json.Marshal(l)
			cm.Owned = string(b)
		}
		cm.Camo = in.Camo
	}
	cm.UpdatedAt = time.Now().UTC()
	db.Save(cm)
	c.JSON(200, gin.H{"ok": true, "camo": cm.Camo, "face": cm.Face})
}

// ---------- weapon upgrades ----------

var weaponUpgradeNames = []string{"", "Zeroed sights", "Polished action", "Match ammunition"}

type SoldierWeaponUpgrade struct {
	ID        uint   `gorm:"primaryKey" json:"-"`
	SoldierID uint   `gorm:"uniqueIndex:ux_sw_upg" json:"soldierId"`
	ItemKey   string `gorm:"size:64;uniqueIndex:ux_sw_upg" json:"item"`
	Level     int    `json:"level"`
}

func weaponUpgradePrice(itemPrice, level int) int {
	return 100*level + itemPrice*level/4
}

// weaponUpgradeCheck : "" if level `next` may be bought for this weapon
func weaponUpgradeCheck(it *Item, owned bool, cur, rank int) string {
	switch {
	case it == nil || (it.Slot != "primary" && it.Slot != "secondary"):
		return "only weapons can be upgraded"
	case !owned:
		return "buy the weapon first"
	case cur >= weaponUpgradeMax:
		return "fully upgraded"
	case rank < cur+2:
		return "requires a higher rank"
	}
	return ""
}

func weaponLevels(sid uint) map[string]int {
	var l []SoldierWeaponUpgrade
	db.Where("soldier_id = ?", sid).Find(&l)
	m := map[string]int{}
	for _, u := range l {
		m[u.ItemKey] = u.Level
	}
	return m
}

func getWeaponUpgrades(c *gin.Context) {
	var s Soldier
	if db.First(&s, c.Param("id")).Error != nil {
		errJ(c, 404, "soldier not found")
		return
	}
	lv := weaponLevels(s.ID)
	own := ownedKeys(s.ID)
	rank := rankNum(s.XP)
	out := []gin.H{}
	for k := range own {
		it := items[k]
		if it == nil || (it.Slot != "primary" && it.Slot != "secondary") {
			continue
		}
		v := gin.H{"item": k, "label": it.Label, "level": lv[k], "max": weaponUpgradeMax}
		if lv[k] < weaponUpgradeMax {
			v["next"], v["price"] = weaponUpgradeNames[lv[k]+1], weaponUpgradePrice(it.Price, lv[k]+1)
		}
		if r := weaponUpgradeCheck(it, true, lv[k], rank); r != "" {
			v["blocked"] = r
		}
		out = append(out, v)
	}
	sort.Slice(out, func(i, j int) bool { return out[i]["item"].(string) < out[j]["item"].(string) })
	c.JSON(200, gin.H{"weapons": out, "names": weaponUpgradeNames})
}

func buyWeaponUpgrade(c *gin.Context) {
	s := mySoldier(c)
	if s == nil {
		return
	}
	key := c.Param("item")
	it := items[key]
	lv := weaponLevels(s.ID)[key]
	if r := weaponUpgradeCheck(it, ownedKeys(s.ID)[key], lv, rankNum(s.XP)); r != "" {
		errJ(c, 400, r)
		return
	}
	price := weaponUpgradePrice(it.Price, lv+1)
	res := db.Model(&Soldier{}).Where("id = ? AND credits >= ?", s.ID, price).Update("credits", gormExprCol("credits", -price))
	if res.RowsAffected == 0 {
		errJ(c, 400, "not enough credits")
		return
	}
	u := SoldierWeaponUpgrade{SoldierID: s.ID, ItemKey: key}
	db.Where("soldier_id = ? AND item_key = ?", s.ID, key).FirstOrCreate(&u)
	db.Model(&u).Update("level", lv+1)
	c.JSON(200, gin.H{"ok": true, "level": lv + 1, "paid": price})
}
