"""Extract real weapon stats from the installed WW2 mods (Finest Hour + vanilla base) for every
weapon of catalog.json, and write web/weapon_stats.json.

Usage:  python -I tools/weapon-stats/extract.py [--reextract]

Sources (read-only):
  * FH CoreMod / GERGear / USGear data.pak, unpacked (.et/.conf only) into EXTRACT_DIR with
    C:/Users/benbo/tools/PakTools/pakextract.py (done automatically if missing or --reextract).
  * Vanilla Reforger prefabs/configs: BASE_DIR (base_et_conf_extract), for inherited values.
Each .et is resolved through its inheritance chain (child first). Unknown values stay null.

Damage-vs-range model (documented in the JSON "_meta.model"):
  Enfusion ShellMoveComponent drag: dv/dt = -AirDrag * v^2 / Mass  =>  v(x) = v0 * exp(-AirDrag/Mass * x)
  Damage scales with kinetic energy: dmg(x) = DamageValue * (v(x)/v0)^2   (estimate, armour ignored)
"""
import json, math, os, re, subprocess, sys

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
ADDONS = 'D:/Serveur/arma-reforger-ww2/profile/addons'
PAKX = 'C:/Users/benbo/tools/PakTools/pakextract.py'
EXTRACT_DIR = 'C:/Users/benbo/tools/PakInspector/fh_extract'
BASE_DIR = 'C:/Users/benbo/tools/PakInspector/base_et_conf_extract'
MODS = {  # extract folder -> addon folder
    'FinestHour-CoreMod': 'FinestHour-CoreMod_69392E57A68724BD',
    'FinestHour-GERGear': 'FinestHour-GERGear_693931E2091E57D1',
    'FinestHour-USGear': 'FinestHour-USGear_6939316200A674C4',
}
WEAPON_CATS = {'rifle', 'smg', 'mg', 'sniper', 'pistol', 'at', 'grenade', 'explosive', 'smoke'}
RANGES = list(range(0, 1001, 50))


def ensure_extract(force):
    for name, addon in MODS.items():
        out = os.path.join(EXTRACT_DIR, name)
        if force or not os.path.isdir(out):
            paks = sorted(p for p in os.listdir(os.path.join(ADDONS, addon)) if re.match(r'data\d*\.pak$', p))
            subprocess.run([sys.executable, '-I', PAKX, out] + [os.path.join(ADDONS, addon, p) for p in paks]
                           + ['--only', 'et,conf'], check=True, stdout=subprocess.DEVNULL)


def build_index():
    roots = [(n, os.path.join(EXTRACT_DIR, n)) for n in MODS] + [('Reforger', BASE_DIR)]
    idx = {}
    for label, root in roots:
        for d, _, fs in os.walk(root):
            for f in fs:
                if f.endswith(('.et', '.conf')):
                    full = os.path.join(d, f)
                    rel = os.path.relpath(full, root).replace('\\', '/').lower()
                    idx.setdefault(rel, (label, full, rel))  # mods override base
                    idx.setdefault('#' + f.lower(), (label, full, rel))  # basename fallback (GUID moved files)
    # GUID -> real path (files moved after being referenced): inventory/extract/ALL_GUIDS.txt
    gp = os.path.join(REPO, 'inventory', 'extract', 'ALL_GUIDS.txt')
    if os.path.exists(gp):
        for line in open(gp, encoding='utf-8', errors='replace'):
            m = re.match(r'\{([0-9A-F]{16})\}(\S+)', line.strip())
            if m:
                idx['{' + m.group(1)] = m.group(2).lower()
    return idx


IDX = None


def resolve(ref):
    if not ref:
        return None
    g = re.match(r'^\{([0-9A-F]{16})\}', ref)
    if g and IDX.get('{' + g.group(1)) in IDX:
        return IDX[IDX['{' + g.group(1)]]
    rel = re.sub(r'^\{[0-9A-F]{16}\}', '', ref).lower()
    return IDX.get(rel) or IDX.get('#' + rel.rsplit('/', 1)[-1])


def read(entry):
    return open(entry[1], encoding='utf-8', errors='replace').read()


def chain(ref):
    """List of (label, path, rel, text) from the prefab up to its root ancestor."""
    out, seen = [], set()
    e = resolve(ref)
    while e and e[2] not in seen:
        seen.add(e[2])
        t = read(e)
        out.append((e[0], e[1], e[2], t))
        m = re.match(r'\s*\w+\s*:\s*"([^"]+)"', t)
        e = resolve(m.group(1)) if m else None
    return out


def first(ch, pattern, cast=float):
    for _, _, rel, t in ch:
        m = re.search(pattern, t, re.M)
        if m:
            return cast(m.group(1)), rel
    return None, None


def fire_modes(ch):
    for _, _, rel, t in ch:
        m = re.search(r'^(\s*)FireModes \{\n(.*?)^\1\}', t, re.M | re.S)
        if not m:
            continue
        modes = []
        for bm in re.finditer(r'BaseFireMode "[^"]*"(?: : "([^"]+)")? \{(.*?)\n\s{6}\}', m.group(2), re.S):
            conf, body = bm.group(1), bm.group(2)
            name = re.search(r'FireMode_(\w+)\.conf', conf or '')
            name = name.group(1).lower() if name else None
            rpm = re.search(r'RoundsPerMinute ([\d.]+)', body)
            burst = re.search(r'BurstSize (\d+)', body)
            if conf and (not rpm or not name):
                cc = chain(conf)
                if not rpm:
                    v, _ = first(cc, r'RoundsPerMinute ([\d.]+)')
                    rpm = v
                else:
                    rpm = float(rpm.group(1))
            else:
                rpm = float(rpm.group(1)) if rpm else None
            modes.append({'mode': name, 'rpm': rpm, 'burst': int(burst.group(1)) if burst else None})
        if modes:
            return modes, rel
    return [], None


def caliber(s):
    if not s:
        return None
    m = re.search(r'AmmunitionID_(\d+)x(\d+)(?:mm)?(R)?', s)
    if not m:
        return s.split('_')[-1]
    a = m.group(1)
    a = a if len(a) <= 2 else (a[:-2] + '.' + a[-2:])
    return f"{a}x{m.group(2)}mm{'R' if m.group(3) else ''}"


def ammo_stats(ammo_ref):
    ch = chain(ammo_ref)
    if not ch:
        return {}, []
    out, srcs = {}, [ch[0][2]]
    for key, pat in [('initSpeed', r'^\s*InitSpeed ([\d.]+)'), ('mass', r'^\s*Mass ([\d.e-]+)'),
                     ('damage', r'^\s*DamageValue ([\d.]+)'), ('airDrag', r'^\s*AirDrag ([\d.e-]+)'),
                     ('penetrationDepth', r'^\s*PenetrationDepth ([\d.]+)'),
                     ('diameter', r'^\s*Diameter ([\d.]+)')]:
        v, r = first(ch, pat)
        out[key] = v
        if r and r not in srcs:
            srcs.append(r)
    out['projectile'] = ch[0][3].startswith('Projectile')
    return out, srcs


def curve(v0, dmg, drag, mass):
    if not (v0 and dmg and drag and mass):
        return None
    k = drag / mass
    pts = []
    for x in RANGES:
        v = v0 * math.exp(-k * x)
        pts.append({'m': x, 'v': round(v, 1), 'dmg': round(dmg * (v / v0) ** 2, 1)})
    return pts


def weapon(key, item):
    ref = item.get('prefab')
    res = {'label': item.get('label'), 'category': item.get('category'), 'prefab': ref,
           'fireModes': [], 'rpm': None, 'magazine': None, 'magazineSize': None, 'caliber': None,
           'ammo': None, 'muzzleVelocity': None, 'damage': None, 'bulletMass': None, 'airDrag': None,
           'penetrationDepth': None, 'weight': None, 'zeroing': None, 'reloadTime': None,
           'damageCurve': None, 'source': {}}
    ch = chain(ref)
    if not ch:
        res['source']['note'] = 'prefab not found in extracted mods'
        return res
    res['source']['prefab'] = [c[0] + ':' + c[2] for c in ch]
    modes, r = fire_modes(ch)
    if modes:
        res['fireModes'] = modes
        rpms = [m['rpm'] for m in modes if m['rpm']]
        res['rpm'] = max(rpms) if rpms else None
        res['source']['fireModes'] = r
    w, r = first(ch, r'^\s*Weight ([\d.]+)')
    res['weight'], res['source']['weight'] = w, r
    rl, r = first(ch, r'^\s*ReloadDuration ([\d.]+)')
    if rl is not None and rl < 100:
        res['reloadTime'], res['source']['reloadTime'] = rl, r
    for _, _, rel, t in ch:
        rs = re.findall(r'SightRangeInfo "[^"]*" \{\s*Range [\d.]+ ([\d.]+)', t)
        if rs:
            rs = sorted({int(float(x)) for x in rs})
            res['zeroing'] = {'min': rs[0], 'max': rs[-1], 'steps': rs}
            res['source']['zeroing'] = rel
            break
    coef, r = first(ch, r'^\s*BulletInitSpeedCoef ([\d.]+)')
    # every MagazineTemplate of the chain, child first; ancestors' magazines fill values the
    # child's magazine lacks (e.g. a variant referencing a belt that is not shipped)
    mags = [m for _, _, _, t in ch for m in re.findall(r'^\s*MagazineTemplate "([^"]+)"', t, re.M)]
    mags = [m for m in mags if resolve(m)]
    mag = mags[0] if mags else None
    ammo_ref = None
    if mag:
        mch = [c for m in mags for c in chain(m)]
        res['magazine'] = mag
        res['source']['magazine'] = [c[2] for c in mch]
        n, _ = first(mch, r'^\s*MaxAmmo (\d+)', int)
        if n is None:  # count AmmoMapping slots
            for _, _, _, t in mch:
                am = re.search(r'AmmoMapping \{\s*([\d ]+)\s*\}', t)
                if am:
                    n = len(am.group(1).split())
                    break
        res['magazineSize'] = n
        cal, _ = first(mch, r'm_sAmmoCaliber "([^"]+)"', str)
        res['caliber'] = caliber(cal)
        ac, _ = first(mch, r'^\s*AmmoConfig "([^"]+)"', str)
        if ac:
            ce = resolve(ac)
            if ce:
                am = re.search(r'AmmoResourceArray \{\s*"([^"]+)"', read(ce))
                ammo_ref = am.group(1) if am else None
                res['source']['ammoConfig'] = ce[2]
        if not ammo_ref:
            ammo_ref, _ = first(mch, r'^\s*AmmoTemplate "([^"]+)"', str)
    if not res['caliber']:
        cal, _ = first(ch, r'm_sCaliber "([^"]+)"', str)
        res['caliber'] = caliber(cal)
    if ammo_ref:
        a, s = ammo_stats(ammo_ref)
        res['ammo'] = ammo_ref
        res['source']['ammo'] = s
        if a.get('projectile'):
            v0 = a['initSpeed'] * (coef or 1.0) if a.get('initSpeed') else None
            res['muzzleVelocity'] = round(v0, 1) if v0 else None
            res['damage'], res['bulletMass'] = a.get('damage'), a.get('mass')
            res['airDrag'], res['penetrationDepth'] = a.get('airDrag'), a.get('penetrationDepth')
            if coef:
                res['source']['muzzleCoef'] = coef
            if res['category'] not in ('at',):
                res['damageCurve'] = curve(v0, a.get('damage'), a.get('airDrag'), a.get('mass'))
    return res


def main():
    global IDX
    ensure_extract('--reextract' in sys.argv)
    IDX = build_index()
    cat = json.load(open(os.path.join(REPO, 'catalog.json'), encoding='utf-8'))['items']
    out = {'_meta': {
        'generated_by': 'tools/weapon-stats/extract.py',
        'sources': 'FH CoreMod/GERGear/USGear data.pak (.et/.conf) + vanilla Reforger base prefabs',
        'model': 'v(x)=v0*exp(-AirDrag/Mass*x); dmg(x)=DamageValue*(v/v0)^2 (estimation, sans armure)',
        'units': {'rpm': 'coups/min', 'muzzleVelocity': 'm/s', 'weight': 'kg', 'bulletMass': 'kg',
                  'zeroing': 'm', 'reloadTime': 's', 'damageCurve.m': 'm'},
        'null': 'valeur absente des fichiers du mod (non inventee)'}, 'weapons': {}}
    for k, it in cat.items():
        if it.get('category') in WEAPON_CATS:
            w = weapon(k, it)
            if (w['damage'] or 0) > 1000:
                w['warnings'] = ['DamageValue %g tel quel dans le mod (valeur aberrante, a verifier)' % w['damage']]
            out['weapons'][k] = w
    dst = os.path.join(REPO, 'web', 'weapon_stats.json')
    json.dump(out, open(dst, 'w', encoding='utf-8'), ensure_ascii=False, indent=1)
    for k, w in out['weapons'].items():
        print(f"{k:20} rpm={w['rpm']} mag={w['magazineSize']} cal={w['caliber']} v0={w['muzzleVelocity']} "
              f"dmg={w['damage']} kg={w['weight']} zero={(w['zeroing'] or {}).get('max')} rl={w['reloadTime']}")


if __name__ == '__main__':
    main()
