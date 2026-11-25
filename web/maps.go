package main

import (
	"os"
	"strings"
)

// War-map city -> (world, layout) for the battle servers (docs/MAPS.md).
// world = key of orchestrator/maps.json (scenario + mods); layout = id in mod Configs/F44/layouts.json.
// A mapping is only sent when its world is listed in F44_MAPS_ENABLED (comma list, e.g.
// "ww2arland,belleauwood"): default = nothing sent, the battle runs as before on Arland.
type CityWorld struct {
	World  string
	Layout string
}

var cityWorlds = map[string]CityWorld{
	// Utah Beach (UTAHBeach 698C2A44E9972692)
	"carentan":     {"utahbeach", "utah-carentan"},
	"montebourg":   {"utahbeach", "utah-montebourg"},
	"sainte-marie": {"utahbeach", "utah-sainte-marie-du-mont"},
	"sainte-mere":  {"utahbeach", "utah-sainte-mere-eglise"},
	// Normandy (616D67C41B1DF93D)
	"brecourt": {"normandy", "nor-brecourt"},
	"hubert":   {"normandy", "nor-hubert"},
	// Belleau Wood (629EDC37CDE3AD35)
	"belleau":    {"belleauwood", "bw-belleau"},
	"bouresches": {"belleauwood", "bw-bouresches"},
	"lucy":       {"belleauwood", "bw-lucy"},
	"torcy":      {"belleauwood", "bw-torcy"},
	// WW2 Arland (6072D68D8DCD0C12): the Arland cities, iRON79 WW2 version
	"ww2-arleville":  {"ww2arland", "ww2-arleville"},
	"ww2-beauregard": {"ww2arland", "ww2-beauregard"},
	"ww2-base":       {"ww2arland", "ww2-military-base"},
}

func cityWorld(cityID string) (CityWorld, bool) {
	cw, ok := cityWorlds[cityID]
	if !ok {
		return cw, false
	}
	for _, w := range strings.Split(os.Getenv("F44_MAPS_ENABLED"), ",") {
		if strings.TrimSpace(w) == cw.World {
			return cw, true
		}
	}
	return cw, false
}
