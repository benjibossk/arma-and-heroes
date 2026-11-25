"""Extract vehicle stats (seats, armament, mass, hull) from the installed WW2 vehicle mods for every
vehicle of web/progression.json, and write web/vehicle_stats.json (served by /api/v1/vehicles/stats).

Usage:  python -I tools/vehicle-stats/extract.py [--reextract]

Sources (read-only): each vehicle mod's data*.pak unpacked (.et/.conf) into EXTRACT_DIR with
C:/Users/benbo/tools/PakTools/pakextract.py, plus vanilla Reforger prefabs (BASE_DIR) for inheritance.
A prefab is resolved through its parent chain AND its child slot prefabs (seats, turrets) recursively.
Values absent from the files stay null (never invented). Speed is not stored in the prefabs.
"""
import json, os, re, subprocess, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
ADDONS = 'D:/Serveur/arma-reforger-ww2/profile/addons'
PAKX = 'C:/Users/benbo/tools/PakTools/pakextract.py'
EXTRACT_DIR = 'C:/Users/benbo/tools/PakInspector/veh_extract'
BASE_DIR = 'C:/Users/benbo/tools/PakInspector/base_et_conf_extract'
MODS = ['FinestHour-GERVehicles_693AE5B800A316FE', 'OperationOverlordUSVehicles_65AD797399E3CF39',
        'M3A1HalfTrack_65BD1CC2A0110122', 'M4A2Sherman_672EFC7887D6AEA7', 'RMSWW2PANZERIV_692C3806D8C77463',
        'SDKFZ251_65C7179DA0B82317', 'Tiger_H1_68E42575A16D4FD1', 'WW2TankCore_696372CF8EB6C647',
        'CIETrackedCore_68CD35053063C6D5', 'ModestVehicleCore_6A7D4E7B69969355']
# dummies + modern vanilla templates some WW2 prefabs inherit from their BTR/LAV/Mi-8 donor (overridden in game)
IMG = {'US_JEEP': 'US_JEEP_WILLYS.webp', 'US_TRUCK': 'US_WC63_UNCOVERED.webp', 'US_SHERMAN': 'US_SHERMAN_M4A2.webp',
       'GER_PZ4': 'GER_PZ4_side.svg'}  # web/static/assets/items renders
SKIP_WEAPON = re.compile(r'dummy|periscope|_empty|invis|M242|LAV25|BM21|Mi8|M1025|PKMT', re.I)


def ensure_extract(force):
    for m in MODS:
        out = os.path.join(EXTRACT_DIR, m)
        if force or not os.path.isdir(out):
            paks = sorted(p for p in os.listdir(os.path.join(ADDONS, m)) if re.match(r'data\d*\.pak$', p))
            subprocess.run([sys.executable, '-I', PAKX, out] + [os.path.join(ADDONS, m, p) for p in paks]
                           + ['--only', 'et,conf'], check=True, stdout=subprocess.DEVNULL)


def build_index():
    idx = {}
    for root in [os.path.join(EXTRACT_DIR, m) for m in MODS] + [BASE_DIR]:
        for d, _, fs in os.walk(root):
            for f in fs:
                if f.endswith('.et'):
                    full = os.path.join(d, f)
                    rel = os.path.relpath(full, root).replace('\\', '/').lower()
                    idx.setdefault(rel, full)
                    idx.setdefault('#' + f.lower(), full)
    return idx


IDX = {}


def resolve(ref):
    rel = re.sub(r'^\{[0-9A-F]{16}\}', '', ref or '').lower()
    return IDX.get(rel) or IDX.get('#' + rel.rsplit('/', 1)[-1]) if rel else None


def chain(ref):
    out, seen, p = [], set(), resolve(ref)
    while p and p not in seen:
        seen.add(p)
        t = open(p, encoding='utf-8', errors='replace').read()
        out.append(t)
        m = re.match(r'\s*\w+\s*:\s*"([^"]+)"', t)
        p = resolve(m.group(1)) if m else None
    return out


def wname(ref):
    n = ref.rsplit('/', 1)[-1][:-3]
    n = re.sub(r'^(MG|HMG|Cannon|Autocannon)_', '', n)
    return re.sub(r'_(Base|Grey|Camo|Desert|Turret|coax|pintle|HE|AP|HEIT|APDST)$', '', n, flags=re.I).replace('_', ' ')


def walk(ref, depth=0, seen=None):
    """seats by type, weapon refs, over the prefab and its child slot prefabs."""
    seen = seen if seen is not None else set()
    seats, weapons = {'pilot': set(), 'turret': set(), 'cargo': set()}, []
    ch, slots = chain(ref), {}
    for t in ch:
        for m in re.finditer(r'^\s*(Pilot|Turret|Cargo)CompartmentSlot (\w+)', t, re.M):
            seats[m.group(1).lower()].add(m.group(2))
        # weapon slots keyed by ID: the leaf-most prefab (first in chain) overrides its parents
        for m in re.finditer(r'WeaponSlotComponent "\{(\w+)\}" \{(?:(?!\n  \w+Component).){0,1500}?WeaponTemplate "([^"]+)"', t, re.S):
            slots.setdefault(m.group(1), m.group(2))
    for w in slots.values():
        if not SKIP_WEAPON.search(w) and w not in weapons:
            weapons.append(w)
    out = {k: len(v) for k, v in seats.items()}
    if depth < 4:
        kids = []
        for t in ch:
            for m in re.finditer(r'^\s*Prefab "([^"]+\.et)"', t, re.M):
                r = m.group(1)
                if not re.search(r'/lights?/|wheel|/Magazines/|/Ammo/|/Items/|/Characters/|/Grenades/|/Handguns/|/Mines/|/Explosives/', r, re.I) and r not in kids:
                    kids.append(r)
        for r in kids:
            p = resolve(r)
            if not p or p in seen:
                continue
            seen.add(p)
            s, w = walk(r, depth + 1, seen)
            for k in out:
                out[k] += s[k]
            weapons += [x for x in w if x not in weapons]
    return out, weapons


def first(ch, pat, cast=float):
    for t in ch:
        m = re.search(pat, t, re.M | re.S)
        if m:
            return cast(m.group(1))
    return None


def stats(prefab):
    ch = chain(prefab)
    if not ch:
        return None
    seats, weapons = walk(prefab)
    names = []
    for w in weapons:
        n = wname(w)
        if n not in names:
            names.append(n)
    hull = None
    for t in ch:
        m = re.search(r'HitZone Hull \{(.{0,600})', t, re.S)
        if m:
            hull = m.group(1)
            break
    return {
        'prefab': prefab,
        'seats': sum(seats.values()) or None, 'crew': seats,
        'armament': names,
        'mass': first(ch, r'^\s*Mass ([\d.]+)', lambda x: round(float(x))),
        'hullHealth': first([hull], r'MaxHealth ([\d.]+)', lambda x: round(float(x))) if hull else None,
        'hullReduction': first([hull], r'DamageReduction ([\d.]+)') if hull else None,
        'parentChain': len(ch),
    }


def main():
    global IDX
    ensure_extract('--reextract' in sys.argv)
    IDX = build_index()
    load = lambda f: json.load(open(os.path.join(REPO, 'web', f), encoding='utf-8'))
    prog, teams, cat = load('progression.json'), load('teams.json'), load('catalog.json')['catalog']
    inv = json.load(open(os.path.join(REPO, 'inventory', 'vehicles.json'), encoding='utf-8'))
    # team pools (web/models.go atSpecs): "ARM": {..., map{"US": {{"US_SHERMAN", 3}}, ...}, "Armor"}
    src = open(os.path.join(REPO, 'web', 'models.go'), encoding='utf-8').read()
    pools = {}
    for m in re.finditer(r'"(\w+)":\s*\{\d+, \d+, \[\]string\{[^}]*\}, map\[string\]\[\]VehicleCount\{(.*?)\}, "([^"]+)"\}', src, re.S):
        for k, n in re.findall(r'\{"(\w+)", (\d+)\}', m.group(2)):
            pools.setdefault(k, []).append({'type': m.group(1), 'label': m.group(3), 'count': int(n)})
    out = {}
    for key, v in prog['vehicles'].items():
        var = teams.get('variants', {}).get(key, {})
        c = cat.get(key, {})
        prefab = var.get('base') or (c.get('prefab') if c.get('status') != 'proxy' else None) or inv.get(key, {}).get('prefab')
        e = {'label': v['label'], 'faction': v['faction'], 'role': teams['vehicleRoles'].get(key),
             'source': c.get('source') or inv.get(key, {}).get('mod'), **(stats(prefab) or {'prefab': prefab})}
        iseats = inv.get(key, {}).get('seats') or (inv.get(key + '_WILLYS') or {}).get('seats')
        if iseats and (e.get('seats') or 0) < iseats:  # seats spawned by scripts/child slots the walker misses
            e['seats'], e['seatsSource'] = iseats, 'inventory/vehicles.json (resourceDatabase)'
        e['img'] = IMG.get(key, key + '.webp')
        e['teams'] = pools.get(key, [])
        e['allowedIn'] = [t for t, roles in teams['allowedRoles'].items() if e['role'] in roles]
        # mod F44_MobileSpawn.c: players spawn in a free seat of a half-track / truck, or any tank of an Armor team
        e['mobileSpawn'] = e['role'] in ('truck', 'tank') or (e['role'] == 'light_armored' and key != 'GER_SDKFZ222')  # 222 = armoured car
        if c.get('crew'):
            e['crewNote'] = c['crew']
        if var.get('armed'):
            a = stats(var['armed'])
            e['armed'] = {'label': var.get('armedLabel'), 'cost': var.get('armedCost'),
                          'armament': a and a['armament'], 'seats': a and a['seats']}
        out[key] = e
        print(key, e.get('seats'), e.get('armament'), e.get('mass'), e.get('hullHealth'), file=sys.stderr)
    doc = {'_meta': {'generated_by': 'tools/vehicle-stats/extract.py',
                     'sources': 'vehicle mods data.pak (.et) + vanilla Reforger base prefabs',
                     'units': {'mass': 'kg', 'hullReduction': 'flat damage reduction of the Hull hit zone'},
                     'null': 'valeur absente des fichiers du mod (non inventee)'}, 'vehicles': out}
    with open(os.path.join(REPO, 'web', 'vehicle_stats.json'), 'w', encoding='utf-8') as f:
        json.dump(doc, f, ensure_ascii=False, indent=1)


if __name__ == '__main__':
    main()
