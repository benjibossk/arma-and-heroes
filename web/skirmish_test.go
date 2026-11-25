package main

import (
	"testing"
	"time"
)

func TestMeetsOnRoad(t *testing.T) {
	a := &AssaultTeam{Faction: "US", CityID: "A", DestCityID: "B", Status: "moving"}
	b := &AssaultTeam{Faction: "GER", CityID: "B", DestCityID: "A", Status: "moving"}
	if !meetsOnRoad(a, b) {
		t.Fatal("opposite hostile teams on the same road must meet")
	}
	same := &AssaultTeam{Faction: "US", CityID: "B", DestCityID: "A", Status: "moving"}
	if meetsOnRoad(a, same) {
		t.Fatal("allies do not fight")
	}
	other := &AssaultTeam{Faction: "GER", CityID: "B", DestCityID: "C", Status: "moving"}
	if meetsOnRoad(a, other) {
		t.Fatal("different road")
	}
	b.Status = "idle"
	if meetsOnRoad(a, b) {
		t.Fatal("idle team is not on the road")
	}
}

func TestSkirmishExit(t *testing.T) {
	now := time.Now()
	w := skirmishExit(true, now)
	if w["status"] != "moving" || w["arrival_at"] != now {
		t.Fatal("winner resumes its march")
	}
	if _, ok := w["dest_city_id"]; ok {
		t.Fatal("winner keeps its destination")
	}
	l := skirmishExit(false, now)
	if l["status"] != "idle" || l["dest_city_id"] != "" {
		t.Fatal("loser falls back to origin")
	}
}
