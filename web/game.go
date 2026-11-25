package main

import (
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"math"
	"time"

	"gorm.io/gorm"
)

func otherSide(f string) string {
	if f == "US" {
		return "GER"
	}
	return "US"
}

func cityIncome(c *City) int {
	v := 10
	if c.IsFactory {
		v += 30
	}
	if c.IsCapital {
		v += 20
	}
	return v
}

func tickLoop() {
	t := time.NewTicker(time.Duration(cfg.TickSec) * time.Second)
	for range t.C {
		func() {
			defer func() {
				if r := recover(); r != nil {
					log.Println("tick panic:", r)
				}
			}()
			gameMu.Lock()
			defer gameMu.Unlock()
			doTick(false)
		}()
	}
}

// doTick : revenus (si l'intervalle est écoulé ou force) + arrivées des AT. Appelant tient gameMu.
func doTick(forceIncome bool) {
	w := currentWar()
	if w == nil || w.Status != "active" {
		return
	}
	now := time.Now().UTC()
	if forceIncome || now.Sub(w.LastIncomeAt) >= time.Duration(cfg.IncomeMin)*time.Minute {
		applyIncome(w)
		w.LastIncomeAt = now
		db.Model(w).Update("last_income_at", now)
	}
	var moving []AssaultTeam
	db.Where("war_id = ? AND status = ? AND arrival_at <= ?", w.ID, "moving", now).Order("arrival_at").Find(&moving)
	for i := range moving {
		arrive(w, &moving[i])
	}
	runDirector(w, now)
}

func applyIncome(w *War) {
	var cities []City
	db.Where("war_id = ?", w.ID).Find(&cities)
	inc := map[string]int{}
	for i := range cities {
		inc[cities[i].Owner] += cityIncome(&cities[i])
	}
	for f, v := range inc {
		if f == "" {
			continue
		}
		db.Model(&Faction{}).Where("war_id = ? AND code = ?", w.ID, f).Update("resources", gorm.Expr("resources + ?", v))
		logEvent(w.ID, "income", f, fmt.Sprintf("Supply: +%d resources", v))
	}
}

func activeBattleAt(warID uint, cityID string) *Battle {
	var b Battle
	if db.Where("war_id = ? AND city_id = ? AND status IN ?", warID, cityID, []string{"pending", "starting", "running"}).First(&b).Error != nil {
		return nil
	}
	return &b
}

func getCity(warID uint, id string) *City {
	var c City
	if db.Where("war_id = ? AND id = ?", warID, id).First(&c).Error != nil {
		return nil
	}
	return &c
}

func addTeamToBattle(b *Battle, t *AssaultTeam, side string) {
	s := b.GetSides()
	for _, id := range s[side] {
		if id == t.TeamID() {
			return
		}
	}
	s[side] = append(s[side], t.TeamID())
	b.SetSides(s)
	db.Model(b).Update("sides", b.Sides)
	t.Status = "in_battle"
	t.BattleID = b.ID
	db.Model(t).Updates(map[string]interface{}{"status": "in_battle", "battle_id": b.ID})
}

func arrive(w *War, t *AssaultTeam) {
	dest := getCity(w.ID, t.DestCityID)
	if dest == nil {
		db.Model(t).Updates(map[string]interface{}{"status": "idle", "dest_city_id": "", "arrival_at": nil, "depart_at": nil})
		return
	}
	db.Model(t).Updates(map[string]interface{}{"arrival_at": nil, "depart_at": nil})
	if b := activeBattleAt(w.ID, dest.ID); b != nil {
		// renforts : l'AT rejoint son camp dans la bataille en cours
		if t.Faction == b.Defender {
			db.Model(t).Updates(map[string]interface{}{"city_id": dest.ID, "dest_city_id": ""})
			t.CityID = dest.ID
		}
		addTeamToBattle(b, t, t.Faction)
		logEvent(w.ID, "battle", t.Faction, fmt.Sprintf("%s (%s) arrives as reinforcement at %s", t.Label(), t.Type, dest.Name))
		return
	}
	if dest.Owner == t.Faction {
		t.CityID, t.DestCityID, t.Status = dest.ID, "", "idle"
		db.Model(t).Updates(map[string]interface{}{"city_id": dest.ID, "dest_city_id": "", "status": "idle"})
		mergeTeam(w, t, dest)
		return
	}
	// ville ennemie : création de bataille. L'attaquant garde city_id = ville d'origine.
	b := Battle{WarID: w.ID, Status: "pending", CityID: dest.ID, Mode: dest.Mode, Attacker: t.Faction, Defender: dest.Owner, TimeLimitMin: 45, CreatedAt: time.Now().UTC()}
	b.SetSides(SideTeams{t.Faction: {}, dest.Owner: {}})
	var defenders []AssaultTeam
	db.Where("war_id = ? AND city_id = ? AND faction = ? AND status = ?", w.ID, dest.ID, dest.Owner, "idle").Find(&defenders)
	if len(defenders) == 0 {
		b.GarrisonMP = dest.Garrison
	}
	db.Create(&b)
	addTeamToBattle(&b, t, t.Faction)
	for i := range defenders {
		addTeamToBattle(&b, &defenders[i], dest.Owner)
	}
	logEvent(w.ID, "battle", t.Faction, fmt.Sprintf("Battle %s: %s attacks %s (%s)", b.BattleID(), t.Faction, dest.Name, dest.Owner))
}

// mergeTeam : fusionne une AT arrivée avec une AT alliée de même type et même général dans la ville
func mergeTeam(w *War, t *AssaultTeam, c *City) {
	var o AssaultTeam
	if db.Where("war_id = ? AND city_id = ? AND faction = ? AND type = ? AND owner_id = ? AND status = ? AND id <> ?",
		w.ID, c.ID, t.Faction, t.Type, t.OwnerID, "idle", t.ID).First(&o).Error != nil {
		logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s (%s) arrives at %s", t.Label(), t.Type, c.Name))
		return
	}
	o.Manpower += t.Manpower
	vm := map[string]int{}
	for _, v := range o.GetVehicles() {
		vm[v.Key] += v.Count
	}
	for _, v := range t.GetVehicles() {
		vm[v.Key] += v.Count
	}
	var vs []VehicleCount
	for k, n := range vm {
		vs = append(vs, VehicleCount{k, n})
	}
	o.SetVehicles(filterVehicles(o.Type, vs))
	if t.EquipLevel > o.EquipLevel {
		o.EquipLevel = t.EquipLevel
	}
	db.Save(&o)
	db.Model(t).Updates(map[string]interface{}{"status": "dissolved", "manpower": 0})
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s merges into %s at %s", t.Label(), o.Label(), c.Name))
}

func roadBetween(warID uint, a, b string) *Road {
	var r Road
	if db.Where("war_id = ? AND ((city_a = ? AND city_b = ?) OR (city_a = ? AND city_b = ?))", warID, a, b, b, a).First(&r).Error != nil {
		return nil
	}
	return &r
}

func travelDuration(km int) time.Duration {
	return time.Duration(math.Round(float64(km)*cfg.TravelMinPerKm*60)) * time.Second
}

// ---------- BattleConfig §3 ----------

type BCTeam struct {
	TeamID      string         `json:"teamId"`
	DisplayName string         `json:"displayName"`
	Type        string         `json:"type"`
	Manpower    int            `json:"manpower"`
	EquipLevel  int            `json:"equipLevel"`
	Classes     []string       `json:"classes"`
	Vehicles    []BCVehicle    `json:"vehicles"`
}

type BCSide struct {
	Teams []BCTeam `json:"teams"`
	AI    AIConfig `json:"ai"`
}

type BattleConfig struct {
	BattleID       string                   `json:"battleId"`
	WarID          uint                     `json:"warId"`
	CityID         string                   `json:"cityId"`
	Mode           string                   `json:"mode"`
	Scenario       string                   `json:"scenario"`
	TimeLimitMin   int                      `json:"timeLimitMin"`
	Attacker       string                   `json:"attacker"`
	Defender       string                   `json:"defender"`
	Sides          map[string]BCSide        `json:"sides"`
	PlayerUnlocks  map[string][]string      `json:"playerUnlocks"`
	PlayerLoadouts map[string]PlayerLoadout `json:"playerLoadouts"`
	SpawnTimes     BCSpawnTimes             `json:"spawnTimes"`
}

// BCSpawnTimes: respawn delay model read by the mod (F44_SpawnTimesConfig, P1-C).
// Player delay = baseSec + min(points*perPointSec, maxEquipSec) - defenderBonusSec (>= minSec);
// forward sector point adds forwardPenaltySec + links from own HQ * perLinkSec.
type BCSpawnTimes struct {
	BaseSec           float64 `json:"baseSec"`
	ForwardPenaltySec float64 `json:"forwardPenaltySec"`
	PerPointSec       float64 `json:"perPointSec"`
	MaxEquipSec       float64 `json:"maxEquipSec"`
	PerLinkSec        float64 `json:"perLinkSec"`
	DefenderBonusSec  float64 `json:"defenderBonusSec"`
	MinSec            float64 `json:"minSec"`
}

var defaultSpawnTimes = BCSpawnTimes{BaseSec: 10, ForwardPenaltySec: 10, PerPointSec: 0.5, MaxEquipSec: 20, PerLinkSec: 3, DefenderBonusSec: 3, MinSec: 3}

func teamByTeamID(id string) *AssaultTeam {
	var n int
	if _, err := fmt.Sscanf(id, "at_%d", &n); err != nil {
		return nil
	}
	var t AssaultTeam
	if db.First(&t, n).Error != nil {
		return nil
	}
	return &t
}

func garrisonName(b *Battle) string {
	name := b.CityID
	if c := getCity(b.WarID, b.CityID); c != nil {
		name = c.Name
	}
	return garrisonDisplayName(name, b.Defender)
}

func buildConfig(b *Battle) BattleConfig {
	c := getCity(b.WarID, b.CityID)
	scen := ""
	if c != nil {
		scen = c.ScenarioID
	}
	out := BattleConfig{BattleID: b.BattleID(), WarID: b.WarID, CityID: b.CityID, Mode: b.Mode, Scenario: scen,
		TimeLimitMin: b.TimeLimitMin, Attacker: b.Attacker, Defender: b.Defender, Sides: map[string]BCSide{}, PlayerUnlocks: map[string][]string{}, PlayerLoadouts: battleLoadouts(b), SpawnTimes: defaultSpawnTimes}
	for side, ids := range b.GetSides() {
		s := BCSide{Teams: []BCTeam{}}
		for _, id := range ids {
			t := teamByTeamID(id)
			if t == nil {
				continue
			}
			s.Teams = append(s.Teams, BCTeam{TeamID: id, DisplayName: t.DisplayName, Type: t.Type, Manpower: t.Manpower, EquipLevel: t.EquipLevel,
				Classes: atSpecs[t.Type].Classes, Vehicles: battleVehicles(t)})
		}
		out.Sides[side] = s
	}
	if b.GarrisonMP > 0 {
		s := out.Sides[b.Defender]
		s.Teams = append(s.Teams, BCTeam{TeamID: "garrison_" + b.CityID, DisplayName: garrisonName(b), Type: "INF", Manpower: b.GarrisonMP, EquipLevel: 1,
			Classes: []string{"RIFLEMAN", "MG"}, Vehicles: []BCVehicle{}})
		out.Sides[b.Defender] = s
	}
	ai := loadAI()
	for _, f := range []string{"US", "GER"} {
		s, ok := out.Sides[f]
		if !ok {
			s = BCSide{Teams: []BCTeam{}}
		}
		var types []string
		for _, t := range s.Teams {
			types = append(types, t.Type)
		}
		s.AI = sideAI(ai, types)
		out.Sides[f] = s
	}
	return out
}

// ---------- BattleResult §4 ----------

type ResLoss struct {
	Manpower int            `json:"manpower"`
	Vehicles map[string]int `json:"vehicles"`
}

type ResPlayer struct {
	IdentityID string `json:"identityId"`
	Name       string `json:"name"`
	Side       string `json:"side"`
	TeamID     string `json:"teamId"`
	Kills      int    `json:"kills"`
	Deaths     int    `json:"deaths"`
	Captures   int    `json:"captures"`
	XP         int    `json:"xp"`
	SoldierID  uint   `json:"soldierId,omitempty"`
	// per weapon category (rifle, smg, mg, sniper, at, grenade, explosive, pistol, vehicle...) for ribbons
	WeaponStats     map[string]WeaponStat `json:"weaponStats,omitempty"`
	KillsByCategory map[string]int        `json:"killsByCategory,omitempty"` // what the mod actually sends
	// optional finer stats (progression.json); absent -> kills counted as AI kills
	PlayerKills  int `json:"playerKills,omitempty"`
	AIKillCount  int `json:"aiKills,omitempty"`
	Assists      int `json:"assists,omitempty"`
	VehicleKills int `json:"vehicleKills,omitempty"`
	Defends      int `json:"defends,omitempty"`
	ObjectiveSec int `json:"objectiveSec,omitempty"`
	PresenceSec  int `json:"presenceSec,omitempty"`
	SquadSec     int `json:"squadSec,omitempty"`
	// ribbon counters (docs/RIBBONS.md)
	PlayerKillsByCategory map[string]int `json:"playerKillsByCategory,omitempty"`
	Headshots             int            `json:"headshots,omitempty"`
	MultiKills            int            `json:"multiKills,omitempty"`
	ResupplyGiven         int            `json:"resupplyGiven,omitempty"`
	HealGiven             int            `json:"healGiven,omitempty"`
	Revives               int            `json:"revives,omitempty"`
	Repairs               int            `json:"repairs,omitempty"`
	Spots                 int            `json:"spots,omitempty"`
	TransportPassengers   int            `json:"transportPassengers,omitempty"`
	Stats                 map[string]int `json:"stats,omitempty"`
}

type WeaponStat struct {
	Kills int `json:"kills"`
}

type BattleResult struct {
	BattleID    string                       `json:"battleId"`
	Winner      string                       `json:"winner"`
	Reason      string                       `json:"reason"`
	DurationSec int                          `json:"durationSec"`
	Losses      map[string]ResLoss           `json:"losses"`
	Players     []ResPlayer                  `json:"players"`
	AIKills     map[string]int               `json:"aiKills,omitempty"`  // per side, display only
	AIDeaths    map[string]int               `json:"aiDeaths,omitempty"` // per side, display only (already included in losses.manpower)
	TeamAIKills map[string]int               `json:"teamAiKills,omitempty"`
	AISkill     map[string]map[string]string `json:"aiSkill,omitempty"` // side -> {final, avg}
}

// applyResult : appelant tient gameMu
func applyResult(b *Battle, r *BattleResult) error {
	if b.Status == "finished" || b.Status == "cancelled" {
		return errors.New("battle already finished")
	}
	if r.Winner != b.Attacker && r.Winner != b.Defender {
		return errors.New("invalid winner")
	}
	raw, _ := json.Marshal(r)
	now := time.Now().UTC()
	w := War{}
	db.First(&w, b.WarID)
	city := getCity(b.WarID, b.CityID)
	// pertes
	for tid, l := range r.Losses {
		t := teamByTeamID(tid)
		if t == nil || t.BattleID != b.ID {
			continue
		}
		t.Manpower -= l.Manpower
		vs := t.GetVehicles()
		for i := range vs {
			vs[i].Count -= l.Vehicles[vs[i].Key]
		}
		t.SetVehicles(vs)
		if t.Manpower < 0 {
			t.Manpower = 0
		}
		db.Model(t).Updates(map[string]interface{}{"manpower": t.Manpower, "vehicles": t.Vehicles})
	}
	// sort des AT engagées
	sides := b.GetSides()
	for side, ids := range sides {
		for _, id := range ids {
			t := teamByTeamID(id)
			if t == nil {
				continue
			}
			upd := map[string]interface{}{"battle_id": 0, "dest_city_id": ""}
			switch {
			case t.Manpower <= 0:
				upd["status"] = "destroyed"
			case side == r.Winner && side == b.Attacker:
				upd["status"], upd["city_id"] = "idle", b.CityID
			case side == r.Winner:
				upd["status"] = "idle"
			case side == b.Attacker: // attaquant repoussé : reste dans sa ville d'origine
				upd["status"] = "idle"
			default: // défenseur battu : retraite vers une ville alliée adjacente
				if dst := retreatCity(b.WarID, b.CityID, side); dst != "" {
					upd["status"], upd["city_id"] = "idle", dst
				} else {
					upd["status"] = "destroyed"
				}
			}
			db.Model(&AssaultTeam{}).Where("id = ?", t.ID).Updates(upd)
		}
	}
	applyMoraleXP(b, r, now)
	cname := b.CityID
	if city != nil {
		cname = city.Name
	}
	if r.Winner == b.Attacker && city != nil {
		db.Model(&City{}).Where("war_id = ? AND id = ?", b.WarID, b.CityID).Update("owner", b.Attacker)
		logEvent(b.WarID, "capture", b.Attacker, fmt.Sprintf("%s captures %s (%s)", b.Attacker, cname, r.Reason))
	} else {
		logEvent(b.WarID, "battle", b.Defender, fmt.Sprintf("%s holds %s against %s (%s)", b.Defender, cname, b.Attacker, r.Reason))
	}
	// joueurs
	for _, rp := range r.Players {
		if rp.IdentityID == "" {
			continue
		}
		var p Player
		// only linked accounts are credited; unlinked identities still count in team losses
		if db.Where("identity_id = ?", rp.IdentityID).First(&p).Error != nil {
			continue
		}
		won := rp.Side == r.Winner
		rw := computeReward(&rp, won, won && !hasWonToday(p.ID))
		xp := rw.XP
		p.XP += xp
		p.Credits += rw.Credits
		p.Kills += rp.Kills
		p.Deaths += rp.Deaths
		p.Captures += rp.Captures
		p.Battles++
		db.Save(&p)
		creditSoldier(&p, b, &rp, xp, rw.Credits)
		db.Create(&PlayerBattle{PlayerID: p.ID, BattleID: b.ID, Side: rp.Side, TeamID: rp.TeamID, Kills: rp.Kills, Deaths: rp.Deaths, Captures: rp.Captures, XP: xp, Credits: rw.Credits, Won: won, CreatedAt: now})
	}
	applyWarFunds(b, r)
	db.Model(b).Updates(map[string]interface{}{"status": "finished", "winner": r.Winner, "result": string(raw), "finished_at": now})
	checkVictory(&w)
	return nil
}

func retreatCity(warID uint, from, faction string) string {
	var roads []Road
	db.Where("war_id = ? AND (city_a = ? OR city_b = ?)", warID, from, from).Order("distance").Find(&roads)
	for _, r := range roads {
		o := r.CityA
		if o == from {
			o = r.CityB
		}
		if c := getCity(warID, o); c != nil && c.Owner == faction && activeBattleAt(warID, o) == nil {
			return o
		}
	}
	return ""
}

func checkVictory(w *War) {
	if w.Status != "active" {
		return
	}
	var cities []City
	db.Where("war_id = ?", w.ID).Find(&cities)
	held := map[string]int{}
	capOwner := map[string]string{} // capitale initiale -> faction qui la détient
	for _, c := range cities {
		held[c.Owner]++
		if c.IsCapital {
			capOwner[c.ID] = c.Owner
		}
	}
	winner := ""
	for _, f := range []string{"US", "GER"} {
		if float64(held[f]) >= 0.7*float64(len(cities)) {
			winner = f
		}
	}
	// capitale adverse prise : la faction détient les deux capitales
	for _, f := range []string{"US", "GER"} {
		n := 0
		for _, o := range capOwner {
			if o == f {
				n++
			}
		}
		if len(capOwner) >= 2 && n == len(capOwner) {
			winner = f
		}
	}
	if winner != "" {
		now := time.Now().UTC()
		db.Model(w).Updates(map[string]interface{}{"status": "finished", "winner": winner, "ended_at": now})
		logEvent(w.ID, "war", winner, "VICTORY: "+winner+" wins the war ("+w.Name+")")
	}
}

func gormExprAdd(v int) interface{} { return gorm.Expr("resources + ?", v) }

func gormExprCol(col string, v int) interface{} { return gorm.Expr(col+" + ?", v) }
