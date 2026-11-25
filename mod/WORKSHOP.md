# Workshop page — Arma & Heroes - Core

- Visibility: **Public**
- Preview image: `mod/workshop_preview.jpg` (1280×720)
- Tags: Game Mode, Multiplayer, Scenario, Historical, WW2
- Dependencies: Arma Reforger only (the server adds the content mods, see below)

## Summary
A persistent WW2 war: generals move the front on the web, soldiers fight it in Arma Reforger.

## Description
Arma & Heroes is a persistent World War II campaign for Arma Reforger.

On the website (heroes.collectifxxl.fr), generals raise assault teams and move them across a strategic war map of Normandy. When two teams meet in a city, a battle starts: our orchestrator launches a dedicated server with exactly the troops, vehicles and tickets the generals brought. The result goes back to the war map.

In game:
- Assault and Capture & Hold on sectors, with tickets from the assault teams
- AI bots fill both sides, capture sectors and cost tickets when they die. Their skill and look adapt to the players in the match
- Vehicle pool from the teams: jeeps, trucks, halftracks, Sherman, Panzer IV, Tiger I
- Persistent soldiers: rank, credits, unlocked weapons and loadouts from the website
- Link your game account to Discord with a one-time code or QR code (works on console)
- Live battle view on the website

This mod is the game mode only. Servers running it also load Finest Hour, Chungus and WW2 vehicle mods; all credit goes to their authors.

Source code (MIT): https://github.com/benjibossk/arma-and-heroes

## Changelog (first release)
First public release: game mode, sectors, tickets, AI bots, vehicle pool, loadouts, account linking, live view.

## Changelog 1.0.1 (ready, not yet published by this change)
Battle results are now more reliable: the result POST to the website is retried up to 3 times with backoff (5 s, 10 s), and an HTTP 409 reply (battle already finished) counts as success. Fixes a battle reported as failed (http 0) when the website answered slowly although it had saved the result.

