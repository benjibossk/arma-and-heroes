# Medical model (bandages, medics, revive)

## What Heroes & Generals did
- Every infantryman carried bandages: stop the bleeding and give back a bit of health. Health did not regenerate fully on its own.
- Medic class: medkit / first-aid that heals teammates (much faster and to full), plus extra bandages.
- Health crates / resupply: refill bandages and health at a box.
- No real revive in most versions: a downed soldier was dead (respawn by the team ticket system).
- Healing others gave XP and fed the medic ribbons (Field Medic, ...).

## What Reforger + Finest Hour give
- Vanilla medical system: per-hitzone damage, BLEEDING damage-over-time, blood hitzone, resilience hitzone (unconscious when it drops), regeneration.
- Consumables (`SCR_ConsumableItemComponent` + `SCR_ConsumableEffect*`): Bandage (stop a limb bleeding), Tourniquet, Morphine, Saline bag (blood), each usable on self or on another soldier (`ApplyEffect(target, user, ...)`). Usage speed factor via labels/group role (`GetUsageSpeedFactor`).
- `SCR_GameModeHealthSettings` on the game mode: permit unconsciousness, bleeding/regen scales, kill indefinitely unconscious players. Vanilla wake-up = blood/resilience restored (saline, bandages, time).
- Heal support station / medical arsenal boxes (`AmmoBoxArsenal_Medical_*`).
- FH prefabs (WW2 server mods): `CarlisleFieldDressing_US_WW2 {9070F5AB358FE739}`, `AirborneFieldDressing_US_WW2 {133721236588F5F0}`, `FieldDressing_HEER_WW2 {4DC939F9FA9D5349}`, `MorphineInjection_01 {0D9A5DCF89AE7AA9}`, `SalineBag_US_01 {00E36F41CA310E2A}`, `Tourniquet_US_01 {D70216B1B2889129}`, `MedicalKit_01_HEER` (GERGear), medic boxes `AmmoBoxArsenal_Medical_US_WW2 {71EC0C461157250A}` / `_HEER {51645317AE0AB785}`, medic characters `Character_FH_*Medic*`.

## Arma & Heroes rules (F44_Core `Scripts/Game/F44/F44_Medical.c`)
| rule | default | key in `$profile:F44/medical.json` |
|---|---|---|
| field dressings for everybody (top-up at spawn, players and AI) | 2 | `bandages` |
| medic: dressings / morphine / saline | 6 / 2 / 1 | `medicBandages`, `medicMorphine`, `medicSaline` |
| any bandage: stops bleeding (vanilla) + small heal | +20 HP | `bandageHeal` |
| medic treating someone else: full heal | on | `medicFullHeal` |
| medic usage speed (vanilla divider) | x2 | `medicSpeed` |
| unconsciousness | on | `unconscious` |
| revive = a medic uses any medical item on an unconscious teammate (full heal + wake up); others can only bandage | on | `medicRevive` |
| bleed-out: unconscious longer than this dies (killer credit kept) | 60 s | `bleedOutSec` (0 = vanilla) |

Medic = catalog class key containing `MEDIC` (players, from the spawn handler) or prefab name containing `medic` (AI).
Dressing faction: HEER/GER prefab -> German dressing, else Carlisle (US).

## BattleResult counters (see RIBBONS.md)
Per player, only when treating **another** soldier: `healGiven` (treatments), `revives`, `stats.bandagesApplied`, `stats.healHp` (health points restored).

## Supply crates hooks
The medical crate ("hold F to heal slowly", supply agent) stays in its own component. To refill dressings from it, call
`F44_Medical.GiveKit(character)` (idempotent top-up to the class kit). Not duplicated here.
