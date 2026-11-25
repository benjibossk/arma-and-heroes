package main

import "testing"

func TestStockpileRules(t *testing.T) {
	if addStock(590, 30) != stockpileCap || addStock(100, 30) != 130 {
		t.Fatal("stockpile cap")
	}
	if s, f := splitCost(500, 300); s != 300 || f != 0 {
		t.Fatal("paid from stock")
	}
	if s, f := splitCost(100, 300); s != 100 || f != 200 {
		t.Fatal("stock then funds")
	}
	if factoryShare(&City{IsFactory: true}) != 0 || factoryShare(&City{IsFactory: true, Owner: "US"}) != 30 || factoryShare(&City{Owner: "US"}) != 0 {
		t.Fatal("factory share")
	}
}

func TestCapitalWinner(t *testing.T) {
	if capitalWinner(map[string]string{"a": "US", "b": "GER"}) != "" {
		t.Fatal("split capitals: no winner")
	}
	if capitalWinner(map[string]string{"a": "US", "b": "US"}) != "US" {
		t.Fatal("both capitals")
	}
	three := map[string]string{"a": "US", "b": "GER", "n": "US"}
	if capitalWinner(three) != "" {
		t.Fatal("2/3 is not enough")
	}
	three["b"] = "US"
	if capitalWinner(three) != "US" {
		t.Fatal("3/3 wins")
	}
	four := map[string]string{"a": "GER", "b": "GER", "c": "GER", "d": ""}
	if capitalWinner(four) != "GER" {
		t.Fatal("3/4 wins")
	}
}

func TestWarEndCredits(t *testing.T) {
	if warEndCredits(true, 0) != warEndWinCredits || warEndCredits(false, 5) != warEndLossCredits+5*warEndPerBattle {
		t.Fatal("payout")
	}
	if warEndCredits(true, 99) != warEndWinCredits+warEndBattleCap*warEndPerBattle {
		t.Fatal("battle cap")
	}
}
