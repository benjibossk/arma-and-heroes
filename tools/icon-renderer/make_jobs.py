"""Build the AH_IconRenderer job list from catalog.json + inventory/vehicles.json.
Usage: python make_jobs.py <out ah_jobs.txt> [--keys K1,K2,...] [--side]
  --keys  only these keys;  --side  vehicles only, spawned in the world and seen from the side
          (camera delta "10,-48,0"), for the SVG pictograms (tools/icon-silhouettes/side/)
Line: KEY|item|<prefab>||<cat>   or   KEY|char|<character>|<cloth,...>|char
"""
import json, os, sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
cat = json.load(open(os.path.join(ROOT, "catalog.json"), encoding="utf-8"))
veh = json.load(open(os.path.join(ROOT, "inventory", "vehicles.json"), encoding="utf-8"))

CHAR = {
    "US": "{77BA10429E70E72E}Prefabs/Characters/Factions/BLUFOR/US_Army/USA_Infantry/Character_FH_USAWW2_Infantry_Rifleman_Garand_Mid.et",
    "GER": "{23C2C71CBBC84D26}Prefabs/Characters/Factions/OPFOR/HEER/HEER_Infantry/Character_FH_HEERWW2_Rifleman_KAR98_Infantry_Mid.et",
}
WEAPON_CATS = {"rifle", "smg", "mg", "sniper", "launcher", "pistol", "at", "lmg"}
# outfit item -> extra clothing worn with it (plain base appearance otherwise)
OUTFIT_FROM_TIER = {"GER_OUTFIT_FJ": ("GER", "Regular", "Para")}

lines = []
for k, it in cat["items"].items():
    f = it.get("faction", "US")
    if it.get("slot") == "outfit" or it.get("category") == "outfit":
        if k in OUTFIT_FROM_TIER:
            fa, t, r = OUTFIT_FROM_TIER[k]
            cloth = cat["aiAppearance"][fa][t][r]
        elif it.get("prefab"):
            cloth = [it["prefab"]]
        else:
            continue
        lines.append(f"{k}|char|{CHAR[f]}|{','.join(cloth)}|char")
        continue
    if not it.get("prefab"):
        continue
    c = "weapon" if it.get("category") in WEAPON_CATS or it.get("slot") in ("primary", "secondary") else "item"
    if it.get("slot") == "sidearm" or it.get("category") == "pistol":
        c = "item"
    lines.append(f"{k}|item|{it['prefab']}||{c}")

for k, v in veh.items():
    if not v.get("prefab") or k.startswith("OTHER_") or "SIMTEST" in k:
        continue
    lines.append(f"{k}|item|{v['prefab']}||veh")

for fa, tiers in cat["aiAppearance"].items():
    for t, roles in tiers.items():
        for r, cloth in roles.items():
            lines.append(f"AI_{fa}_{t}_{r}|char|{CHAR[fa]}|{','.join(cloth)}|char")

args = sys.argv[2:]
if "--keys" in args:
    keep = set(args[args.index("--keys") + 1].split(","))
    lines = [l for l in lines if l.split("|")[0] in keep]
if "--side" in args:
    lines = [l.replace("|item|", "|spawn|", 1) + "|10,-48,0" for l in lines if l.endswith("|veh")]
open(sys.argv[1], "w", encoding="utf-8").write("\n".join(lines) + "\n")
print(len(lines), "jobs")
