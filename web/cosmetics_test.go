package main

import "testing"

func TestCrossFaction(t *testing.T) {
	us := &Soldier{Faction: "US", Class: "INF", XP: 0}
	ger := &Item{Faction: "GER", Slot: "primary", Price: 200}
	if !isCrossFaction(ger, us) || unlockPrice(ger, us) != 300 {
		t.Fatal("+50% surcharge on enemy weapon")
	}
	if unlockPrice(&Item{Faction: "US", Slot: "primary", Price: 200}, us) != 200 {
		t.Fatal("own faction full price")
	}
	if isCrossFaction(&Item{Faction: "GER", Slot: "outfit"}, us) {
		t.Fatal("only weapons cross over")
	}
}

func TestCamoPurchase(t *testing.T) {
	own := map[string]bool{"standard": true}
	if p, m := camoPurchase("standard", own, 1); p != 0 || m != "" {
		t.Fatal("owned camo free")
	}
	if _, m := camoPurchase("veteran", own, 2); m == "" {
		t.Fatal("rank gate")
	}
	if p, m := camoPurchase("mud", own, 2); m != "" || p != camos["mud"].Price {
		t.Fatal("buy camo")
	}
	if _, m := camoPurchase("pink", own, 9); m == "" {
		t.Fatal("unknown camo")
	}
}

func TestWeaponUpgrade(t *testing.T) {
	gun := &Item{Slot: "primary", Price: 400}
	if weaponUpgradeCheck(gun, true, 0, 2) != "" || weaponUpgradeCheck(gun, true, 1, 2) == "" {
		t.Fatal("level n needs rank n+1")
	}
	if weaponUpgradeCheck(gun, false, 0, 9) == "" || weaponUpgradeCheck(gun, true, 3, 9) == "" {
		t.Fatal("ownership / max")
	}
	if weaponUpgradeCheck(&Item{Slot: "equipment"}, true, 0, 9) == "" {
		t.Fatal("weapons only")
	}
	if weaponUpgradePrice(400, 2) != 400 {
		t.Fatal("price 100*2 + 400*2/4")
	}
}
