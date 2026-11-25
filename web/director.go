package main

// Battle director (GAP_PLAN P1-A): prep window before a battle can be claimed,
// minimum-size check at the end of the prep window (undersized attack is called
// off, attacker retreats with a manpower penalty), and auto-resolve of battles
// nobody plays (never claimed, or server heartbeat lost): the stronger side wins.
// Called from doTick; caller holds gameMu.

import (
	"fmt"
	"time"
)

const (
	dirPrepMin          = 5  // minutes after creation before a server may claim the battle
	dirMinAttackMP      = 20 // attacker manpower needed for the battle to be worth playing
	dirRetreatPenalty   = 10 // % attacker manpower lost when an undersized attack is called off
	dirIdleResolveMin   = 60 // unclaimed battle auto-resolved this long after the prep window
	dirHeartbeatLostMin = 15 // claimed battle auto-resolved after this long without heartbeat
	dirAutoLossPct      = 20 // % manpower lost by the winner in an auto-resolve (loser: 2x)
)

// battleReadyAt : end of the prep window (battle claimable from then on).
func battleReadyAt(b *Battle) time.Time {
	return b.CreatedAt.Add(dirPrepMin * time.Minute)
}

func battleClaimable(b *Battle, now time.Time) bool {
	return b.Status == "pending" && !now.Before(battleReadyAt(b))
}

// battleAutoResolveAt : moment the director resolves the battle if nobody plays it.
func battleAutoResolveAt(b *Battle) time.Time {
	switch b.Status {
	case "starting", "running":
		t := b.CreatedAt
		if b.HeartbeatAt != nil {
			t = *b.HeartbeatAt
		} else if b.ClaimedAt != nil {
			t = *b.ClaimedAt
		}
		return t.Add(dirHeartbeatLostMin * time.Minute)
	}
	return battleReadyAt(b).Add(dirIdleResolveMin * time.Minute)
}

type dirAction int

const (
	dirNone dirAction = iota
	dirCancel
	dirResolve
)

// directorDecide : pure decision, unit-tested. strength = side -> combat value.
func directorDecide(b *Battle, strength map[string]int, now time.Time) dirAction {
	switch b.Status {
	case "pending":
		if now.Before(battleReadyAt(b)) {
			return dirNone
		}
		if strength[b.Attacker] < dirMinAttackMP {
			return dirCancel
		}
		if !now.Before(battleAutoResolveAt(b)) {
			return dirResolve
		}
	case "starting", "running":
		if !now.Before(battleAutoResolveAt(b)) {
			return dirResolve
		}
	}
	return dirNone
}

// autoWinner : stronger side wins, ties go to the defender.
func autoWinner(b *Battle, strength map[string]int) string {
	if strength[b.Attacker] > strength[b.Defender] {
		return b.Attacker
	}
	return b.Defender
}

func dirTeamStrength(t *AssaultTeam) int {
	s := t.Manpower
	for _, v := range t.GetVehicles() {
		s += 10 * v.Count
	}
	return s
}

func battleStrength(b *Battle) (map[string]int, map[string]int) {
	str, mp := map[string]int{}, map[string]int{}
	for side, ids := range b.GetSides() {
		for _, id := range ids {
			if t := teamByTeamID(id); t != nil {
				str[side] += dirTeamStrength(t)
				mp[side] += t.Manpower
			}
		}
	}
	str[b.Defender] += b.GarrisonMP
	return str, mp
}

func runDirector(w *War, now time.Time) {
	var bs []Battle
	db.Where("war_id = ? AND status IN ?", w.ID, []string{"pending", "starting", "running"}).Find(&bs)
	for i := range bs {
		b := &bs[i]
		str, _ := battleStrength(b)
		switch directorDecide(b, str, now) {
		case dirCancel:
			cancelUndersized(b)
		case dirResolve:
			autoResolve(b, str)
		}
	}
}

func cancelUndersized(b *Battle) {
	for _, ids := range b.GetSides() {
		for _, id := range ids {
			t := teamByTeamID(id)
			if t == nil || t.BattleID != b.ID {
				continue
			}
			upd := map[string]interface{}{"battle_id": 0, "dest_city_id": "", "status": "idle"}
			if t.Faction == b.Attacker {
				upd["manpower"] = t.Manpower - t.Manpower*dirRetreatPenalty/100
			}
			db.Model(&AssaultTeam{}).Where("id = ?", t.ID).Updates(upd)
		}
	}
	db.Model(&Battle{}).Where("id = ?", b.ID).Updates(map[string]interface{}{"status": "cancelled", "finished_at": time.Now().UTC()})
	logEvent(b.WarID, "battle", b.Attacker, fmt.Sprintf("Battle %s called off: attack force too small, %s falls back (-%d%% manpower)", b.BattleID(), b.Attacker, dirRetreatPenalty))
}

func autoResolve(b *Battle, str map[string]int) {
	win := autoWinner(b, str)
	r := &BattleResult{BattleID: b.BattleID(), Winner: win, Reason: "auto-resolved", Losses: map[string]ResLoss{}}
	for side, ids := range b.GetSides() {
		pct := dirAutoLossPct
		if side != win {
			pct *= 2
		}
		for _, id := range ids {
			if t := teamByTeamID(id); t != nil {
				r.Losses[id] = ResLoss{Manpower: t.Manpower * pct / 100}
			}
		}
	}
	if err := applyResult(b, r); err == nil {
		logEvent(b.WarID, "battle", win, fmt.Sprintf("Battle %s auto-resolved (no server played it): %s wins", b.BattleID(), win))
	}
}
