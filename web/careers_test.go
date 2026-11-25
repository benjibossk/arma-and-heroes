package main

import (
	"testing"
	"time"
)

func TestCareerLock(t *testing.T) {
	now := time.Date(2026, 1, 10, 0, 0, 0, 0, time.UTC)
	if p, m := careerChange(nil, "armor", now); p != 0 || m != "" {
		t.Fatal("first pick is free")
	}
	if _, m := careerChange(nil, "cook", now); m == "" {
		t.Fatal("unknown career")
	}
	cur := &PlayerCareer{Career: "armor", ChangedAt: now.Add(-24 * time.Hour)}
	if _, m := careerChange(cur, "recon", now); m == "" {
		t.Fatal("locked during lock window")
	}
	cur.ChangedAt = now.Add(-8 * 24 * time.Hour)
	if p, m := careerChange(cur, "recon", now); m != "" || p != careerSwitchPrice {
		t.Fatal("paid switch after lock")
	}
	if _, m := careerChange(cur, "armor", now); m == "" {
		t.Fatal("same career")
	}
}

func TestCareerClasses(t *testing.T) {
	if !careerAllowsClass("", "CREW") {
		t.Fatal("grandfathered")
	}
	if careerAllowsClass("infantry", "CREW") || !careerAllowsClass("armor", "CREW") || !careerAllowsClass("recon", "INF") {
		t.Fatal("class gating")
	}
}
