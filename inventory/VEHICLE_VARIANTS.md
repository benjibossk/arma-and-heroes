# Vehicle variants — unarmed / armed pairs (installed mods only)

Source: `inventory/vehicles.json` / `VEHICLES.md` (built from each installed mod's `resourceDatabase.rdb`, mod list =
`arma-reforger-ww2/config.json`). A pair = same model, one prefab without and one with a mounted MG.
Used by `web/teams.json` `variants` (base = unarmed prefab sent to the mod, armed = bought upgrade).

| Key | Model | Mod | Unarmed (base) | Armed | Armed cost |
|---|---|---|---|---|---|
| `GER_KFZ` | VW Kübelwagen | FH GER Vehicles | `{1D3D9E6ED1DED30D}Kubelwagen.et` (also `Kubelwagen_Canvas.et`) | `{2F09C97004CFD1B5}Kubelwagen_MG34.et` | 40 |
| `US_JEEP` | Willys Jeep | Operation Overlord US Vehicles | `{0CFB9416659192D2}Willys Jeep_uncovered.et` (also `_covered`, `_Medical`) | `{10448D0351BF0979}Willys Jeep_MG.et` | 40 |
| `US_TRUCK` | Dodge WC-63 6x6 | Operation Overlord US Vehicles | `{773E8E154D18321C}WC63_transport.et` (also `WC63`, `_Covered`, `_Uncovered`) | `{B12E900E074349E4}WC63_covered_mg.et` (ring mount) | 60 |
| `GER_SDKFZ251` | Sd.Kfz. 251 | SDKFZ 251 | `{D253646DF96D27EA}Sd.kfz_251_Transport.et` (+ `_Camo`) | `{561C8A71E69790A1}Sd.kfz_251_MG.et` (+ `_MG_camo` `{AB976FB2C94E3BF9}`) | 60 |
| `US_M3A1` | M3A1 halftrack | M3A1 HalfTrack | `{EC62ED156FC6F05C}M3A1 HalfTrack_Transport.et` | `{80C7FDE83DE6D000}M3A1 HalfTrack_MG.et` | 60 |

Notes
- `US_JEEP` / `US_TRUCK` in `catalog.json` still point at vanilla M151A2 / M923A1 proxies; with `variants.base` the
  battle payload now sends the WW2 Willys / WC-63 prefab instead. No GMC CCKW is installed (WC-63 is the US truck).
- `GER_SDKFZ251` and `US_M3A1` have no `catalog.json` entry: the `prefab` field is what makes them spawn; their
  ticket cost (catalog `cost`) is 0 until a catalog entry is added.

## No MG pair
- **Opel Blitz** (`GER_TRUCK`): transport / covered / command / repair / ammo… but the only armed ones are
  `Opil_Bletz_Armed_Flak38` / `_Flakvierling` (20 mm AA gun, not an MG mount) — not offered as an upgrade.
- **Sd.Kfz. 222** (armed only, 20 mm + MG), **Sd.Kfz. 223** (radio car, MG only): no unarmed twin.
- Tanks (Sherman, Panzer IV family, Tiger, Sturmtiger, Jagdpanzer): always armed.
