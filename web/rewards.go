package main

// P3 — daily reward cycle + seasonal track (docs/CAMPAIGN.md).
//   - Daily: one claim per UTC day; a 7-day cycle of growing credit rewards. Claiming on
//     consecutive days advances the streak, missing a day restarts the cycle at day 1.
//   - Season = calendar quarter (e.g. "2026-Q4"). Season points = battle XP earned since
//     the season start (PlayerBattle). Every seasonTierPts points unlocks a tier; tiers are
//     claimed for credits. Progress resets each season.

import (
	"fmt"
	"time"

	"github.com/gin-gonic/gin"
)

var dailyCycle = []int{50, 75, 100, 150, 200, 250, 500}

const (
	seasonTierPts = 1000
	seasonTiers   = 10
)

type PlayerReward struct {
	PlayerID      uint       `gorm:"primaryKey" json:"playerId"`
	Streak        int        `json:"streak"` // days in the current cycle already claimed (1..7)
	LastDaily     *time.Time `json:"lastDaily"`
	SeasonID      string     `gorm:"size:16" json:"seasonId"`
	SeasonClaimed int        `json:"seasonClaimed"` // highest tier claimed in SeasonID
}

func dayOf(t time.Time) time.Time { y, m, d := t.UTC().Date(); return time.Date(y, m, d, 0, 0, 0, 0, time.UTC) }

// dailyNext : (next streak day 1..7, credits) or ok=false if already claimed today
func dailyNext(last *time.Time, streak int, now time.Time) (int, int, bool) {
	today := dayOf(now)
	if last != nil && !dayOf(*last).Before(today) {
		return streak, 0, false
	}
	day := 1
	if last != nil && dayOf(*last).Equal(today.AddDate(0, 0, -1)) && streak < len(dailyCycle) {
		day = streak + 1
	}
	return day, dailyCycle[day-1], true
}

func seasonOf(t time.Time) (string, time.Time) {
	t = t.UTC()
	q := (int(t.Month()) - 1) / 3
	return fmt.Sprintf("%d-Q%d", t.Year(), q+1), time.Date(t.Year(), time.Month(q*3+1), 1, 0, 0, 0, 0, time.UTC)
}

func seasonTier(points int) int {
	t := points / seasonTierPts
	if t > seasonTiers {
		t = seasonTiers
	}
	return t
}

func seasonTierCredits(tier int) int {
	v := 100 * tier
	if tier%5 == 0 {
		v += 500
	}
	return v
}

// seasonClaim : credits due when claiming every reached tier above `claimed`
func seasonClaim(claimed, reached int) int {
	s := 0
	for t := claimed + 1; t <= reached; t++ {
		s += seasonTierCredits(t)
	}
	return s
}

func rewardRow(pid uint, now time.Time) *PlayerReward {
	r := PlayerReward{PlayerID: pid}
	db.First(&r, "player_id = ?", pid)
	if sid, _ := seasonOf(now); r.SeasonID != sid {
		r.SeasonID, r.SeasonClaimed = sid, 0
	}
	return &r
}

func seasonPoints(pid uint, start time.Time) int {
	var n int
	db.Model(&PlayerBattle{}).Select("COALESCE(SUM(xp),0)").Where("player_id = ? AND created_at >= ?", pid, start).Scan(&n)
	return n
}

func getRewards(c *gin.Context) {
	u := user(c)
	now := time.Now().UTC()
	r := rewardRow(u.ID, now)
	day, cr, can := dailyNext(r.LastDaily, r.Streak, now)
	sid, start := seasonOf(now)
	pts := seasonPoints(u.ID, start)
	tiers := []gin.H{}
	for t := 1; t <= seasonTiers; t++ {
		tiers = append(tiers, gin.H{"tier": t, "points": t * seasonTierPts, "credits": seasonTierCredits(t), "reached": pts >= t*seasonTierPts, "claimed": t <= r.SeasonClaimed})
	}
	c.JSON(200, gin.H{"daily": gin.H{"cycle": dailyCycle, "streak": r.Streak, "canClaim": can, "nextDay": day, "nextCredits": cr},
		"season": gin.H{"id": sid, "start": start, "end": start.AddDate(0, 3, 0), "points": pts, "claimed": r.SeasonClaimed, "due": seasonClaim(r.SeasonClaimed, seasonTier(pts)), "tiers": tiers}})
}

func claimDaily(c *gin.Context) {
	u := user(c)
	gameMu.Lock()
	defer gameMu.Unlock()
	now := time.Now().UTC()
	r := rewardRow(u.ID, now)
	day, cr, can := dailyNext(r.LastDaily, r.Streak, now)
	if !can {
		errJ(c, 400, "already claimed today")
		return
	}
	r.Streak, r.LastDaily = day, &now
	db.Save(r)
	grantCredits(u, u.Faction, cr)
	c.JSON(200, gin.H{"ok": true, "day": day, "credits": cr})
}

func claimSeason(c *gin.Context) {
	u := user(c)
	gameMu.Lock()
	defer gameMu.Unlock()
	now := time.Now().UTC()
	r := rewardRow(u.ID, now)
	_, start := seasonOf(now)
	reached := seasonTier(seasonPoints(u.ID, start))
	cr := seasonClaim(r.SeasonClaimed, reached)
	if cr == 0 {
		errJ(c, 400, "no new season tier reached")
		return
	}
	r.SeasonClaimed = reached
	db.Save(r)
	grantCredits(u, u.Faction, cr)
	c.JSON(200, gin.H{"ok": true, "tier": reached, "credits": cr})
}
