package main

import "testing"

func TestUpgradeBranchRules(t *testing.T) {
	if err := checkUpgrade(nil, "SHOCK", 10); err == nil {
		t.Fatal("tier 2 needs its prerequisite")
	}
	if err := checkUpgrade([]string{"DRILL"}, "SHOCK", 10); err != nil {
		t.Fatal(err)
	}
	if err := checkUpgrade([]string{"DRILL", "SHOCK"}, "ENTRENCH", 10); err == nil {
		t.Fatal("branches must exclude each other")
	}
	if err := checkUpgrade([]string{"DRILL"}, "DRILL", 10); err == nil {
		t.Fatal("no double buy")
	}
	if err := checkUpgrade([]string{"DRILL"}, "SHOCK", 2); err == nil {
		t.Fatal("CP budget must be enforced (1+2 > 2)")
	}
	if err := checkUpgrade(nil, "NOPE", 10); err == nil {
		t.Fatal("unknown key")
	}
}

func TestCommandPointsAndCap(t *testing.T) {
	if cpBudget(0, 1) != 1 || cpBudget(3, 6) != 5 {
		t.Fatal("cp budget = team level + rank/3")
	}
	if teamCap(1) != 2 || teamCap(8) != 6 {
		t.Fatal("team cap by rank")
	}
	mp, sup := upgradeTotals([]string{"LOGISTICS", "RAIL_DEPOT", "DRILL", "SHOCK"})
	if mp != 15 || sup != 50 {
		t.Fatalf("totals mp=%d sup=%d", mp, sup)
	}
	tm := &AssaultTeam{Type: "INF"}
	tm.SetUpgrades([]string{"DRILL", "SHOCK"})
	if teamMaxManpower(tm) != atSpecs["INF"].Manpower*115/100 {
		t.Fatal("max manpower bonus")
	}
}
