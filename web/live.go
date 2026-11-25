package main

import (
	"encoding/json"
	"sync"
	"time"

	"github.com/gin-gonic/gin"
)

// live battle snapshots (memory only, last one per battle), posted every ~5 s by the mod
type liveSnap struct {
	Data json.RawMessage
	At   time.Time
}

var (
	liveMu   sync.RWMutex
	liveData = map[string]liveSnap{}
)

func postLive(c *gin.Context) {
	b := loadBattle(c)
	if b == nil {
		return
	}
	body, _ := c.GetRawData()
	if len(body) == 0 || len(body) > 64000 || !json.Valid(body) {
		errJ(c, 400, "invalid live snapshot")
		return
	}
	if b.Status == "starting" { // the mod posts live data, not heartbeats
		db.Model(&Battle{}).Where("id = ? AND status = ?", b.ID, "starting").Updates(map[string]interface{}{"status": "running", "heartbeat_at": time.Now().UTC()})
	} else if b.Status == "running" {
		db.Model(&Battle{}).Where("id = ?", b.ID).Update("heartbeat_at", time.Now().UTC())
	}
	liveMu.Lock()
	liveData[b.BattleID()] = liveSnap{Data: json.RawMessage(body), At: time.Now().UTC()}
	for k, v := range liveData { // forget stale battles
		if time.Since(v.At) > 2*time.Hour {
			delete(liveData, k)
		}
	}
	liveMu.Unlock()
	c.JSON(200, gin.H{"ok": true})
}

func getLive(c *gin.Context) {
	b := loadBattle(c)
	if b == nil {
		return
	}
	liveMu.RLock()
	s, ok := liveData[b.BattleID()]
	liveMu.RUnlock()
	out := gin.H{"battleId": b.BattleID(), "cityName": "", "status": b.Status, "attacker": b.Attacker, "defender": b.Defender, "winner": b.Winner}
	if ci := getCity(b.WarID, b.CityID); ci != nil {
		out["cityName"] = ci.Name
	}
	if ok {
		out["snapshot"] = s.Data
		out["at"] = s.At
	}
	c.JSON(200, out)
}
