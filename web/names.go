package main

import "fmt"

// Faction-flavoured unit names for assault teams and garrisons (display only; ids stay at_N / garrison_city).
var unitNames = map[string]map[string][]string{
	"US": {
		"INF":  {"Able Company, 2nd Bn, 116th Infantry", "Baker Company, 1st Bn, 16th Infantry", "Charlie Company, 3rd Bn, 8th Infantry", "Dog Company, 2nd Bn, 22nd Infantry", "Fox Company, 2nd Bn, 175th Infantry", "George Company, 3rd Bn, 26th Infantry"},
		"PARA": {"Easy Company, 2nd Bn, 506th PIR", "Dog Company, 2nd Bn, 505th PIR", "Item Company, 3rd Bn, 501st PIR", "Fox Company, 2nd Bn, 507th PIR"},
		"REC":  {"C Troop, 102nd Cavalry Recon", "B Troop, 4th Cavalry Recon Sqn", "A Troop, 24th Cavalry Recon Sqn"},
		"ARM":  {"B Company, 741st Tank Bn", "C Company, 70th Tank Bn", "A Company, 743rd Tank Bn", "Dog Company, 746th Tank Bn"},
		"MOT":  {"Able Company, 1st Bn, 359th Infantry (Mot.)", "Baker Company, 2nd Bn, 357th Infantry (Mot.)", "Charlie Company, 41st Armored Infantry"},
	},
	"GER": {
		"INF":  {"1. Kompanie, Grenadier-Regiment 916", "3. Kompanie, Grenadier-Regiment 914", "5. Kompanie, Grenadier-Regiment 726", "7. Kompanie, Grenadier-Regiment 919", "2. Kompanie, Grenadier-Regiment 1058"},
		"PARA": {"Fallschirmjäger-Kompanie 6/6", "3. Kompanie, Fallschirmjäger-Regiment 6", "9. Kompanie, Fallschirmjäger-Regiment 3"},
		"REC":  {"Aufklärungs-Abteilung 352", "Panzer-Aufklärungs-Abteilung 21", "Aufklärungs-Abteilung 709"},
		"ARM":  {"Panzer-Kompanie 3, Pz.Rgt. 22", "4. Kompanie, Panzer-Regiment 130", "2. Kompanie, Panzer-Abteilung 100", "Sturmgeschütz-Batterie 902"},
		"MOT":  {"Panzergrenadier-Kompanie 2, PzGrenRgt 125", "6. Kompanie, Panzergrenadier-Regiment 192"},
	},
}

var garrisonSuffix = map[string]string{"US": "MP Detachment", "GER": "Landesschützen"}

// teamDisplayName picks a name from the faction/type list, cycling with the team number.
func teamDisplayName(faction, typ string, n uint) string {
	l := unitNames[faction][typ]
	if len(l) == 0 {
		l = unitNames[faction]["INF"]
	}
	if len(l) == 0 {
		return fmt.Sprintf("Team %d", n)
	}
	name := l[int(n)%len(l)]
	if cycle := int(n) / len(l); cycle > 0 {
		name = fmt.Sprintf("%s (%d)", name, cycle+1)
	}
	return name
}

func garrisonDisplayName(cityName, faction string) string {
	s := garrisonSuffix[faction]
	if s == "" {
		s = "Garrison"
	}
	return "Garrison " + cityName + " — " + s
}

// backfillTeamNames names teams created before display names existed.
func backfillTeamNames() {
	var ts []AssaultTeam
	db.Where("display_name = '' OR display_name IS NULL").Find(&ts)
	for _, t := range ts {
		db.Model(&AssaultTeam{}).Where("id = ?", t.ID).Update("display_name", teamDisplayName(t.Faction, t.Type, t.ID))
	}
}

// Label: display name for logs, falls back to the internal id.
func (t *AssaultTeam) Label() string {
	if t.DisplayName != "" {
		return t.DisplayName
	}
	return t.TeamID()
}
