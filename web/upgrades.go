package main

// P2-A — assault-team upgrade tree + command points (docs/GAP_PLAN.md, docs/CAMPAIGN.md).
//
// Design (own, paraphrased from the archived mechanic "teams improve along branching
// doctrines, limited by the commander's authority"):
//   - every team carries a small set of doctrine upgrades (AssaultTeam.Upgrades, JSON list);
//   - each upgrade costs war funds (faction resources) AND command points (CP);
//   - CP budget of a team = its team level (P1-B) + commander rank / 3;
//   - tier-2 picks are branching: the two doctrines of a branch exclude each other;
//   - the number of live teams a commander may own is capped by his rank.

import (
	"encoding/json"
	"errors"
	"fmt"
	"sort"

	"github.com/gin-gonic/gin"
)

type TeamUpgrade struct {
	Key       string   `json:"key"`
	Label     string   `json:"label"`
	Desc      string   `json:"desc"`
	Tier      int      `json:"tier"`
	CP        int      `json:"cp"`
	Cost      int      `json:"cost"`
	Requires  string   `json:"requires,omitempty"`
	Excludes  []string `json:"excludes,omitempty"`
	EquipPlus int      `json:"equipPlus,omitempty"` // +equipment level (instant, permanent)
	MPPct     int      `json:"mpPct,omitempty"`     // +% max manpower (resupply refills to it)
	Morale    int      `json:"morale,omitempty"`    // instant morale gain
	SupplyPct int      `json:"supplyPct,omitempty"` // resupply discount %
}

var teamUpgrades = map[string]TeamUpgrade{
	"DRILL":      {Key: "DRILL", Label: "Field drill", Desc: "+1 equipment level", Tier: 1, CP: 1, Cost: 150, EquipPlus: 1},
	"LOGISTICS":  {Key: "LOGISTICS", Label: "Supply officer", Desc: "Resupply 25% cheaper", Tier: 1, CP: 1, Cost: 100, SupplyPct: 25},
	"SHOCK":      {Key: "SHOCK", Label: "Shock doctrine", Desc: "+15% max manpower", Tier: 2, CP: 2, Cost: 250, Requires: "DRILL", Excludes: []string{"ENTRENCH"}, MPPct: 15},
	"ENTRENCH":   {Key: "ENTRENCH", Label: "Entrenching doctrine", Desc: "+20 morale, +10% max manpower", Tier: 2, CP: 2, Cost: 250, Requires: "DRILL", Excludes: []string{"SHOCK"}, MPPct: 10, Morale: 20},
	"SPEARHEAD":  {Key: "SPEARHEAD", Label: "Spearhead", Desc: "+1 equipment level, +10% max manpower", Tier: 3, CP: 3, Cost: 400, Requires: "SHOCK", EquipPlus: 1, MPPct: 10},
	"BULWARK":    {Key: "BULWARK", Label: "Bulwark", Desc: "+20% max manpower, +20 morale", Tier: 3, CP: 3, Cost: 400, Requires: "ENTRENCH", MPPct: 20, Morale: 20},
	"RAIL_DEPOT": {Key: "RAIL_DEPOT", Label: "Rail depot", Desc: "Resupply a further 25% cheaper", Tier: 2, CP: 2, Cost: 200, Requires: "LOGISTICS", SupplyPct: 25},
}

func (t *AssaultTeam) GetUpgrades() []string {
	var u []string
	if t.Upgrades != "" {
		_ = json.Unmarshal([]byte(t.Upgrades), &u)
	}
	return u
}

func (t *AssaultTeam) SetUpgrades(u []string) {
	sort.Strings(u)
	b, _ := json.Marshal(u)
	t.Upgrades = string(b)
}

func upgradeSet(keys []string) map[string]bool {
	m := map[string]bool{}
	for _, k := range keys {
		m[k] = true
	}
	return m
}

func cpUsed(keys []string) int {
	n := 0
	for _, k := range keys {
		n += teamUpgrades[k].CP
	}
	return n
}

// cpBudget : team level (>=1) + commander rank / 3
func cpBudget(teamLevel, commanderRank int) int {
	if teamLevel < 1 {
		teamLevel = 1
	}
	return teamLevel + commanderRank/3
}

// teamCap : live teams one commander may own (rank 1 -> 2 ... rank 8 -> 6)
func teamCap(commanderRank int) int { return 2 + commanderRank/2 }

func checkUpgrade(owned []string, key string, budget int) error {
	up, ok := teamUpgrades[key]
	if !ok {
		return errors.New("unknown upgrade")
	}
	have := upgradeSet(owned)
	if have[key] {
		return errors.New("already owned")
	}
	if up.Requires != "" && !have[up.Requires] {
		return errors.New("requires " + teamUpgrades[up.Requires].Label)
	}
	for _, x := range up.Excludes {
		if have[x] {
			return errors.New("excluded by " + teamUpgrades[x].Label + " (other branch)")
		}
	}
	if cpUsed(owned)+up.CP > budget {
		return fmt.Errorf("not enough command points (%d/%d used)", cpUsed(owned), budget)
	}
	return nil
}

func upgradeTotals(keys []string) (mpPct, supplyPct int) {
	for _, k := range keys {
		mpPct += teamUpgrades[k].MPPct
		supplyPct += teamUpgrades[k].SupplyPct
	}
	if supplyPct > 75 {
		supplyPct = 75
	}
	return
}

// teamMaxManpower : base manpower of the type, raised by doctrine upgrades
func teamMaxManpower(t *AssaultTeam) int {
	mp, _ := upgradeTotals(t.GetUpgrades())
	return atSpecs[t.Type].Manpower * (100 + mp) / 100
}

func ownerRank(id uint) int {
	var p Player
	if id == 0 || db.First(&p, id).Error != nil {
		return 1
	}
	return rankNum(p.XP)
}

// teamCapReached : "" if the commander may raise another team
func teamCapReached(u *Player, warID uint) string {
	if u.Role == "admin" {
		return ""
	}
	var n int64
	db.Model(&AssaultTeam{}).Where("war_id = ? AND owner_id = ? AND status NOT IN ?", warID, u.ID, []string{"destroyed", "dissolved"}).Count(&n)
	if c := teamCap(rankNum(u.XP)); int(n) >= c {
		return fmt.Sprintf("team cap reached: your rank allows %d assault teams", c)
	}
	return ""
}

func loadTeamParam(c *gin.Context) *AssaultTeam {
	var t AssaultTeam
	id := c.Param("id")
	if len(id) > 3 && id[:3] == "at_" {
		id = id[3:]
	}
	if db.First(&t, "id = ?", id).Error != nil {
		errJ(c, 404, "team not found")
		return nil
	}
	return &t
}

func getTeamUpgrades(c *gin.Context) {
	t := loadTeamParam(c)
	if t == nil {
		return
	}
	owned := t.GetUpgrades()
	budget := cpBudget(t.TeamLevel, ownerRank(t.OwnerID))
	list := make([]gin.H, 0, len(teamUpgrades))
	keys := make([]string, 0, len(teamUpgrades))
	for k := range teamUpgrades {
		keys = append(keys, k)
	}
	sort.Slice(keys, func(i, j int) bool {
		a, b := teamUpgrades[keys[i]], teamUpgrades[keys[j]]
		if a.Tier != b.Tier {
			return a.Tier < b.Tier
		}
		return a.Key < b.Key
	})
	for _, k := range keys {
		st := ""
		if err := checkUpgrade(owned, k, budget); err != nil {
			st = err.Error()
		}
		list = append(list, gin.H{"upgrade": teamUpgrades[k], "owned": upgradeSet(owned)[k], "blocked": st})
	}
	c.JSON(200, gin.H{"teamId": t.TeamID(), "cpUsed": cpUsed(owned), "cpBudget": budget, "upgrades": list})
}

func buyTeamUpgrade(c *gin.Context) {
	u := user(c)
	gameMu.Lock()
	defer gameMu.Unlock()
	t := loadTeamParam(c)
	if t == nil {
		return
	}
	if !ownsTeam(u, t) {
		errJ(c, 403, "not your team")
		return
	}
	if t.Status != "idle" {
		errJ(c, 400, "team must be idle")
		return
	}
	key := c.Param("key")
	owned := t.GetUpgrades()
	if err := checkUpgrade(owned, key, cpBudget(t.TeamLevel, ownerRank(t.OwnerID))); err != nil {
		errJ(c, 400, err.Error())
		return
	}
	up := teamUpgrades[key]
	var f Faction
	if db.Where("war_id = ? AND code = ?", t.WarID, t.Faction).First(&f).Error != nil || f.Resources < up.Cost {
		errJ(c, 400, "not enough war funds")
		return
	}
	db.Model(&f).Update("resources", f.Resources-up.Cost)
	t.SetUpgrades(append(owned, key))
	upd := map[string]interface{}{"upgrades": t.Upgrades}
	if up.EquipPlus > 0 {
		upd["equip_level"] = t.EquipLevel + up.EquipPlus
	}
	if up.Morale > 0 {
		m := t.Morale + up.Morale
		if m > 100 {
			m = 100
		}
		upd["morale"] = m
	}
	db.Model(t).Updates(upd)
	logEvent(t.WarID, "order", t.Faction, fmt.Sprintf("%s adopts %s (-%d resources)", t.Label(), up.Label, up.Cost))
	c.JSON(200, gin.H{"ok": true})
}
