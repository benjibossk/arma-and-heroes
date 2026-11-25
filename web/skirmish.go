package main

// P2-B — road skirmishes (docs/CAMPAIGN.md).
// Two hostile teams travelling the same road in opposite directions meet half-way: when
// the first one reaches the end of the road while the other is still en route, both stop
// and fight a SKIRMISH battle (mod mode "SKIRMISH", no garrison, no city capture).
// Played on the scenario of the city the arriving team was heading to.
// Outcome: the winner resumes its march (arrives on the next tick), the loser falls back
// to its city of origin; a team reduced to 0 manpower is destroyed as usual.

import (
	"fmt"
	"time"
)

const skirmishTimeLimitMin = 30

// meetsOnRoad : b travels the same road as a, in the opposite direction, for another faction
func meetsOnRoad(a, b *AssaultTeam) bool {
	return a.Faction != b.Faction && b.Status == "moving" && a.CityID != "" &&
		b.CityID == a.DestCityID && b.DestCityID == a.CityID
}

// startSkirmish : called by arrive() before the normal arrival; true if a skirmish started
func startSkirmish(w *War, t *AssaultTeam) bool {
	var cands []AssaultTeam
	db.Where("war_id = ? AND status = ? AND city_id = ? AND dest_city_id = ? AND faction <> ?",
		w.ID, "moving", t.DestCityID, t.CityID, t.Faction).Order("arrival_at").Find(&cands)
	var foe *AssaultTeam
	for i := range cands {
		if meetsOnRoad(t, &cands[i]) {
			foe = &cands[i]
			break
		}
	}
	if foe == nil || activeBattleAt(w.ID, t.DestCityID) != nil {
		return false
	}
	rd := roadBetween(w.ID, t.CityID, t.DestCityID)
	if rd == nil {
		return false
	}
	b := Battle{WarID: w.ID, Status: "pending", CityID: t.DestCityID, Mode: "SKIRMISH", RoadID: rd.ID,
		Attacker: t.Faction, Defender: foe.Faction, TimeLimitMin: skirmishTimeLimitMin, CreatedAt: time.Now().UTC()}
	b.SetSides(SideTeams{t.Faction: {}, foe.Faction: {}})
	db.Create(&b)
	db.Model(foe).Updates(map[string]interface{}{"arrival_at": nil, "depart_at": nil})
	addTeamToBattle(&b, t, t.Faction)
	addTeamToBattle(&b, foe, foe.Faction)
	a, z := getCity(w.ID, t.CityID), getCity(w.ID, t.DestCityID)
	an, zn := t.CityID, t.DestCityID
	if a != nil && z != nil {
		an, zn = a.Name, z.Name
	}
	logEvent(w.ID, "battle", t.Faction, fmt.Sprintf("Battle %s: road skirmish between %s and %s on the %s - %s road", b.BattleID(), t.Label(), foe.Label(), an, zn))
	return true
}

// skirmishExit : status update of a team leaving a road skirmish (manpower > 0)
func skirmishExit(won bool, now time.Time) map[string]interface{} {
	if won { // keep dest_city_id: arrive() fires on the next tick
		return map[string]interface{}{"battle_id": 0, "status": "moving", "depart_at": now, "arrival_at": now}
	}
	return map[string]interface{}{"battle_id": 0, "status": "idle", "dest_city_id": ""}
}
