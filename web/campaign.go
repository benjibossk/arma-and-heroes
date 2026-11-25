package main

// campaign.go — route + migration entry point for the GAP_PLAN P2/P3 web packages
// (upgrades.go, skirmish.go, economy.go, careers.go, cosmetics.go, rewards.go).
// Kept out of api.go so parallel lanes only share one registration line.

import "github.com/gin-gonic/gin"

func registerCampaignRoutes(a *gin.RouterGroup) {
	if err := db.AutoMigrate(&PlayerCareer{}); err != nil {
		panic(err)
	}
	// P2-A team doctrine upgrades
	a.GET("/teams/:id/upgrades", getTeamUpgrades)
	a.POST("/teams/:id/upgrades/:key", requireGeneral, buyTeamUpgrade)
	// P3 careers
	a.GET("/me/career", requireUser, getMyCareer)
	a.POST("/me/career", requireUser, setMyCareer)
	a.GET("/careers/by-identity/:identityId", requireServer, careerByIdentity)
}
