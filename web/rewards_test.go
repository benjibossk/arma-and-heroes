package main

import (
	"testing"
	"time"
)

func TestDailyCycle(t *testing.T) {
	now := time.Date(2026, 11, 26, 15, 0, 0, 0, time.UTC)
	if d, cr, ok := dailyNext(nil, 0, now); !ok || d != 1 || cr != dailyCycle[0] {
		t.Fatal("first claim = day 1")
	}
	y := now.Add(-20 * time.Hour) // yesterday 19:00
	if d, _, ok := dailyNext(&y, 3, now); !ok || d != 4 {
		t.Fatal("consecutive day advances streak")
	}
	same := now.Add(-time.Hour)
	if _, _, ok := dailyNext(&same, 3, now); ok {
		t.Fatal("once per day")
	}
	old := now.AddDate(0, 0, -3)
	if d, _, _ := dailyNext(&old, 5, now); d != 1 {
		t.Fatal("missed day resets")
	}
	if d, _, _ := dailyNext(&y, 7, now); d != 1 {
		t.Fatal("cycle loops after day 7")
	}
}

func TestSeason(t *testing.T) {
	id, start := seasonOf(time.Date(2026, 11, 26, 0, 0, 0, 0, time.UTC))
	if id != "2026-Q4" || !start.Equal(time.Date(2026, 10, 1, 0, 0, 0, 0, time.UTC)) {
		t.Fatal(id, start)
	}
	if seasonTier(2500) != 2 || seasonTier(99999) != seasonTiers {
		t.Fatal("tiers")
	}
	if seasonClaim(0, 2) != 300 || seasonClaim(4, 5) != 1000 || seasonClaim(3, 3) != 0 {
		t.Fatal("claim sums")
	}
}
