package main

// P2-C — strategic economy (docs/CAMPAIGN.md).
//   - Factories bank their production bonus in a local stockpile (City.Stockpile, capped)
//     instead of the national treasury; raising a team in that city draws on it first.
//   - Capturing a city seizes half its stockpile, the rest is lost.
//   - Neutral cities (owner "") rally to the first team that marches in (no battle).
//   - Capital-count victory: holding >= 3/4 of all capitals (min 2) wins — with the two
//     faction capitals only this is the historic "hold both capitals" rule; neutral
//     capitals make it a race.
//   - War end: every player who fought gets a payout (winner bonus / loser refund).

import (
	"fmt"
	"math"
)

const (
	stockpileCap      = 600
	stockpileSeizePct = 50
	warEndWinCredits  = 400
	warEndLossCredits = 150
	warEndPerBattle   = 20 // + per battle fought in this war, capped
	warEndBattleCap   = 20
)

// factoryShare : part of cityIncome banked locally instead of going to the faction
func factoryShare(c *City) int {
	if c.IsFactory && c.Owner != "" {
		return 30
	}
	return 0
}

func addStock(cur, add int) int {
	if cur+add > stockpileCap {
		return stockpileCap
	}
	return cur + add
}

// splitCost : how much of cost is paid from the local stockpile vs national funds
func splitCost(stock, cost int) (fromStock, fromFunds int) {
	if stock >= cost {
		return cost, 0
	}
	return stock, cost - stock
}

// produceStockpiles : called by applyIncome; returns per-faction amount to remove from the national income
func produceStockpiles(cities []City) map[string]int {
	held := map[string]int{}
	for i := range cities {
		c := &cities[i]
		if sh := factoryShare(c); sh > 0 {
			held[c.Owner] += sh
			c.Stockpile = addStock(c.Stockpile, sh)
			db.Model(&City{}).Where("war_id = ? AND id = ?", c.WarID, c.ID).Update("stockpile", c.Stockpile)
		}
	}
	return held
}

// payTeamCost : charges a new team on the city's stockpile then the faction funds; false if short
func payTeamCost(f *Faction, city *City, cost int) bool {
	st, fu := splitCost(city.Stockpile, cost)
	if f.Resources < fu {
		return false
	}
	if st > 0 {
		db.Model(&City{}).Where("war_id = ? AND id = ?", city.WarID, city.ID).Update("stockpile", city.Stockpile-st)
	}
	if fu > 0 {
		db.Model(f).Update("resources", f.Resources-fu)
	}
	return true
}

// seizeStockpile : on capture, half of the local stockpile goes to the captor's funds
func seizeStockpile(warID uint, cityID, captor string) {
	c := getCity(warID, cityID)
	if c == nil || c.Stockpile <= 0 {
		return
	}
	got := c.Stockpile * stockpileSeizePct / 100
	db.Model(&City{}).Where("war_id = ? AND id = ?", warID, cityID).Update("stockpile", 0)
	if got > 0 {
		db.Model(&Faction{}).Where("war_id = ? AND code = ?", warID, captor).Update("resources", gormExprAdd(got))
		logEvent(warID, "income", captor, fmt.Sprintf("Stockpile seized at %s: +%d resources", c.Name, got))
	}
}

// rallyNeutral : a team entering a neutral city takes it without a fight
func rallyNeutral(w *War, t *AssaultTeam, c *City) {
	db.Model(&City{}).Where("war_id = ? AND id = ?", w.ID, c.ID).Update("owner", t.Faction)
	t.CityID, t.DestCityID, t.Status = c.ID, "", "idle"
	db.Model(t).Updates(map[string]interface{}{"city_id": c.ID, "dest_city_id": "", "status": "idle"})
	logEvent(w.ID, "capture", t.Faction, fmt.Sprintf("%s rallies the neutral city of %s", t.Label(), c.Name))
	c.Owner = t.Faction
	checkVictory(w)
}

// capitalWinner : faction holding >= 3/4 (rounded up, min 2) of all capitals
func capitalWinner(capOwner map[string]string) string {
	n := len(capOwner)
	if n < 2 {
		return ""
	}
	need := int(math.Ceil(0.75 * float64(n)))
	if need < 2 {
		need = 2
	}
	cnt := map[string]int{}
	for _, o := range capOwner {
		cnt[o]++
	}
	for _, f := range []string{"US", "GER"} {
		if cnt[f] >= need {
			return f
		}
	}
	return ""
}

func warEndCredits(won bool, battles int) int {
	if battles > warEndBattleCap {
		battles = warEndBattleCap
	}
	v := warEndLossCredits
	if won {
		v = warEndWinCredits
	}
	return v + battles*warEndPerBattle
}

// grantCredits : adds credits to the player and his active soldier of that side
func grantCredits(p *Player, side string, n int) {
	db.Model(&Player{}).Where("id = ?", p.ID).Update("credits", gormExprCol("credits", n))
	if s := activeSoldierFor(p, []string{side}); s != nil {
		db.Model(&Soldier{}).Where("id = ?", s.ID).Update("credits", gormExprCol("credits", n))
	}
}

// warEndPayout : refund / bonus for everyone who fought in the war
func warEndPayout(w *War, winner string) {
	var rows []struct {
		PlayerID uint
		Side     string
		N        int
	}
	db.Model(&PlayerBattle{}).Select("player_battles.player_id, player_battles.side, count(*) as n").
		Joins("JOIN battles ON battles.id = player_battles.battle_id").Where("battles.war_id = ?", w.ID).
		Group("player_battles.player_id, player_battles.side").Scan(&rows)
	best := map[uint]int{}
	for i, r := range rows { // a player who switched sides is paid once, on his main side
		if j, ok := best[r.PlayerID]; !ok || r.N > rows[j].N {
			best[r.PlayerID] = i
		}
	}
	for _, i := range best {
		r := rows[i]
		var p Player
		if db.First(&p, r.PlayerID).Error != nil {
			continue
		}
		grantCredits(&p, r.Side, warEndCredits(r.Side == winner, r.N))
	}
	logEvent(w.ID, "war", winner, fmt.Sprintf("War end payout to %d veterans", len(best)))
}
