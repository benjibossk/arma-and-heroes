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
  const nav = [['index.html', 'War map'], ['battles.html', 'Battles'], ['link.html', 'Link game'], ['profile.html', 'Profile & ranking'], ['admin.html', 'Admin']];
  document.body.insertAdjacentHTML('afterbegin', `<header><h1>ARMA &amp; HEROES</h1><nav>${nav.map(([h, l]) =>
    `<a href="/${h}" class="${h === active ? 'on' : ''}" ${h === 'admin.html' ? 'id="nav-admin" style="display:none"' : ''}>${l}</a>`).join('')}</nav><div id="who"></div></header>`);
  try { const m = await api('/auth/me'); ME = m.user ? { ...m.user, rank: m.rank, isGeneral: m.isGeneral, linked: m.linked } : null; } catch (e) { ME = null; }
  const who = document.getElementById('who');
  if (ME) {
    who.innerHTML = `${esc(ME.rank)} <b>${esc(ME.name)}</b> ${ME.faction ? `<span class="${ME.faction}">[${ME.faction}]</span>` : ''} · ${esc(ME.role)} · <a href="#" id="logout">log out</a>`;
    document.getElementById('logout').onclick = async e => { e.preventDefault(); await api('/auth/logout', { method: 'POST' }); location.reload(); };
    if (ME.role === 'admin') document.getElementById('nav-admin').style.display = '';
  } else who.innerHTML = `<a href="/login.html?next=${encodeURIComponent(location.pathname + location.search)}">Log in with Discord</a>`;
  return ME;
}
