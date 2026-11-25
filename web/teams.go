package main

// Assault-team rules: vehicles allowed per team type, strength, resupply (tunable in teams.json).

import (
	_ "embed"
	"encoding/json"
	"fmt"
	"log"
	"math"
	"os"

	"github.com/gin-gonic/gin"
)

//go:embed teams.json
var teamsRaw []byte

type TeamRules struct {
	ResupplyFactor float64             `json:"resupplyFactor"`
	MinAttackPct   int                 `json:"minAttackPct"`
	AllowedRoles   map[string][]string `json:"allowedRoles"`
	VehicleShare   map[string]float64  `json:"vehicleShare"`
	VehicleRoles   map[string]string   `json:"vehicleRoles"`
}

var teamRules TeamRules

func loadTeamRules() {
	raw := teamsRaw
	if b, err := os.ReadFile("teams.json"); err == nil {
		raw = b
	}
	if err := json.Unmarshal(raw, &teamRules); err != nil {
		log.Fatal("teams.json: ", err)
	}
}

// vehicleAllowed : unknown vehicle keys are refused (no role = no tank sneaking into an INF team)
func vehicleAllowed(teamType, key string) bool {
	role, ok := teamRules.VehicleRoles[key]
	if !ok {
		return false
	}
	for _, r := range teamRules.AllowedRoles[teamType] {
		if r == role {
			return true
		}
	}
	return false
}

func filterVehicles(teamType string, vs []VehicleCount) []VehicleCount {
	out := []VehicleCount{}
	for _, v := range vs {
		if v.Count > 0 && vehicleAllowed(teamType, v.Key) {
			out = append(out, v)
		}
	}
	return out
}

type TeamStrength struct {
	Pct          int  `json:"strengthPct"`
	ResupplyCost int  `json:"resupplyCost"`
	CanAttack    bool `json:"canAttack"`
}

// teamStrength : value of what the team still has vs a full team of its type (pro-rata of the buy cost)
func teamStrength(t *AssaultTeam) TeamStrength {
	spec := atSpecs[t.Type]
	full := map[string]int{}
	totalVeh := 0
	for _, v := range filterVehicles(t.Type, spec.Vehicles[t.Faction]) {
		full[v.Key] += v.Count
		totalVeh += v.Count
	}
	vshare := teamRules.VehicleShare[t.Type]
	if totalVeh == 0 {
		vshare = 0
	}
	cost := float64(spec.Cost)
	missing := 0.0
	if spec.Manpower > 0 && t.Manpower < spec.Manpower {
		missing += cost * (1 - vshare) * float64(spec.Manpower-t.Manpower) / float64(spec.Manpower)
	}
	have := map[string]int{}
	for _, v := range t.GetVehicles() {
		have[v.Key] += v.Count
	}
	for k, n := range full {
		if have[k] < n {
			missing += cost * vshare * float64(n-have[k]) / float64(totalVeh)
		}
	}
	pct := 100
	if cost > 0 {
		pct = int(math.Round(100 * (1 - missing/cost)))
	}
	return TeamStrength{Pct: pct, ResupplyCost: int(math.Ceil(missing * teamRules.ResupplyFactor)), CanAttack: pct >= teamRules.MinAttackPct}
}

// resupplyTeam : POST /api/teams/:id/resupply — refill manpower and vehicles to the type's full strength
func resupplyTeam(c *gin.Context) {
	u := user(c)
	gameMu.Lock()
	defer gameMu.Unlock()
	w := currentWar()
	var t AssaultTeam
	if w == nil || db.Where("war_id = ?", w.ID).First(&t, c.Param("id")).Error != nil {
		errJ(c, 404, "assault team not found")
		return
	}
	if !ownsTeam(u, &t) {
		errJ(c, 403, "this assault team is not yours")
		return
	}
	if t.Status != "idle" {
		errJ(c, 400, "only an idle assault team can be resupplied")
		return
	}
	city := getCity(w.ID, t.CityID)
	if city == nil || city.Owner != t.Faction || activeBattleAt(w.ID, city.ID) != nil {
		errJ(c, 400, "resupply needs a friendly city that is not under attack")
		return
	}
	st := teamStrength(&t)
	if st.ResupplyCost <= 0 {
		errJ(c, 400, "assault team already at full strength")
		return
	}
	var f Faction
	if db.Where("war_id = ? AND code = ?", w.ID, t.Faction).First(&f).Error != nil || f.Resources < st.ResupplyCost {
		errJ(c, 400, "not enough resources")
		return
	}
	spec := atSpecs[t.Type]
	have := map[string]int{}
	for _, v := range t.GetVehicles() {
		have[v.Key] += v.Count
	}
	for _, v := range filterVehicles(t.Type, spec.Vehicles[t.Faction]) {
		if have[v.Key] < v.Count {
			have[v.Key] = v.Count
		}
	}
	var vs []VehicleCount
	for k, n := range have {
		vs = append(vs, VehicleCount{k, n})
	}
	t.SetVehicles(filterVehicles(t.Type, vs))
	t.Manpower = max(t.Manpower, spec.Manpower)
	db.Model(&f).Update("resources", f.Resources-st.ResupplyCost)
	db.Model(&t).Updates(map[string]interface{}{"manpower": t.Manpower, "vehicles": t.Vehicles})
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s resupplies %s (-%d resources)", u.Name, t.Label(), st.ResupplyCost))
	c.JSON(200, gin.H{"ok": true, "cost": st.ResupplyCost})
}
