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
}

type AISettings struct {
	Default AIConfig       `json:"default"`
	PerType map[string]int `json:"perTypeMaxAlive"` // maxAlive per assault team type
}

type Setting struct {
	Key   string `gorm:"primaryKey;size:64"`
	Value string `gorm:"type:text"`
}

func defaultAI() AISettings {
	return AISettings{Default: AIConfig{true, 24, 1.0}, PerType: map[string]int{"INF": 24, "PARA": 16, "ARM": 8, "REC": 8, "MOT": 20}}
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
	return out
}

func adminGetAI(c *gin.Context) { c.JSON(200, loadAI()) }

func adminPutAI(c *gin.Context) {
	var s AISettings
	if c.ShouldBindJSON(&s) != nil || s.Default.MaxAlive < 0 || s.Default.MaxAlive > 200 || s.Default.PerPlayerScale < 0 || s.Default.PerPlayerScale > 10 {
		errJ(c, 400, "invalid AI settings")
		return
	}
	b, _ := json.Marshal(s)
	db.Save(&Setting{Key: "ai", Value: string(b)})
	c.JSON(200, s)
}
