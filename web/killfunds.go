package main

// P2-E per-kill war funds (GAP_PLAN.md). The mod reports, per killer team, the enemy players / bots killed and the
// enemy pool vehicles destroyed (BattleResult.teamKills, F44_Rules.c). Each kill pays the killer team's faction
// treasury (the commanders' war funds) the value of the destroyed resource. A vehicle whose killer team is unknown
// is reported under "_side_<SIDE>" and pays that side.

import (
	"strconv"
	"strings"
)

// VictimKills : one entry of BattleResult.teamKills
type VictimKills struct {
	Players  int            `json:"players"`
	Bots     int            `json:"bots"`
	Vehicles map[string]int `json:"vehicles"`
}

// war funds per destroyed resource (design choice: an infantryman is worth ~ team cost / manpower,
// bots half of it, vehicles by role)
var killFundValues = struct {
	Player, Bot int
	Role        map[string]int
	DefaultVeh  int
	BattleCap   int // max paid to one faction per battle (anti-farm)
}{
	Player: 10, Bot: 5,
	Role:       map[string]int{"tank": 80, "light_armored": 40, "light_armored_aa": 40, "truck": 15, "light_utility": 10, "light_armed": 20},
	DefaultVeh: 15,
	BattleCap:  1500,
}

// killFundsFor : war funds earned by one teamKills entry
func killFundsFor(k VictimKills) int {
	v := k.Players*killFundValues.Player + k.Bots*killFundValues.Bot
	for key, n := range k.Vehicles {
		pay := killFundValues.DefaultVeh
		if p, ok := killFundValues.Role[teamRules.VehicleRoles[key]]; ok {
			pay = p
		}
		v += n * pay
	}
	return v
}

// killFundsSide : faction code that receives the funds of a teamKills key ("at_12" or "_side_US")
func killFundsSide(b *Battle, key string) string {
	if strings.HasPrefix(key, "_side_") {
		return key[len("_side_"):]
	}
	if strings.HasPrefix(key, "at_") {
		id, err := strconv.Atoi(key[3:])
		if err != nil {
			return ""
		}
		var t AssaultTeam
		if db.Select("faction").Where("id = ? AND war_id = ?", id, b.WarID).First(&t).Error == nil {
			return t.Faction
		}
	}
	return ""
}

// applyKillFunds : caller holds gameMu (called from applyWarFunds)
func applyKillFunds(b *Battle, r *BattleResult) {
	total := map[string]int{}
	for key, k := range r.TeamKills {
		side := killFundsSide(b, key)
		if side != b.Attacker && side != b.Defender {
			continue
		}
		total[side] += killFundsFor(k)
	}
	for side, v := range total {
		if v > killFundValues.BattleCap {
			v = killFundValues.BattleCap
		}
		if v > 0 {
			db.Model(&Faction{}).Where("war_id = ? AND code = ?", b.WarID, side).Update("resources", gormExprAdd(v))
		}
	}
}
