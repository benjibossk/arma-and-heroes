// Arma & Heroes — utilitaires communs
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
const FNAME = { US: 'Alliés (US)', GER: 'Axe (GER)' };
const TLABEL = { INF: 'Infanterie', PARA: 'Parachutistes', ARM: 'Blindés', REC: 'Reconnaissance', MOT: 'Motorisée' };
const STLABEL = { pending: 'En attente de serveur', starting: 'Démarrage', running: 'En cours', finished: 'Terminée', cancelled: 'Annulée' };
const fmtDate = d => d ? new Date(d).toLocaleString('fr-FR', { dateStyle: 'short', timeStyle: 'short' }) : '—';
function fmtLeft(d) { const s = Math.max(0, Math.round((new Date(d) - Date.now()) / 1000)); return Math.floor(s / 60) + ' min ' + String(s % 60).padStart(2, '0') + ' s'; }
let ME = null;
async function initPage(active) {
  const nav = [['index.html', 'Carte de guerre'], ['battles.html', 'Batailles'], ['profile.html', 'Profil & classement'], ['admin.html', 'Admin']];
  document.body.insertAdjacentHTML('afterbegin', `<header><h1>ARMA &amp; HEROES</h1><nav>${nav.map(([h, l]) =>
    `<a href="/${h}" class="${h === active ? 'on' : ''}" ${h === 'admin.html' ? 'id="nav-admin" style="display:none"' : ''}>${l}</a>`).join('')}</nav><div id="who"></div></header>`);
  try { const m = await api('/auth/me'); ME = m.user ? { ...m.user, rank: m.rank, isGeneral: m.isGeneral } : null; } catch (e) { ME = null; }
  const who = document.getElementById('who');
  if (ME) {
    who.innerHTML = `${esc(ME.rank)} <b>${esc(ME.name)}</b> ${ME.faction ? `<span class="${ME.faction}">[${ME.faction}]</span>` : ''} · ${esc(ME.role)} · <a href="#" id="logout">déconnexion</a>`;
    document.getElementById('logout').onclick = async e => { e.preventDefault(); await api('/auth/logout', { method: 'POST' }); location.reload(); };
    if (ME.role === 'admin') document.getElementById('nav-admin').style.display = '';
  } else who.innerHTML = '<a href="/login.html">Se connecter</a>';
  return ME;
}
