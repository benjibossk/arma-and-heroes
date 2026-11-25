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
  return `<div class="icard ${i.locked ? 'locked' : ''} ${i.owned ? 'owned' : ''}">${itemPic(i.key)}
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
