package main

import (
	"testing"
	"time"
)

func dirBattle(status string, created time.Time) *Battle {
	return &Battle{Status: status, Attacker: "US", Defender: "GER", CreatedAt: created}
}

func TestDirectorPrepWindow(t *testing.T) {
	t0 := time.Date(2026, 1, 1, 12, 0, 0, 0, time.UTC)
	b := dirBattle("pending", t0)
	str := map[string]int{"US": 100, "GER": 80}
	if battleClaimable(b, t0.Add(time.Minute)) || directorDecide(b, str, t0.Add(time.Minute)) != dirNone {
		t.Fatal("battle must wait during prep window")
	}
	after := t0.Add((dirPrepMin + 1) * time.Minute)
	if !battleClaimable(b, after) || directorDecide(b, str, after) != dirNone {
		t.Fatal("battle must be claimable after prep, not touched")
	}
}

func TestDirectorUndersizedCancelled(t *testing.T) {
	t0 := time.Date(2026, 1, 1, 12, 0, 0, 0, time.UTC)
	b := dirBattle("pending", t0)
	str := map[string]int{"US": dirMinAttackMP - 1, "GER": 80}
	if directorDecide(b, str, t0.Add(time.Minute)) != dirNone {
		t.Fatal("no cancel during prep")
	}
	if directorDecide(b, str, t0.Add((dirPrepMin+1)*time.Minute)) != dirCancel {
		t.Fatal("undersized attack must be cancelled after prep")
	}
}

func TestDirectorIdleResolved(t *testing.T) {
	t0 := time.Date(2026, 1, 1, 12, 0, 0, 0, time.UTC)
	b := dirBattle("pending", t0)
	str := map[string]int{"US": 150, "GER": 80}
	late := t0.Add((dirPrepMin + dirIdleResolveMin + 1) * time.Minute)
	if directorDecide(b, str, late) != dirResolve {
		t.Fatal("unclaimed battle must be auto-resolved")
	}
	if autoWinner(b, str) != "US" {
		t.Fatal("stronger side must win")
	}
	if autoWinner(b, map[string]int{"US": 80, "GER": 80}) != "GER" {
		t.Fatal("tie goes to defender")
	}
	// claimed battle with fresh heartbeat is left alone, stale one is resolved
	r := dirBattle("running", t0)
	hb := late.Add(-time.Minute)
	r.HeartbeatAt = &hb
	if directorDecide(r, str, late) != dirNone {
		t.Fatal("live battle must not be resolved")
	}
	if directorDecide(r, str, late.Add((dirHeartbeatLostMin+1)*time.Minute)) != dirResolve {
		t.Fatal("battle with lost heartbeat must be resolved")
	}
}
