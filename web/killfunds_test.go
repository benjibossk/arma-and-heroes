package main

import "testing"

func TestKillFundsFor(t *testing.T) {
	old := teamRules.VehicleRoles
	teamRules.VehicleRoles = map[string]string{"GER_PZ4": "tank", "GER_TRUCK": "truck"}
	defer func() { teamRules.VehicleRoles = old }()
	k := VictimKills{Players: 2, Bots: 4, Vehicles: map[string]int{"GER_PZ4": 1, "GER_TRUCK": 2, "X": 1}}
	want := 2*10 + 4*5 + 80 + 2*15 + 15
	if got := killFundsFor(k); got != want {
		t.Fatalf("killFundsFor = %d, want %d", got, want)
	}
}
