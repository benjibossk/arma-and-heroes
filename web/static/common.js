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
  const tabs = [['barracks.html', 'Barracks'], ['index.html', 'War map'], ['profile.html', 'Progression'], ['quartermaster', 'Quartermaster']];
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
  <div class="ibody"><div class="iname">${esc(i.label)}</div><div class="imeta">${esc(i.slot)} · ${esc(i.category || '')} · ${i.weight} pt</div>
  <div class="ifoot"><span class="iprice">${i.price ? i.price + ' cr.' : 'Free'}</span>${action}</div>
  ${i.locked ? `<div class="ilock">🔒 ${esc(i.locked)}</div>` : ''}</div></div>`;
}
