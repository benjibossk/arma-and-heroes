package main

import (
	"time"
)

// Morale et expérience des équipes d'assaut (P1-B).
// Moral : départ 100, coût par bataille, bonus de victoire, régénération
// paresseuse (calculée au moment de la bataille depuis MoraleAt), déroute
// sous un seuil. XP : gagnée en victoire comme en défaite, seulement si la
// bataille a duré assez longtemps (seuil de participation).
const (
	moraleMax         = 100
	moraleBattleCost  = 15
	moraleWinBonus    = 10
	moraleLossPenalty = 10
	moraleRegenPerH   = 5
	moraleRoutBelow   = 25
	routDesertionPct  = 10 // % d'effectif perdu en déroute

	xpWin          = 100
	xpLoss         = 40
	xpMinDuration  = 300 // s : en dessous, pas d'XP (participation)
	xpPerLevelBase = 200 // niveau n -> n+1 coûte base*n
	teamMaxLevel   = 10
)

// regenMorale applique la régénération depuis last jusqu'à now.
func regenMorale(cur int, last *time.Time, now time.Time) int {
	if last != nil && now.After(*last) {
		cur += int(now.Sub(*last).Hours() * moraleRegenPerH)
	}
	if cur > moraleMax {
		cur = moraleMax
	}
	return cur
}

// moraleAfterBattle renvoie le nouveau moral et si l'équipe est en déroute.
func moraleAfterBattle(cur int, won bool) (int, bool) {
	cur -= moraleBattleCost
	if won {
		cur += moraleWinBonus
	} else {
		cur -= moraleLossPenalty
	}
	if cur > moraleMax {
		cur = moraleMax
	}
	if cur < 0 {
		cur = 0
	}
	return cur, !won && cur < moraleRoutBelow
}

// addTeamXP ajoute l'XP et renvoie (xp, niveau).
func addTeamXP(xp, level int, won bool, durationSec int) (int, int) {
	if level < 1 {
		level = 1
	}
	if durationSec < xpMinDuration {
		return xp, level
	}
	if won {
		xp += xpWin
	} else {
		xp += xpLoss
	}
	for level < teamMaxLevel && xp >= xpPerLevelBase*level {
		xp -= xpPerLevelBase * level
		level++
	}
	return xp, level
}

// adjustMorale modifie le moral d'une équipe (utilisable par le directeur).
func adjustMorale(t *AssaultTeam, delta int, now time.Time) {
	m := regenMorale(t.Morale, t.MoraleAt, now) + delta
	if m < 0 {
		m = 0
	}
	if m > moraleMax {
		m = moraleMax
	}
	t.Morale, t.MoraleAt = m, &now
	db.Model(&AssaultTeam{}).Where("id = ?", t.ID).Updates(map[string]interface{}{"morale": m, "morale_at": now})
}

// applyMoraleXP : appelé par applyResult après la sortie des équipes.
func applyMoraleXP(b *Battle, r *BattleResult, now time.Time) {
	for side, ids := range b.GetSides() {
		won := side == r.Winner
		for _, id := range ids {
			t := teamByTeamID(id)
			if t == nil || t.Status == "destroyed" {
				continue
			}
			m, routed := moraleAfterBattle(regenMorale(t.Morale, t.MoraleAt, now), won)
			xp, lvl := addTeamXP(t.TeamXP, t.TeamLevel, won, r.DurationSec)
			upd := map[string]interface{}{"morale": m, "morale_at": now, "team_xp": xp, "team_level": lvl, "routed": routed}
			if routed {
				upd["manpower"] = t.Manpower - t.Manpower*routDesertionPct/100
			}
			db.Model(&AssaultTeam{}).Where("id = ?", t.ID).Updates(upd)
		}
	}
}
