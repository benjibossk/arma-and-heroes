package main

import (
	"testing"
	"time"
)

func TestMoraleAfterBattle(t *testing.T) {
	if m, r := moraleAfterBattle(100, true); m != 95 || r {
		t.Fatalf("win: %d %v", m, r)
	}
	if m, r := moraleAfterBattle(100, false); m != 75 || r {
		t.Fatalf("loss: %d %v", m, r)
	}
	if m, r := moraleAfterBattle(40, false); m != 15 || !r {
		t.Fatalf("rout: %d %v", m, r)
	}
	if m, _ := moraleAfterBattle(5, false); m != 0 {
		t.Fatalf("floor: %d", m)
	}
}

func TestRegenMorale(t *testing.T) {
	now := time.Now()
	last := now.Add(-4 * time.Hour)
	if m := regenMorale(50, &last, now); m != 70 {
		t.Fatalf("regen %d", m)
	}
	if m := regenMorale(95, &last, now); m != 100 {
		t.Fatalf("cap %d", m)
	}
	if m := regenMorale(50, nil, now); m != 50 {
		t.Fatalf("nil %d", m)
	}
}

func TestAddTeamXP(t *testing.T) {
	if xp, l := addTeamXP(0, 1, true, 100); xp != 0 || l != 1 {
		t.Fatalf("short battle gives xp: %d %d", xp, l)
	}
	xp, l := addTeamXP(150, 1, true, 600)
	if l != 2 || xp != 50 {
		t.Fatalf("level up: %d %d", xp, l)
	}
	if xp, l = addTeamXP(0, 0, false, 600); xp != 40 || l != 1 {
		t.Fatalf("loss: %d %d", xp, l)
	}
}
