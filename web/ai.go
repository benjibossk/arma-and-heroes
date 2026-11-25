package main

import (
	"encoding/json"

	"github.com/gin-gonic/gin"
)

// AI bots settings, sent per side in BattleConfig
type AIConfig struct {
	Enabled        bool    `json:"enabled"`
	MaxAlive       int     `json:"maxAlive"`
	PerPlayerScale float64 `json:"perPlayerScale"`
	Skill          string  `json:"skill"` // Recruit | Regular | Veteran | Elite (applied by the mod at spawn)
}

var skillRank = map[string]int{"Recruit": 1, "Regular": 2, "Veteran": 3, "Elite": 4}

type AISettings struct {
	Default AIConfig       `json:"default"`
	PerType map[string]int `json:"perTypeMaxAlive"` // maxAlive per assault team type
	PerTypeSkill map[string]string `json:"perTypeSkill"` // skill per assault team type (best one wins)
}

type Setting struct {
	Key   string `gorm:"primaryKey;size:64"`
	Value string `gorm:"type:text"`
}

func defaultAI() AISettings {
	return AISettings{Default: AIConfig{true, 24, 1.0, "Regular"}, PerType: map[string]int{"INF": 24, "PARA": 16, "ARM": 8, "REC": 8, "MOT": 20},
		PerTypeSkill: map[string]string{"INF": "Regular", "PARA": "Elite", "ARM": "Regular", "REC": "Veteran", "MOT": "Regular"}}
}

func loadAI() AISettings {
	s := defaultAI()
	var st Setting
	if db.First(&st, "`key` = ?", "ai").Error == nil {
		_ = json.Unmarshal([]byte(st.Value), &s)
	}
	if s.PerType == nil {
		s.PerType = defaultAI().PerType
	}
	if s.PerTypeSkill == nil {
		s.PerTypeSkill = defaultAI().PerTypeSkill
	}
	if skillRank[s.Default.Skill] == 0 {
		s.Default.Skill = "Regular"
	}
	return s
}

// sideAI : maxAlive = highest per-type value among the side's teams (default if none)
func sideAI(s AISettings, types []string) AIConfig {
	out := s.Default
	best := 0
	for _, t := range types {
		if v, ok := s.PerType[t]; ok && v > best {
			best = v
		}
	}
	if best > 0 {
		out.MaxAlive = best
	}
	bestSkill := 0
	for _, t := range types {
		if sk := s.PerTypeSkill[t]; skillRank[sk] > bestSkill {
			bestSkill, out.Skill = skillRank[sk], sk
		}
	}
	return out
}

func adminGetAI(c *gin.Context) { c.JSON(200, loadAI()) }

func adminPutAI(c *gin.Context) {
	var s AISettings
	if c.ShouldBindJSON(&s) != nil || s.Default.MaxAlive < 0 || s.Default.MaxAlive > 200 || s.Default.PerPlayerScale < 0 || s.Default.PerPlayerScale > 10 {
		errJ(c, 400, "invalid AI settings")
		return
	}
	if s.Default.Skill == "" {
		s.Default.Skill = "Regular"
	}
	ok := skillRank[s.Default.Skill] > 0
	for _, v := range s.PerTypeSkill {
		ok = ok && skillRank[v] > 0
	}
	if !ok {
		errJ(c, 400, "skill must be Recruit, Regular, Veteran or Elite")
		return
	}
	b, _ := json.Marshal(s)
	db.Save(&Setting{Key: "ai", Value: string(b)})
	c.JSON(200, s)
}
