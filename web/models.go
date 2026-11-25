package main

import (
	"encoding/json"
	"time"
)

type War struct {
	ID           uint      `gorm:"primaryKey" json:"id"`
	Name         string    `gorm:"size:100" json:"name"`
	Status       string    `gorm:"size:20" json:"status"` // active | finished
	Winner       string    `gorm:"size:8" json:"winner"`
	StartedAt    time.Time `json:"startedAt"`
	EndedAt      *time.Time `json:"endedAt"`
	LastIncomeAt time.Time `json:"lastIncomeAt"`
}

type Faction struct {
	ID        uint   `gorm:"primaryKey" json:"-"`
	WarID     uint   `gorm:"index" json:"-"`
	Code      string `gorm:"size:8" json:"code"` // US | GER
	Name      string `gorm:"size:60" json:"name"`
	Resources int    `json:"resources"`
}

type City struct {
	ID         string `gorm:"primaryKey;size:64" json:"id"`
	WarID      uint   `gorm:"index" json:"-"`
	Name       string `gorm:"size:100" json:"name"`
	X          int    `json:"x"`
	Y          int    `json:"y"`
	Owner      string `gorm:"size:8" json:"owner"`
	IsCapital  bool   `json:"isCapital"`
	IsFactory  bool   `json:"isFactory"`
	ScenarioID string `gorm:"size:255" json:"scenarioId"`
	Terrain    string `gorm:"size:100" json:"terrain"`
	Mode       string `gorm:"size:16" json:"mode"` // ASSAULT | SKIRMISH
	Garrison   int    `json:"garrison"`           // manpower de la garnison si aucune AT ne défend
}

type Road struct {
	ID       uint   `gorm:"primaryKey" json:"id"`
	WarID    uint   `gorm:"index" json:"-"`
	CityA    string `gorm:"size:64" json:"cityA"`
	CityB    string `gorm:"size:64" json:"cityB"`
	Distance int    `json:"distance"` // km
}

type VehicleCount struct {
	Key   string `json:"key"`
	Count int    `json:"count"`
}

type AssaultTeam struct {
	ID         uint       `gorm:"primaryKey" json:"-"`
	WarID      uint       `gorm:"index" json:"-"`
	Type       string     `gorm:"size:8" json:"type"`
	Faction    string     `gorm:"size:8" json:"faction"`
	OwnerID    uint       `json:"ownerId"`
	Manpower   int        `json:"manpower"`
	EquipLevel int        `json:"equipLevel"`
	Vehicles   string     `gorm:"type:text" json:"-"`
	CityID     string     `gorm:"size:64" json:"cityId"`
	DestCityID string     `gorm:"size:64" json:"destCityId"`
	DepartAt   *time.Time `json:"departAt"`
	ArrivalAt  *time.Time `json:"arrivalAt"`
	Status     string     `gorm:"size:16;index" json:"status"` // idle | moving | in_battle | destroyed | dissolved
	BattleID   uint       `json:"-"`
	CreatedAt  time.Time  `json:"createdAt"`
}

func (t *AssaultTeam) TeamID() string { return "at_" + itoa(int(t.ID)) }

func (t *AssaultTeam) GetVehicles() []VehicleCount {
	var v []VehicleCount
	_ = json.Unmarshal([]byte(t.Vehicles), &v)
	if v == nil {
		v = []VehicleCount{}
	}
	return v
}

func (t *AssaultTeam) SetVehicles(v []VehicleCount) {
	out := []VehicleCount{}
	for _, x := range v {
		if x.Count > 0 {
			out = append(out, x)
		}
	}
	b, _ := json.Marshal(out)
	t.Vehicles = string(b)
}

// SideTeams : par camp, liste des teamId engagés (sérialisé dans Battle.Sides)
type SideTeams map[string][]string

type Battle struct {
	ID             uint       `gorm:"primaryKey" json:"-"`
	WarID          uint       `gorm:"index" json:"warId"`
	Status         string     `gorm:"size:16;index" json:"status"` // pending | starting | running | finished | cancelled
	CityID         string     `gorm:"size:64" json:"cityId"`
	Mode           string     `gorm:"size:16" json:"mode"`
	Attacker       string     `gorm:"size:8" json:"attacker"`
	Defender       string     `gorm:"size:8" json:"defender"`
	TimeLimitMin   int        `json:"timeLimitMin"`
	Sides          string     `gorm:"type:text" json:"-"`
	GarrisonMP     int        `json:"garrison"`
	Result         string     `gorm:"type:mediumtext" json:"-"`
	Winner         string     `gorm:"size:8" json:"winner"`
	ServerInstance string     `gorm:"size:64" json:"serverInstance"`
	CreatedAt      time.Time  `json:"createdAt"`
	ClaimedAt      *time.Time `json:"claimedAt"`
	HeartbeatAt    *time.Time `json:"heartbeatAt"`
	FinishedAt     *time.Time `json:"finishedAt"`
	LastStats      string     `gorm:"type:text" json:"-"`
}

func (b *Battle) BattleID() string { return "b_" + pad6(int(b.ID)) }

func (b *Battle) GetSides() SideTeams {
	s := SideTeams{}
	_ = json.Unmarshal([]byte(b.Sides), &s)
	return s
}

func (b *Battle) SetSides(s SideTeams) {
	bs, _ := json.Marshal(s)
	b.Sides = string(bs)
}

type Player struct {
	ID         uint      `gorm:"primaryKey" json:"id"`
	Name       string    `gorm:"size:64;uniqueIndex" json:"name"`
	IdentityID string    `gorm:"size:64;index" json:"identityId"`
	DiscordID  string    `gorm:"size:32;index" json:"-"`
	PassHash   string    `gorm:"size:100" json:"-"`
	Role       string    `gorm:"size:16" json:"role"` // admin | general | soldier
	Faction    string    `gorm:"size:8" json:"faction"`
	XP         int       `json:"xp"`
	Credits    int       `json:"credits"`
	Kills      int       `json:"kills"`
	Deaths     int       `json:"deaths"`
	Captures   int       `json:"captures"`
	Battles    int       `json:"battles"`
	CreatedAt  time.Time `json:"createdAt"`
}

func (p *Player) IsGeneral() bool { return p.Role == "general" || p.Role == "admin" }

type Session struct {
	Token    string `gorm:"primaryKey;size:64"`
	PlayerID uint
	Expires  time.Time
}

type Event struct {
	ID        uint      `gorm:"primaryKey" json:"id"`
	WarID     uint      `gorm:"index" json:"warId"`
	Type      string    `gorm:"size:24" json:"type"` // order | battle | capture | income | war | admin
	Faction   string    `gorm:"size:8" json:"faction"`
	Message   string    `gorm:"size:500" json:"message"`
	CreatedAt time.Time `gorm:"index" json:"createdAt"`
}

// PlayerBattle : participation d'un joueur à une bataille (historique profil)
type PlayerBattle struct {
	ID       uint   `gorm:"primaryKey" json:"-"`
	PlayerID uint   `gorm:"index" json:"-"`
	BattleID uint   `gorm:"index" json:"battleId"`
	Side     string `gorm:"size:8" json:"side"`
	TeamID   string `gorm:"size:16" json:"teamId"`
	Kills    int    `json:"kills"`
	Deaths   int    `json:"deaths"`
	Captures int    `json:"captures"`
	XP       int    `json:"xp"`
}

var ranks = []struct {
	XP   int
	Name string
}{{0, "Soldat"}, {500, "Caporal"}, {1500, "Sergent"}, {4000, "Sergent-chef"}, {8000, "Adjudant"}, {15000, "Sous-lieutenant"}, {25000, "Lieutenant"}, {40000, "Capitaine"}}

func rankFor(xp int) string {
	r := ranks[0].Name
	for _, x := range ranks {
		if xp >= x.XP {
			r = x.Name
		}
	}
	return r
}

// Catalogue logique par type d'AT
type ATSpec struct {
	Cost     int
	Manpower int
	Classes  []string
	Vehicles map[string][]VehicleCount
	Label    string
}

var atSpecs = map[string]ATSpec{
	"INF": {300, 120, []string{"RIFLEMAN", "MG", "MEDIC", "AT", "OFFICER"}, map[string][]VehicleCount{}, "Infanterie"},
	"PARA": {450, 80, []string{"PARA_RIFLEMAN", "PARA_MG", "MEDIC", "OFFICER"}, map[string][]VehicleCount{}, "Parachutistes"},
	"ARM": {800, 60, []string{"CREW", "RIFLEMAN"}, map[string][]VehicleCount{
		"US": {{"US_M4A2", 3}}, "GER": {{"GER_PZIV", 2}, {"GER_SDKFZ222", 1}}}, "Blindés"},
	"REC": {250, 40, []string{"SNIPER", "SCOUT", "RIFLEMAN"}, map[string][]VehicleCount{
		"US": {{"US_JEEP", 2}}, "GER": {{"GER_KUBELWAGEN", 2}}}, "Reconnaissance"},
	"MOT": {400, 100, []string{"RIFLEMAN", "MG", "DRIVER"}, map[string][]VehicleCount{
		"US": {{"US_GMC", 2}, {"US_JEEP", 1}}, "GER": {{"GER_OPEL_BLITZ", 2}, {"GER_SDKFZ251", 1}}}, "Motorisée"},
}
