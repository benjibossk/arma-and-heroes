package main

import (
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"log"
	"os"
	"strconv"
	"sync"
	"time"

	"gorm.io/driver/mysql"
	"gorm.io/gorm"
	"gorm.io/gorm/logger"
)

var (
	db  *gorm.DB
	cfg Config
	// gameMu sérialise toutes les mutations de l'état de guerre (tick, ordres, résultats)
	gameMu sync.Mutex
)

type Config struct {
	DSN            string
	ServerKey      string
	StaticDir      string
	PublicURL      string
	TickSec        int
	IncomeMin      int
	TravelMinPerKm float64
	DiscordID      string
	DiscordSecret  string
	DiscordRedir   string
	AdminUser      string
	AdminPass      string
}

func env(k, d string) string {
	if v := os.Getenv(k); v != "" {
		return v
	}
	return d
}

func envInt(k string, d int) int {
	if v, err := strconv.Atoi(os.Getenv(k)); err == nil {
		return v
	}
	return d
}

func itoa(i int) string { return strconv.Itoa(i) }
func pad6(i int) string  { return fmt.Sprintf("%06d", i) }

func randToken(n int) string {
	b := make([]byte, n)
	_, _ = rand.Read(b)
	return hex.EncodeToString(b)
}

func main() {
	tpk, _ := strconv.ParseFloat(env("F44_TRAVEL_MIN_PER_KM", "3"), 64)
	cfg = Config{
		DSN:            env("F44_DB_DSN", "f44:change-me@tcp(f44-db:3306)/front44?charset=utf8mb4&parseTime=True&loc=UTC"),
		ServerKey:      os.Getenv("F44_SERVER_KEY"),
		StaticDir:      env("F44_STATIC_DIR", "./static"),
		PublicURL:      env("F44_PUBLIC_URL", "https://heroes.collectifxxl.fr"),
		TickSec:        envInt("F44_TICK_SEC", 30),
		IncomeMin:      envInt("F44_INCOME_MIN", 60),
		TravelMinPerKm: tpk,
		DiscordID:      os.Getenv("DISCORD_CLIENT_ID"),
		DiscordSecret:  os.Getenv("DISCORD_CLIENT_SECRET"),
		DiscordRedir:   os.Getenv("DISCORD_REDIRECT_URI"),
		AdminUser:      env("F44_ADMIN_USER", "admin"),
		AdminPass:      os.Getenv("F44_ADMIN_PASSWORD"),
	}
	if cfg.ServerKey == "" {
		log.Println("WARNING: F44_SERVER_KEY empty, server endpoints disabled")
	}
	var err error
	for i := 0; i < 30; i++ {
		db, err = gorm.Open(mysql.Open(cfg.DSN), &gorm.Config{Logger: logger.Default.LogMode(logger.Warn)})
		if err == nil {
			break
		}
		log.Println("DB unavailable, retrying...", err)
		time.Sleep(2 * time.Second)
	}
	if err != nil {
		log.Fatal(err)
	}
	if err := db.AutoMigrate(&War{}, &Faction{}, &City{}, &Road{}, &AssaultTeam{}, &Battle{}, &Player{}, &Session{}, &Event{}, &PlayerBattle{}, &LinkCode{}, &Setting{}, &Soldier{}, &SoldierItem{}); err != nil {
		log.Fatal(err)
	}
	loadItems()
	loadProgression()
	seedStarterKits()
	ensureAdmin()
	if currentWar() == nil {
		seedWar()
	}
	backfillTeamNames()

	go tickLoop()
	r := setupRouter()
	log.Println("Arma & Heroes listening on :8080")
	log.Fatal(r.Run(":8080"))
}
