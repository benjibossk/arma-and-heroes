package main

import (
	"log"
	"time"

	"golang.org/x/crypto/bcrypt"
)

func ensureAdmin() {
	if cfg.AdminPass == "" {
		return
	}
	var p Player
	if db.Where("name = ?", cfg.AdminUser).First(&p).Error == nil {
		return
	}
	h, _ := bcrypt.GenerateFromPassword([]byte(cfg.AdminPass), bcrypt.DefaultCost)
	db.Create(&Player{Name: cfg.AdminUser, PassHash: string(h), Role: "admin"})
	log.Println("compte admin créé :", cfg.AdminUser)
}

func currentWar() *War {
	var w War
	if db.Order("id desc").First(&w).Error != nil {
		return nil
	}
	return &w
}

const arlandPlaceholder = "{0000000000000000}Missions/F44_Arland_"

// seedWar crée une guerre de démonstration (12 villes, 2 AT)
func seedWar() *War {
	now := time.Now().UTC()
	w := War{Name: "Guerre n°" + itoa(int(countWars()+1)) + " — Front de Normandie", Status: "active", StartedAt: now, LastIncomeAt: now}
	db.Create(&w)
	db.Create(&Faction{WarID: w.ID, Code: "US", Name: "Armée américaine", Resources: 1500})
	db.Create(&Faction{WarID: w.ID, Code: "GER", Name: "Wehrmacht", Resources: 1500})
	type c = City
	cities := []c{
		{ID: "saint-aubin", Name: "Saint-Aubin", X: 90, Y: 360, Owner: "US", IsCapital: true, IsFactory: true, Terrain: "Everon WW2"},
		{ID: "vierville", Name: "Vierville", X: 200, Y: 180, Owner: "US", Terrain: "Everon WW2"},
		{ID: "colleville", Name: "Colleville", X: 220, Y: 520, Owner: "US", IsFactory: true, Terrain: "Montferrier1944"},
		{ID: "sainte-marie", Name: "Sainte-Marie", X: 330, Y: 350, Owner: "US", Terrain: "Montferrier1944"},
		// V0 : trois villes sur WW2 Arland
		{ID: "arland-village", Name: "Les Mesnils (Arland)", X: 470, Y: 230, Owner: "GER", Terrain: "WW2 Arland", ScenarioID: arlandPlaceholder + "Village.conf"},
		{ID: "arland-ferme", Name: "Ferme du Carrefour (Arland)", X: 490, Y: 420, Owner: "GER", Terrain: "WW2 Arland", ScenarioID: arlandPlaceholder + "Ferme.conf", Mode: "SKIRMISH"},
		{ID: "arland-ville", Name: "Saint-Arland (Arland)", X: 620, Y: 330, Owner: "GER", Terrain: "WW2 Arland", ScenarioID: arlandPlaceholder + "Ville.conf"},
		{ID: "carentan", Name: "Carentan", X: 640, Y: 560, Owner: "GER", IsFactory: true, Terrain: "Utah Beach"},
		{ID: "montebourg", Name: "Montebourg", X: 650, Y: 120, Owner: "GER", Terrain: "Everon WW2"},
		{ID: "valognes", Name: "Valognes", X: 780, Y: 230, Owner: "GER", IsFactory: true, Terrain: "Everon WW2"},
		{ID: "pont-l-abbe", Name: "Pont-l'Abbé", X: 790, Y: 460, Owner: "GER", Terrain: "Montferrier1944"},
		{ID: "cherbourg", Name: "Cherbourg", X: 910, Y: 340, Owner: "GER", IsCapital: true, IsFactory: true, Terrain: "Everon WW2"},
	}
	for i := range cities {
		cities[i].WarID = w.ID
		if cities[i].Mode == "" {
			cities[i].Mode = "ASSAULT"
		}
		if cities[i].ScenarioID == "" {
			cities[i].ScenarioID = "{0000000000000000}Missions/F44_" + cities[i].ID + ".conf"
		}
		cities[i].Garrison = 40
		if cities[i].IsCapital {
			cities[i].Garrison = 100
		}
	}
	db.Exec("DELETE FROM cities")
	db.Create(&cities)
	roads := [][3]interface{}{
		{"saint-aubin", "vierville", 12}, {"saint-aubin", "colleville", 14}, {"saint-aubin", "sainte-marie", 15},
		{"vierville", "sainte-marie", 10}, {"colleville", "sainte-marie", 11}, {"vierville", "arland-village", 18},
		{"sainte-marie", "arland-village", 10}, {"sainte-marie", "arland-ferme", 10}, {"colleville", "arland-ferme", 16},
		{"arland-village", "arland-ville", 8}, {"arland-ferme", "arland-ville", 8}, {"arland-village", "montebourg", 14},
		{"arland-ferme", "carentan", 12}, {"montebourg", "valognes", 10}, {"arland-ville", "valognes", 12},
		{"arland-ville", "pont-l-abbe", 14}, {"carentan", "pont-l-abbe", 10}, {"valognes", "cherbourg", 10},
		{"pont-l-abbe", "cherbourg", 12},
	}
	for _, r := range roads {
		db.Create(&Road{WarID: w.ID, CityA: r[0].(string), CityB: r[1].(string), Distance: r[2].(int)})
	}
	var owner uint
	var admin Player
	if db.Where("role = ?", "admin").First(&admin).Error == nil {
		owner = admin.ID
	}
	us := newTeam(w.ID, "INF", "US", owner, "sainte-marie")
	ger := newTeam(w.ID, "INF", "GER", owner, "arland-ferme")
	db.Create(&us)
	db.Create(&ger)
	logEvent(w.ID, "war", "", "Début de la guerre : "+w.Name)
	return &w
}

func countWars() int64 {
	var n int64
	db.Model(&War{}).Count(&n)
	return n
}

func newTeam(warID uint, typ, faction string, owner uint, city string) AssaultTeam {
	s := atSpecs[typ]
	t := AssaultTeam{WarID: warID, Type: typ, Faction: faction, OwnerID: owner, Manpower: s.Manpower, EquipLevel: 1, CityID: city, Status: "idle"}
	t.SetVehicles(s.Vehicles[faction])
	return t
}

func logEvent(warID uint, typ, faction, msg string) {
	db.Create(&Event{WarID: warID, Type: typ, Faction: faction, Message: msg, CreatedAt: time.Now().UTC()})
}
