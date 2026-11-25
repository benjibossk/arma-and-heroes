// Arma & Heroes — shared helpers
const API = '/api/v1';
async function api(path, opts = {}) {
  const o = { credentials: 'same-origin', headers: {}, ...opts };
  if (o.body && typeof o.body !== 'string') { o.body = JSON.stringify(o.body); o.headers['Content-Type'] = 'application/json'; }
  const r = await fetch(API + path, o);
  const j = await r.json().catch(() => ({}));
  if (!r.ok) throw new Error(j.error || ('HTTP ' + r.status));
  return j;
}
const esc = s => String(s ?? '').replace(/[&<>"']/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;', "'": '&#39;' }[c]));
const FNAME = { US: 'Allies (US)', GER: 'Axis (GER)' };
const TLABEL = { INF: 'Infantry', PARA: 'Paratroopers', ARM: 'Armor', REC: 'Recon', MOT: 'Motorized' };
const STLABEL = { pending: 'Waiting for server', starting: 'Starting', running: 'In progress', finished: 'Finished', cancelled: 'Cancelled' };
const fmtDate = d => d ? new Date(d).toLocaleString('en-GB', { dateStyle: 'short', timeStyle: 'short' }) : '—';
function fmtLeft(d) { const s = Math.max(0, Math.round((new Date(d) - Date.now()) / 1000)); return Math.floor(s / 60) + ' min ' + String(s % 60).padStart(2, '0') + ' s'; }
const discordLogin = next => '/api/v1/auth/discord?next=' + encodeURIComponent(next || (location.pathname + location.search));
let ME = null;
async function initPage(active) {
  const tabs = [['barracks.html', 'Barracks'], ['index.html', 'War map'], ['profile.html', 'Progression'], ['unlocks.html', 'Unlocks'], ['quartermaster', 'Quartermaster']];
  const sub = [['battles.html', 'Battles'], ['link.html', 'Link game'], ['admin.html', 'Admin']];
  const href = h => h === 'quartermaster' ? '/barracks.html#depot' : '/' + h;
  document.body.insertAdjacentHTML('afterbegin', `<header><div class="logo">A&amp;H</div><nav>${tabs.map(([h, l]) =>
    `<a href="${href(h)}" class="${h === active ? 'on' : ''}">${l}</a>`).join('')}</nav><div class="subnav">${sub.map(([h, l]) =>
    `<a href="/${h}" class="${h === active ? 'on' : ''}" ${h === 'admin.html' ? 'id="nav-admin" style="display:none"' : ''}>${l}</a>`).join('')}<span id="who"></span></div></header>`);
  try { const m = await api('/auth/me'); ME = m.user ? { ...m.user, rank: m.rank, isGeneral: m.isGeneral, linked: m.linked } : null; } catch (e) { ME = null; }
  const who = document.getElementById('who');
  if (ME) {
    who.innerHTML = `${esc(ME.rank)} <b>${esc(ME.name)}</b> ${ME.faction ? `<span class="${ME.faction}">[${ME.faction}]</span>` : ''} · ${esc(ME.role)} · <a href="#" id="logout">log out</a>`;
    document.getElementById('logout').onclick = async e => { e.preventDefault(); await api('/auth/logout', { method: 'POST' }); location.reload(); };
    if (ME.role === 'admin') document.getElementById('nav-admin').style.display = '';
  } else who.innerHTML = `<a href="/login.html?next=${encodeURIComponent(location.pathname + location.search)}">Log in with Discord</a>`;
  return ME;
}
// Item pictures (assets/items/manifest.json): real game previews where a mod ships one, Style B silhouettes otherwise
let ITEMIMG = {};
const itemImgsReady = fetch('/assets/items/manifest.json').then(r => r.json()).then(j => { ITEMIMG = j; }).catch(() => {});
const itemSrc = k => '/assets/items/' + (ITEMIMG[k]?.img || '_sil_rifle.svg');
const itemPic = (k, cls = '') => `<div class="ipic ${ITEMIMG[k]?.kind === 'photo' ? 'photo' : ''} ${cls}"><img src="${itemSrc(k)}" alt="" loading="lazy"></div>`;
function itemCard(i, action = '') {
  return `<div class="icard ${i.locked ? 'locked' : ''} ${i.owned ? 'owned' : ''}" data-k="${esc(i.key)}">${itemPic(i.key)}
  <div class="ibody"><div class="iname">${catBadge(i.key)}${esc(i.label)}</div><div class="imeta">${esc(i.slot)} · ${esc(i.category || '')} · ${i.weight} pt</div>
  <div class="ifoot"><span class="iprice">${i.price ? i.price + ' cr.' : 'Free'}</span>${action}</div>
  ${i.locked ? `<div class="ilock">🔒 ${esc(i.locked)}</div>` : ''}</div></div>`;
}

// ---------- ribbons (our own drawing: striped ribbon bar + tier pip) ----------
const RIB_COL = { weapon: ['#6B4A2B', '#C9B27C'], skill: ['#2F4F6B', '#C9B27C'], objective: ['#5A2E2E', '#E8E6E1'], support: ['#2F5A36', '#E8E6E1'], vehicle: ['#3E3F44', '#F0A830'], service: ['#4A3A5A', '#C9B27C'] };
const TIER_COL = ['#555', '#B87333', '#C0C0C0', '#F0A830'];
function ribbonSvg(r) {
  const [a, b] = RIB_COL[r.category] || RIB_COL.service, t = r.tier, dim = t ? '' : ' opacity=".35"';
  return `<svg class="rsvg" viewBox="0 0 64 22" width="64" height="22"${dim}><rect width="64" height="22" rx="2" fill="${a}"/><rect x="14" width="6" height="22" fill="${b}"/><rect x="44" width="6" height="22" fill="${b}"/><rect x="29" width="6" height="22" fill="${TIER_COL[t] || TIER_COL[3]}"/>${[...Array(t)].map((_, i) => `<circle cx="7" cy="${6 + i * 5}" r="2" fill="${TIER_COL[t]}"/>`).join('')}<rect width="64" height="22" rx="2" fill="none" stroke="#000" stroke-opacity=".5"/></svg>`;
}
function ribbonCard(r) {
  const top = r.tiers[r.tiers.length - 1].at, nx = r.next || top, prev = r.tier ? r.tiers[r.tier - 1].at : 0;
  const pct = r.next ? Math.min(100, Math.round((r.value - prev) * 100 / Math.max(1, nx - prev))) : 100;
  const badges = r.tiers.map(x => `<span class="rtier ${r.tier >= x.tier ? 'on t' + x.tier : ''}" title="${x.xp} XP · ${x.credits} cr.">${esc(x.name)} ${x.at}</span>`).join('');
  const unl = (r.unlocks || []).map(u => `<div class="runl ${u.unlocked ? 'ok' : ''}">${u.unlocked ? '✔' : '🔒'} ${esc(u.label)} <span class="muted">(${esc(u.tierName)})</span></div>`).join('');
  return `<div class="rcard">${ribbonSvg(r)}<div class="rbody"><div class="rname">${esc(r.name)}</div>
  <div class="rbar"><i style="width:${pct}%"></i></div><div class="rmeta">${r.value} / ${r.next || top} ${r.next ? '' : '· max'}</div>
  <div class="rtiers">${badges}</div>${unl}</div></div>`;
}

// ---------- flat silhouette icons (assets/icons, traced from our renders; fill=currentColor) ----------
const CAT_RX = [[/TIGER|PZ|PANZER4|SHERMAN|TANK/, 'tank'], [/SDKFZ251|M3A1|HALFTRACK/, 'halftrack'], [/SDKFZ22|ARMORED/, 'armoredcar'], [/KFZ|JEEP/, 'jeep'],
  [/TRUCK|WC63|GMC/, 'truck'], [/FLAK/, 'aa'], [/PAK|ATGUN/, 'atgun'], [/LEFH|LEIG|HOWITZER/, 'artillery']];
const itemOf = k => ITEMIMG[k] || ITEMIMG[Object.keys(ITEMIMG).find(x => x.startsWith(k + '_')) || ''];
const catOf = k => itemOf(k)?.cat || (CAT_RX.find(([r]) => r.test(k)) || [])[1];
const iconFile = k => itemOf(k)?.icon || (catOf(k) ? 'cat-' + catOf(k) + '.svg' : 'cat-tank.svg');
const renderOf = k => itemOf(k)?.kind === 'render' ? '/assets/items/' + itemOf(k).img : '';
// HTML: tinted by CSS color (mask-image)
const icoHtml = (file, cls = '') => `<i class="ico ${cls}" style="--ico:url(/assets/icons/${file})"></i>`;
const catBadge = k => catOf(k) ? icoHtml('cat-' + catOf(k) + '.svg', 'cbadge') : '';
function vehChips(vs, fac = '') {
  return (vs || []).map(v => `<span class="vchip ${fac}">${icoHtml(iconFile(v.key))}${v.count}×<span class="muted"> ${esc(v.key)}</span>${renderOf(v.key) ? `<img class="vthumb" src="${renderOf(v.key)}" alt="" loading="lazy">` : ''}</span>`).join(' ') || 'none';
}
// SVG: inline the path so it can be filled with any colour
const ICOD = {};
const loadIcons = files => Promise.all([...new Set(files)].filter(f => !(f in ICOD)).map(f => fetch('/assets/icons/' + f).then(r => r.text())
  .then(t => { ICOD[f] = (t.match(/ d="([^"]+)"/) || [])[1] || ''; }).catch(() => { ICOD[f] = ''; })));
function svgIcon(parent, file, cx, cy, size, attrs = {}) {
  const p = document.createElementNS('http://www.w3.org/2000/svg', 'path'), s = size / 100;
  p.setAttribute('d', ICOD[file] || 'M10 10H90V90H10Z'); p.setAttribute('fill-rule', 'evenodd');
  p.setAttribute('transform', `translate(${cx - size / 2} ${cy - size / 2}) scale(${s})`);
  for (const k in attrs) p.setAttribute(k, attrs[k]);
  parent.appendChild(p); return p;
}

// ---------- weapon detail card (stats extracted from the mods: /api/v1/weapons/stats) ----------
let WSTATS = null;
const weaponStats = () => WSTATS || (WSTATS = fetch('/api/v1/weapons/stats').then(r => r.json()).then(j => j.weapons || {}).catch(() => ({})));
const WBARS = [ // label, value fn, max, unit
  ['Rate of fire', w => w.rpm, 1200, 'rpm'],
  ['Reload', w => w.reloadTime, 10, 's'],
  ['Magazine', w => w.magazineSize, 100, 'rds'],
  ['Muzzle velocity', w => w.muzzleVelocity, 900, 'm/s'],
  ['Weight', w => w.weight, 15, 'kg'],
];
function wBars(a, b) {
  return WBARS.map(([l, f, mx, u]) => {
    const row = (w, cls) => { const v = w && f(w); return `<div class="wbar ${cls}"><i style="width:${v ? Math.min(100, v / mx * 100) : 0}%"></i></div>`; };
    const va = f(a), vb = b ? f(b) : undefined;
    return `<div class="wstat"><span>${l}</span><div class="wbs">${row(a, 'a')}${b ? row(b, 'b') : ''}</div><b>${va ?? '—'}${va != null ? ' ' + u : ''}${b ? ` <em>/ ${vb ?? '—'}</em>` : ''}</b></div>`;
  }).join('');
}
function wChart(a, b) {
  const cs = [a, b].filter(w => w && w.damageCurve);
  if (!cs.length) return '<p class="muted wnc">No ballistic data for this item (explosive / launcher or not defined in the mod).</p>';
  const W = 520, H = 210, L = 44, B = 26, T = 10, R = 10;
  const ymax = Math.max(...cs.map(w => w.damageCurve[0].dmg)) * 1.08 || 1;
  const X = m => L + m / 1000 * (W - L - R), Y = d => T + (1 - d / ymax) * (H - T - B);
  let g = '';
  for (let m = 0; m <= 1000; m += 200) g += `<line x1="${X(m)}" y1="${T}" x2="${X(m)}" y2="${H - B}" class="gl"/><text x="${X(m)}" y="${H - 8}" text-anchor="middle">${m} m</text>`;
  for (let k = 0; k <= 4; k++) { const d = ymax / 1.08 * k / 4; g += `<line x1="${L}" y1="${Y(d)}" x2="${W - R}" y2="${Y(d)}" class="gl"/><text x="${L - 5}" y="${Y(d) + 4}" text-anchor="end">${Math.round(d)}</text>`; }
  const path = (w, cls) => `<polyline class="${cls}" points="${w.damageCurve.map(p => X(p.m).toFixed(1) + ',' + Y(p.dmg).toFixed(1)).join(' ')}"/>`;
  return `<svg class="wchart" viewBox="0 0 ${W} ${H}" role="img" aria-label="Damage versus range">${g}
    ${a.damageCurve ? path(a, 'ca') : ''}${b && b.damageCurve ? path(b, 'cb') : ''}</svg>
    <div class="muted wnc">Damage per hit vs range (energy model from mod drag/mass, armour ignored)${cs.some(w => w.warnings) ? ' — ⚠ ' + esc(cs.find(w => w.warnings).warnings[0]) : ''}</div>`;
}
// i: item as returned by /unlocks or /store (key, label, category, price, rank, rankName, classes, tree)
async function openWeapon(i, list = []) {
  const S = await weaponStats(), a = S[i.key];
  let box = document.getElementById('wdlg');
  if (!box) { box = document.createElement('div'); box.id = 'wdlg'; box.className = 'wdlg'; document.body.appendChild(box); box.onclick = e => { if (e.target === box) box.remove(); }; }
  const others = list.filter(o => o.key !== i.key && S[o.key] && S[o.key].damageCurve);
  const draw = cmpKey => {
    const b = cmpKey ? S[cmpKey] : null, bi = list.find(o => o.key === cmpKey);
    const modes = a && a.fireModes && a.fireModes.length ? a.fireModes.map(m => m.mode).filter(Boolean).join(' / ') : '—';
    box.innerHTML = `<div class="wcard"><button class="wx" onclick="document.getElementById('wdlg').remove()">✕</button>
     <div class="whead"><div class="wimg ${ITEMIMG[i.key]?.kind === 'photo' ? 'photo' : ''}"><img src="${itemSrc(i.key)}" alt=""></div>
      <div><div class="wcat">${esc(i.category || '')}${a && a.caliber ? ' · ' + esc(a.caliber) : ''}</div><h2>${esc(i.label)}</h2>
       <div class="wtags"><span>${i.price ? nfmt(i.price) + ' cr' : 'Free'}</span><span>${i.rank ? 'Rank ' + i.rank + (i.rankName ? ' · ' + esc(i.rankName) : '') : esc(i.rankRequired || 'No rank required')}</span>
       ${(i.classes || []).map(c => `<span>${esc(c)}</span>`).join('')}${i.tree ? `<span>${esc(i.tree)}</span>` : ''}</div>
       <div class="muted" style="font-size:13px">Fire modes: ${esc(modes)}${a && a.zeroing ? ` · Sights ${a.zeroing.min}–${a.zeroing.max} m` : ''}${a && a.damage ? ` · Damage ${a.damage}` : ''}</div></div></div>
     ${a ? `<div class="wbars">${b ? `<div class="wleg"><i class="a"></i>${esc(i.label)} <i class="b"></i>${esc(bi?.label || cmpKey)}</div>` : ''}${wBars(a, b)}</div>${wChart(a, b)}` : '<p class="muted">No stats extracted for this item.</p>'}
     ${others.length && a && a.damageCurve ? `<div class="wcmp">Compare with <select onchange="this.closest('.wdlg')._draw(this.value)"><option value="">—</option>${others.map(o => `<option value="${o.key}" ${o.key === cmpKey ? 'selected' : ''}>${esc(o.label)}</option>`).join('')}</select></div>` : ''}</div>`;
  };
  box._draw = draw; draw('');
}
const nfmt = n => Number(n).toLocaleString('en-US').replace(/,/g, ' ');
// make every .icard of a container open the weapon card (items = list the cards came from)
function bindWeaponCards(root, items) {
  root.querySelectorAll('.icard[data-k]').forEach(el => {
    el.classList.add('wclick');
    el.onclick = e => { if (e.target.closest('button,select,a')) return; const i = items.find(x => x.key === el.dataset.k); if (i) openWeapon(i, items.filter(x => x.slot !== 'license')); };
  });
}
