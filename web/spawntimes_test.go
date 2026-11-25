package main

import (
	"encoding/json"
	"testing"
)

func TestSpawnTimesRoundTrip(t *testing.T) {
	b, err := json.Marshal(BattleConfig{SpawnTimes: defaultSpawnTimes})
	if err != nil {
		t.Fatal(err)
	}
	var m map[string]map[string]float64
	var raw map[string]json.RawMessage
	json.Unmarshal(b, &raw)
	if err := json.Unmarshal([]byte(`{"x":`+string(raw["spawnTimes"])+`}`), &m); err != nil {
		t.Fatal(err)
	}
	for _, k := range []string{"baseSec", "forwardPenaltySec", "perPointSec", "maxEquipSec", "perLinkSec", "defenderBonusSec", "minSec"} {
		if _, ok := m["x"][k]; !ok {
			t.Errorf("missing %s", k)
		}
	}
	var back BattleConfig
	json.Unmarshal(b, &back)
	if back.SpawnTimes != defaultSpawnTimes {
		t.Errorf("round trip mismatch %+v", back.SpawnTimes)
	}
}
