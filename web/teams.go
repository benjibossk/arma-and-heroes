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
	Crates         map[string]CrateRule   `json:"crates"`
	Variants       map[string]VariantRule `json:"variants"`
}

// CrateRule : supply crate a general can fit on a team vehicle (mod F44_FieldSupply.AttachToVehicle)
type CrateRule struct {
	Label string `json:"label"`
	Cost  int    `json:"cost"`
}

// VariantRule : unarmed / armed prefab pair of the same model (inventory/VEHICLE_VARIANTS.md)
type VariantRule struct {
	Base       string `json:"base"`
	Armed      string `json:"armed"`
	ArmedLabel string `json:"armedLabel"`
	ArmedCost  int    `json:"armedCost"`
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
	if maxMP := teamMaxManpower(t); maxMP > 0 && t.Manpower < maxMP {
		missing += cost * (1 - vshare) * float64(maxMP-t.Manpower) / float64(maxMP)
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
	_, disc := upgradeTotals(t.GetUpgrades())
	return TeamStrength{Pct: pct, ResupplyCost: int(math.Ceil(missing * teamRules.ResupplyFactor * float64(100-disc) / 100)), CanAttack: pct >= teamRules.MinAttackPct}
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
	t.Manpower = max(t.Manpower, teamMaxManpower(&t))
	db.Model(&f).Update("resources", f.Resources-st.ResupplyCost)
	db.Model(&t).Updates(map[string]interface{}{"manpower": t.Manpower, "vehicles": t.Vehicles})
	logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s resupplies %s (-%d resources)", u.Name, t.Label(), st.ResupplyCost))
	c.JSON(200, gin.H{"ok": true, "cost": st.ResupplyCost})
}

// ---------- per-vehicle configuration (supply crate, armed variant) ----------

// VehicleOpt : configuration of the Slot-th vehicle of Key in the team
type VehicleOpt struct {
	Key   string `json:"key"`
	Slot  int    `json:"slot"`
	Crate string `json:"crate,omitempty"` // "" | AMMO | MED | AT
	Armed bool   `json:"armed,omitempty"`
}

// GetVehicleOpts : stored options, trimmed to the vehicles the team still has (losses remove the last slots)
func (t *AssaultTeam) GetVehicleOpts() []VehicleOpt {
	var in []VehicleOpt
	_ = json.Unmarshal([]byte(t.VehicleCfg), &in)
	have := map[string]int{}
	for _, v := range t.GetVehicles() {
		have[v.Key] += v.Count
	}
	out := []VehicleOpt{}
	for _, o := range in {
		if o.Slot >= 0 && o.Slot < have[o.Key] && (o.Crate != "" || o.Armed) {
			out = append(out, o)
		}
	}
	return out
}

func (t *AssaultTeam) SetVehicleOpts(v []VehicleOpt) {
	b, _ := json.Marshal(v)
	t.VehicleCfg = string(b)
}

// BCVehicle : battle payload vehicle entry (docs/BATTLE_CONTRACT.md); count vehicles of the same key and config
type BCVehicle struct {
	Key    string `json:"key"`
	Count  int    `json:"count"`
	Crate  string `json:"crate,omitempty"`
	Armed  bool   `json:"armed,omitempty"`
	Prefab string `json:"prefab,omitempty"`
}

// battleVehicles : allowed vehicles expanded with their per-vehicle config, identical ones grouped
func battleVehicles(t *AssaultTeam) []BCVehicle {
	opts := map[string]VehicleOpt{}
	for _, o := range t.GetVehicleOpts() {
		opts[o.Key+"#"+itoa(o.Slot)] = o
	}
	out := []BCVehicle{}
	for _, v := range filterVehicles(t.Type, t.GetVehicles()) {
		idx := map[string]int{}
		for i := 0; i < v.Count; i++ {
			o := opts[v.Key+"#"+itoa(i)]
			e := BCVehicle{Key: v.Key, Count: 1, Crate: o.Crate}
			if vr, ok := teamRules.Variants[v.Key]; ok {
				e.Prefab = vr.Base
				if o.Armed && vr.Armed != "" {
					e.Armed, e.Prefab = true, vr.Armed
				}
			}
			sig := e.Crate + "|" + e.Prefab
			if j, ok := idx[sig]; ok {
				out[j].Count++
				continue
			}
			idx[sig] = len(out)
			out = append(out, e)
		}
	}
	return out
}

// vehicleOptions : GET /api/teams/options — crate and armed-variant prices
func vehicleOptions(c *gin.Context) {
	c.JSON(200, gin.H{"crates": teamRules.Crates, "variants": teamRules.Variants})
}

// configureVehicle : POST /api/teams/:id/vehicle {key, slot, crate, armed} — new options are paid (war funds), removal is not refunded
func configureVehicle(c *gin.Context) {
	var in VehicleOpt
	if c.ShouldBindJSON(&in) != nil {
		errJ(c, 400, "bad request")
		return
	}
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
		errJ(c, 400, "only an idle assault team can be refitted")
		return
	}
	n := 0
	for _, v := range filterVehicles(t.Type, t.GetVehicles()) {
		if v.Key == in.Key {
			n += v.Count
		}
	}
	if in.Slot < 0 || in.Slot >= n {
		errJ(c, 400, "no such vehicle in this team")
		return
	}
	cr, crateOK := teamRules.Crates[in.Crate]
	if in.Crate != "" && !crateOK {
		errJ(c, 400, "unknown crate")
		return
	}
	vr, hasVar := teamRules.Variants[in.Key]
	if in.Armed && (!hasVar || vr.Armed == "") {
		errJ(c, 400, "no armed variant for this vehicle")
		return
	}
	opts := t.GetVehicleOpts()
	cur, at := VehicleOpt{}, -1
	for i, o := range opts {
		if o.Key == in.Key && o.Slot == in.Slot {
			cur, at = o, i
		}
	}
	cost := 0
	if in.Crate != "" && in.Crate != cur.Crate {
		cost += cr.Cost
	}
	if in.Armed && !cur.Armed {
		cost += vr.ArmedCost
	}
	if cost > 0 {
		city := getCity(w.ID, t.CityID)
		if city == nil || city.Owner != t.Faction || activeBattleAt(w.ID, city.ID) != nil {
			errJ(c, 400, "refitting needs a friendly city that is not under attack")
			return
		}
		var f Faction
		if db.Where("war_id = ? AND code = ?", w.ID, t.Faction).First(&f).Error != nil || f.Resources < cost {
			errJ(c, 400, "not enough resources")
			return
		}
		db.Model(&f).Update("resources", f.Resources-cost)
	}
	no := VehicleOpt{Key: in.Key, Slot: in.Slot, Crate: in.Crate, Armed: in.Armed}
	if at >= 0 {
		opts[at] = no
	} else {
		opts = append(opts, no)
	}
	t.SetVehicleOpts(opts)
	t.SetVehicleOpts(t.GetVehicleOpts()) // drop empty entries
	db.Model(&t).Update("vehicle_cfg", t.VehicleCfg)
	if cost > 0 {
		logEvent(w.ID, "order", t.Faction, fmt.Sprintf("%s refits %s %s #%d (-%d resources)", u.Name, t.Label(), in.Key, in.Slot+1, cost))
	}
	c.JSON(200, gin.H{"ok": true, "cost": cost})
}
