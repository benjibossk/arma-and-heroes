package main

// P2-D — rank salary (docs/CAMPAIGN.md): credits = rank pay per minute alive, capped,
// + a stay-to-end bonus. Added to the battle credits after computeReward (outside the cap:
// it rewards presence, not farming). Tunables in progression.json "salary".

type SalaryRule struct {
	PerMinByRank []int `json:"perMinByRank"`
	StayToEndPct int   `json:"stayToEndPct"`
	MaxMin       int   `json:"maxMin"`
}

func salaryCredits(rank int, rp *ResPlayer, battleSec int) int {
	sr := prog.Salary
	if len(sr.PerMinByRank) == 0 || rank < 1 {
		return 0
	}
	if rank > len(sr.PerMinByRank) {
		rank = len(sr.PerMinByRank)
	}
	alive := rp.AliveSec
	if alive == 0 {
		alive = rp.PresenceSec
	}
	min := alive / 60
	if sr.MaxMin > 0 && min > sr.MaxMin {
		min = sr.MaxMin
	}
	pay := min * sr.PerMinByRank[rank-1]
	stayed := false
	if rp.StayedToEnd != nil {
		stayed = *rp.StayedToEnd
	} else if battleSec > 0 && rp.PresenceSec > 0 {
		stayed = rp.PresenceSec*10 >= battleSec*9
	}
	if stayed {
		pay += pay * sr.StayToEndPct / 100
	}
	return pay
}
