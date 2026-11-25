"""Flat pictogram silhouettes from our own transparent renders (alpha mask -> potrace -> SVG).
Usage: python tools/icon-silhouettes/make_icons.py   (needs opencv-python-headless, potracer, pillow)"""
import json, re, os
import numpy as np, cv2, potrace
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.join(os.path.dirname(__file__), '..', '..')
ITEMS = os.path.join(ROOT, 'web/static/assets/items'); OUT = os.path.join(ROOT, 'web/static/assets/icons')
SHEET = os.path.join(ROOT, 'docs/screenshots/icons.png')
SIDE = os.path.join(os.path.dirname(__file__), 'side')  # vehicle side-view renders (make_jobs.py --side): profiles read better as pictograms
os.makedirs(OUT, exist_ok=True)

RULES = [  # (regex on key, category) — first match wins
    (r'WIRBELWIND|FLAKVIERLING_TRUCK|FLAK_TRUCK', 'aa'), (r'FLAK', 'aa'), (r'PAK', 'atgun'), (r'LEFH|LEIG', 'artillery'),
    (r'TIGER|PZ4|JAGDPANZER|SHERMAN', 'tank'), (r'SDKFZ251|M3A1', 'halftrack'), (r'SDKFZ22', 'armoredcar'),
    (r'KFZ|JEEP', 'jeep'), (r'TRUCK|WC63', 'truck'), (r'MG34|MG42|_BAR$|M1919', 'mg'), (r'SCOPE|M1903A4', 'sniper'),
    (r'GARAND|CARBINE|KAR98K|M1903A3|STG44|FG42|G43$', 'rifle'), (r'MP40|MP38|THOMPSON|GREASE', 'smg'), (r'BAZOOKA|PANZERFAUST|PANZERSCHRECK', 'at'),
    (r'M1911|P38|LP34|FLAREGUN', 'pistol'), (r'GRENADE|SMOKE|GAMMON', 'grenade'), (r'MEDKIT', 'medic'), (r'TNT|MINES|GEBALLTE', 'explosive'),
    (r'BINOCULARS', 'binoculars'), (r'_Para$|OUTFIT_PARA|OUTFIT_FJ', 'para'), (r'ArmorCrew|OUTFIT_CREW|OUTFIT_PANZER', 'armor'),
    (r'_Recon$|NETTING|SMOCK', 'recon'), (r'_Infantry$|OUTFIT_M4', 'infantry')]
BEST = {'tank': 'GER_TIGER1', 'halftrack': 'GER_SDKFZ251', 'armoredcar': 'GER_SDKFZ222', 'jeep': 'US_JEEP_WILLYS', 'truck': 'GER_TRUCK',
        'aa': 'GER_FLAK38', 'atgun': 'GER_PAK40', 'artillery': 'GER_LEFH18', 'rifle': 'US_M1_GARAND', 'smg': 'GER_MP40', 'mg': 'GER_MG42',
        'sniper': 'GER_KAR98K_SCOPE', 'at': 'US_BAZOOKA', 'pistol': 'US_M1911', 'grenade': 'GER_M24_GRENADE', 'medic': 'US_MEDKIT',
        'explosive': 'US_TNT', 'binoculars': 'GER_BINOCULARS', 'infantry': 'AI_US_Regular_Infantry', 'recon': 'AI_US_Regular_Recon',
        'para': 'AI_US_Regular_Para', 'armor': 'AI_US_Regular_ArmorCrew'}
ALIAS = {'gun': 'artillery'}  # cat-gun.svg = artillery piece
S = 100  # viewBox size

def category(k):
    for rx, c in RULES:
        if re.search(rx, k): return c
    return None

def mask(path):
    a = np.array(Image.open(path).convert('RGBA'))[:, :, 3]
    m = (a > 60).astype(np.uint8)
    ys, xs = np.nonzero(m); m = m[ys.min():ys.max() + 1, xs.min():xs.max() + 1]
    f = 400 / max(m.shape); m = cv2.resize(m * 255, None, fx=f, fy=f, interpolation=cv2.INTER_AREA)
    m = np.pad((m > 127).astype(np.uint8), 12)
    m = cv2.morphologyEx(m, cv2.MORPH_CLOSE, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (9, 9)))
    m = cv2.morphologyEx(m, cv2.MORPH_OPEN, cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (5, 5)))
    n, lab, st, _ = cv2.connectedComponentsWithStats(m)  # drop floaters
    big = st[1:, cv2.CC_STAT_AREA].max()
    m = np.isin(lab, [i for i in range(1, n) if st[i, cv2.CC_STAT_AREA] > 0.03 * big]).astype(np.uint8)
    cs, _ = cv2.findContours(m, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)  # fill holes -> single flat fill
    m = np.zeros_like(m); cv2.drawContours(m, cs, -1, 1, cv2.FILLED)
    return m

def trace(m):
    h, w = m.shape; sc = (S - 8) / max(w, h); ox, oy = (S - w * sc) / 2, (S - h * sc) / 2
    P = lambda p: (round(ox + p.x * sc, 2), round(oy + p.y * sc, 2))
    d, polys = [], []
    for c in potrace.Bitmap(m == 0).trace(turdsize=40, alphamax=1.1, opticurve=True, opttolerance=0.4):
        cur = P(c.start_point); d.append('M%g %g' % cur); poly = [cur]
        for s in c.segments:
            if s.is_corner:
                a, b = P(s.c), P(s.end_point); d.append('L%g %g %g %g' % (*a, *b)); poly += [a, b]
            else:
                c1, c2, e = P(s.c1), P(s.c2), P(s.end_point); d.append('C%g %g %g %g %g %g' % (*c1, *c2, *e))
                for t in np.linspace(0.1, 1, 10):
                    poly.append(tuple((1-t)**3*np.array(cur) + 3*(1-t)**2*t*np.array(c1) + 3*(1-t)*t*t*np.array(c2) + t**3*np.array(e)))
            cur = P(s.end_point)
        d.append('Z'); polys.append(poly)
    return ''.join(d), polys

def write(name, d):
    open(os.path.join(OUT, name), 'w').write(
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {S} {S}"><path fill="currentColor" fill-rule="evenodd" d="{d}"/></svg>\n')

man = json.load(open(os.path.join(ITEMS, 'manifest.json')))
polys = {}
for k, v in man.items():
    if v.get('kind') != 'render': v.pop('icon', None); continue
    side = os.path.join(SIDE, k + '.webp')
    d, polys[k] = trace(mask(side if os.path.exists(side) else os.path.join(ITEMS, v['img']))); write(k + '.svg', d); v['icon'] = k + '.svg'
    c = category(k)
    if c: v['cat'] = c
for k, v in man.items():  # non-render items get their category icon
    c = category(k)
    if v.get('kind') != 'render' and c: v['cat'] = c; v['icon'] = 'cat-' + c + '.svg'
cats = {}
for c, k in BEST.items():
    src = open(os.path.join(OUT, k + '.svg')).read(); cats[c] = k
    open(os.path.join(OUT, 'cat-' + c + '.svg'), 'w').write(src)
for a, c in ALIAS.items(): open(os.path.join(OUT, 'cat-' + a + '.svg'), 'w').write(open(os.path.join(OUT, 'cat-' + c + '.svg')).read())
json.dump(man, open(os.path.join(ITEMS, 'manifest.json'), 'w'), indent=1)

# contact sheet: rasterise the traced paths themselves (what the SVG shows)
def draw(img, poly_list, x, y, sz, col):
    lay = Image.new('L', (S * 4, S * 4)); dr = ImageDraw.Draw(lay)
    for p in poly_list: dr.polygon([(a * 4, b * 4) for a, b in p], fill=255)  # outer shapes (holes are filled already)
    lay = lay.resize((sz, sz), Image.LANCZOS); img.paste(Image.new('RGB', (sz, sz), col), (x, y), lay)
keys = list(polys); cols, cell, sz = 12, 120, 84
rows = -(-len(keys) // cols) + 2
img = Image.new('RGB', (cols * cell + 8, rows * cell + 8), '#3A3C3A'); dr = ImageDraw.Draw(img)
font = ImageFont.load_default()
def cellat(i, j, k, col, label):
    x, y = 4 + i * cell, 4 + j * cell
    dr.rectangle([x + 3, y + 3, x + cell - 3, y + cell - 3], fill='#2E3031')
    draw(img, polys[k], x + (cell - sz) // 2, y + 6, sz, col); dr.text((x + 6, y + cell - 18), label[:19], fill='#9a978f', font=font)
for n, k in enumerate(keys): cellat(n % cols, n // cols, k, '#E8E6E1', k)
cl = list(BEST.items())
for n, (c, k) in enumerate(cl[:cols * 2]):
    col = ['#7FA8E0', '#E58C80'][n % 2]
    cellat(n % cols, rows - 2 + n // cols, k, col, 'cat-' + c)
img.save(SHEET); print(len(keys), 'icons,', len(BEST) + len(ALIAS), 'categories')
