package main

import "testing"

func TestSalary(t *testing.T) {
	old := prog.Salary
	defer func() { prog.Salary = old }()
	prog.Salary = SalaryRule{PerMinByRank: []int{2, 4}, StayToEndPct: 50, MaxMin: 30}
	yes, no := true, false
	if v := salaryCredits(1, &ResPlayer{AliveSec: 600, StayedToEnd: &no}, 1200); v != 20 {
		t.Fatalf("10 min x 2 = 20, got %d", v)
	}
	if v := salaryCredits(2, &ResPlayer{AliveSec: 600, StayedToEnd: &yes}, 1200); v != 60 {
		t.Fatalf("10 min x 4 +50%% = 60, got %d", v)
	}
	if v := salaryCredits(9, &ResPlayer{AliveSec: 7200}, 0); v != 120 {
		t.Fatalf("rank clamped, minutes capped: 30 x 4 = 120, got %d", v)
	}
	if v := salaryCredits(1, &ResPlayer{PresenceSec: 1140}, 1200); v != 57 {
		t.Fatalf("fallback presence 19 min x2 = 38 +50%% (stayed >=90%%) = 57, got %d", v)
	}
}
