package main

// P3 — account careers + career lock (web part; docs/CAMPAIGN.md).
// A player picks one career for his account. It decides which specialist soldier classes
// he may enlist and which specialist seats the mod should let him take (seat enforcement
// itself is mod work [GM], fed by GET /careers/by-identity/:identityId).
// Lock: first pick is free; afterwards a change costs credits and is possible only
// once every careerLockDays. Accounts without a career are grandfathered (no restriction).

import (
	"fmt"
	"time"

	"github.com/gin-gonic/gin"
)

const (
	careerLockDays    = 7
	careerSwitchPrice = 500
)

type CareerSpec struct {
	Key     string   `json:"key"`
	Label   string   `json:"label"`
	Desc    string   `json:"desc"`
	Classes []string `json:"classes"` // soldier classes allowed (INF always allowed)
	Seats   []string `json:"seats"`   // specialist seats for the mod
}

var careers = map[string]CareerSpec{
	"infantry": {"infantry", "Infantryman", "Line and airborne infantry.", []string{"INF", "PARA"}, []string{}},
	"armor":    {"armor", "Tanker", "Crews tanks and armoured cars.", []string{"INF", "CREW"}, []string{"tank_driver", "tank_gunner", "tank_commander"}},
	"pilot":    {"pilot", "Aviator", "Flies aircraft, jumps with the airborne.", []string{"INF", "PARA"}, []string{"pilot"}},
	"recon":    {"recon", "Scout", "Reconnaissance, snipers and light vehicles.", []string{"INF", "REC"}, []string{"recon_driver"}},
}

type PlayerCareer struct {
	PlayerID  uint      `gorm:"primaryKey" json:"playerId"`
	Career    string    `gorm:"size:16" json:"career"`
	ChangedAt time.Time `json:"changedAt"`
}

func careerOf(playerID uint) *PlayerCareer {
	var pc PlayerCareer
	if db.First(&pc, "player_id = ?", playerID).Error != nil {
		return nil
	}
	return &pc
}

func careerAllowsClass(career, class string) bool {
	cs, ok := careers[career]
	if !ok {
		return true // no career yet: grandfathered
	}
	for _, c := range cs.Classes {
		if c == class {
			return true
		}
	}
	return false
}

// careerChange : price to pay and error ("" ok) for switching from cur to next at now
func careerChange(cur *PlayerCareer, next string, now time.Time) (int, string) {
	if _, ok := careers[next]; !ok {
		return 0, "unknown career"
	}
	if cur == nil || cur.Career == "" {
		return 0, ""
	}
	if cur.Career == next {
		return 0, "already your career"
	}
	if until := cur.ChangedAt.Add(careerLockDays * 24 * time.Hour); now.Before(until) {
		return 0, fmt.Sprintf("career locked until %s", until.Format("2006-01-02 15:04 UTC"))
	}
	return careerSwitchPrice, ""
}

// careerBlocksClass : hook for createSoldier, "" if allowed
func careerBlocksClass(u *Player, class string) string {
	pc := careerOf(u.ID)
	if pc == nil || careerAllowsClass(pc.Career, class) {
		return ""
	}
	return "your career (" + careers[pc.Career].Label + ") does not allow this class"
}

func getMyCareer(c *gin.Context) {
	u := user(c)
	out := gin.H{"careers": careers, "lockDays": careerLockDays, "switchPrice": careerSwitchPrice}
	if pc := careerOf(u.ID); pc != nil {
		out["career"], out["changedAt"] = pc.Career, pc.ChangedAt
		out["lockedUntil"] = pc.ChangedAt.Add(careerLockDays * 24 * time.Hour)
	}
	c.JSON(200, out)
}

func setMyCareer(c *gin.Context) {
	u := user(c)
	var in struct {
		Career string `json:"career"`
	}
	_ = c.ShouldBindJSON(&in)
	now := time.Now().UTC()
	cur := careerOf(u.ID)
	price, msg := careerChange(cur, in.Career, now)
	if msg != "" {
		errJ(c, 400, msg)
		return
	}
	if price > 0 {
		res := db.Model(&Player{}).Where("id = ? AND credits >= ?", u.ID, price).Update("credits", gormExprCol("credits", -price))
		if res.RowsAffected == 0 {
			errJ(c, 400, fmt.Sprintf("a career change costs %d credits", price))
			return
		}
	}
	db.Save(&PlayerCareer{PlayerID: u.ID, Career: in.Career, ChangedAt: now})
	c.JSON(200, gin.H{"ok": true, "career": in.Career, "paid": price})
}

func careerByIdentity(c *gin.Context) {
	var p Player
	if db.Where("identity_id = ?", c.Param("identityId")).First(&p).Error != nil {
		c.JSON(200, gin.H{"linked": false})
		return
	}
	out := gin.H{"linked": true, "career": "", "seats": []string{}}
	if pc := careerOf(p.ID); pc != nil {
		out["career"], out["seats"] = pc.Career, careers[pc.Career].Seats
	}
	c.JSON(200, out)
}
