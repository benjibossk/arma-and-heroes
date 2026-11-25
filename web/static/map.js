// Arma & Heroes — interactive war map
let M = null, selCity = null, selTeam = null;
const COL = { US: '#3B6FB6', GER: '#B3302B', '': '#888' };
const svgNS = 'http://www.w3.org/2000/svg';
const cityById = id => M.cities.find(c => c.id === id);
const neighbors = id => M.roads.filter(r => r.cityA === id || r.cityB === id).map(r => ({ id: r.cityA === id ? r.cityB : r.cityA, km: r.distance }));
const myFaction = () => ME && ME.role === 'admin' && selTeam ? selTeam.faction : (ME && ME.faction);
const canOrder = t => ME && ME.isGeneral && (ME.role === 'admin' || (t.faction === ME.faction && (t.ownerId === ME.id || t.ownerId === 0)));

function el(tag, attrs, parent) { const e = document.createElementNS(svgNS, tag); for (const k in attrs) e.setAttribute(k, attrs[k]); if (parent) parent.appendChild(e); return e; }

function teamIcon(g, t, x, y) {
  const c = COL[t.faction];
  el('rect', { x: x - 13, y: y - 9, width: 26, height: 18, fill: c }, g);
  // simplified NATO symbology
  const s = { stroke: '#fff', 'stroke-width': 1.6, fill: 'none' };
  if (t.type === 'INF' || t.type === 'PARA') { el('line', { x1: x - 13, y1: y - 9, x2: x + 13, y2: y + 9, ...s }, g); el('line', { x1: x + 13, y1: y - 9, x2: x - 13, y2: y + 9, ...s }, g); }
  if (t.type === 'PARA') el('path', { d: `M${x - 6} ${y + 6} q6 -8 12 0`, ...s }, g);
  if (t.type === 'ARM') el('ellipse', { cx: x, cy: y, rx: 9, ry: 5, ...s }, g);
  if (t.type === 'REC') el('line', { x1: x - 13, y1: y + 9, x2: x + 13, y2: y - 9, ...s }, g);
  if (t.type === 'MOT') { el('line', { x1: x - 13, y1: y - 9, x2: x + 13, y2: y + 9, ...s }, g); el('line', { x1: x + 13, y1: y - 9, x2: x - 13, y2: y + 9, ...s }, g); el('line', { x1: x, y1: y - 9, x2: x, y2: y + 9, ...s }, g); }
  const tx = el('text', { x: x, y: y + 21, 'text-anchor': 'middle', fill: '#E8E6E1', style: 'fill:#E8E6E1;font-size:10px' }, g); tx.textContent = t.manpower;
}

function draw() {
  const s = document.getElementById('map'); s.innerHTML = '';
  const defs = el('defs', {}, s);
  ['US', 'GER'].forEach(f => { const m = el('marker', { id: 'arrow' + f, viewBox: '0 0 10 10', refX: 8, refY: 5, markerWidth: 6, markerHeight: 6, orient: 'auto' }, defs); el('path', { d: 'M0 0L10 5L0 10z', fill: COL[f] }, m); });
  el('rect', { x: 0, y: 0, width: 1000, height: 680, fill: '#2a2c2b' }, s);
  for (let i = 0; i < 1000; i += 50) el('line', { x1: i, y1: 0, x2: i, y2: 680, stroke: '#373938', 'stroke-width': .5 }, s);
  for (let j = 0; j < 680; j += 50) el('line', { x1: 0, y1: j, x2: 1000, y2: j, stroke: '#373938', 'stroke-width': .5 }, s);
  const targets = selTeam && selTeam.status === 'idle' ? neighbors(selTeam.cityId).map(n => n.id) : [];
  M.roads.forEach(r => {
    const a = cityById(r.cityA), b = cityById(r.cityB); if (!a || !b) return;
    const tg = selTeam && ((r.cityA === selTeam.cityId && targets.includes(r.cityB)) || (r.cityB === selTeam.cityId && targets.includes(r.cityA)));
    el('line', { x1: a.x, y1: a.y, x2: b.x, y2: b.y, class: 'road' + (tg ? ' target' : '') }, s);
    const t = el('text', { x: (a.x + b.x) / 2, y: (a.y + b.y) / 2 - 4, 'font-size': 10, fill: '#B9B8B2', 'text-anchor': 'middle' }, s); t.textContent = r.distance + ' km';
  });
  // movement arrows
  M.teams.filter(t => t.status === 'moving' && t.destCityId).forEach(t => {
    const a = cityById(t.cityId), b = cityById(t.destCityId); if (!a || !b) return;
    const dx = b.x - a.x, dy = b.y - a.y, L = Math.hypot(dx, dy) || 1;
    el('line', { x1: a.x + dx / L * 22, y1: a.y + dy / L * 22, x2: b.x - dx / L * 26, y2: b.y - dy / L * 26, stroke: COL[t.faction], class: 'move', 'marker-end': `url(#arrow${t.faction})` }, s);
  });
  M.cities.forEach(c => {
    const g = el('g', { class: 'city' + (selCity === c.id ? ' sel' : '') }, s);
    if (M.battles.some(b => b.cityId === c.id)) el('circle', { cx: c.x, cy: c.y, r: 24, class: 'battle-ring' }, g);
    el('circle', { cx: c.x, cy: c.y, r: c.isCapital ? 16 : 12, fill: COL[c.owner] }, g);
    const sym = (c.isCapital ? '★' : '') + (c.isFactory ? '⚙' : '');
    if (sym) { const t = el('text', { x: c.x, y: c.y + 4, 'text-anchor': 'middle', style: 'fill:#fff;stroke:none;font-size:11px' }, g); t.textContent = sym; }
    const t = el('text', { x: c.x, y: c.y - 20, 'text-anchor': 'middle' }, g); t.textContent = c.name;
    g.onclick = () => clickCity(c);
  });
  // teams grouped by city (moving: along the road)
  const slots = {};
  M.teams.forEach(t => {
    let a = cityById(t.cityId); if (!a) return;
    let x = a.x, y = a.y, key = t.cityId;
    if (t.status === 'moving' && t.destCityId && cityById(t.destCityId) && t.departAt) {
      const b = cityById(t.destCityId), p = Math.min(1, Math.max(0, (Date.now() - new Date(t.departAt)) / (new Date(t.arrivalAt) - new Date(t.departAt))));
      x = a.x + (b.x - a.x) * p; y = a.y + (b.y - a.y) * p; key = 'm' + t.teamId;
    } else if (t.status === 'in_battle') { const bc = M.battles.find(b => b.battleId === t.battleId); if (bc && cityById(bc.cityId)) { const b = cityById(bc.cityId); x = (a.x + b.x) / 2; y = (a.y + b.y) / 2; if (bc.cityId === t.cityId) { x = b.x; y = b.y; } key = 'b' + bc.cityId + t.faction; } }
    const n = slots[key] = (slots[key] || 0) + 1;
    const g = el('g', { class: 'team' + (selTeam && selTeam.teamId === t.teamId ? ' sel' : '') }, s);
    teamIcon(g, t, x + 28 + (n - 1) * 30, y + 14);
    const ti = el('title', {}, g); ti.textContent = `${t.teamId} ${TLABEL[t.type]} — ${t.manpower} men`;
    g.onclick = ev => { ev.stopPropagation(); clickTeam(t); };
  });
}

function clickTeam(t) { selTeam = selTeam && selTeam.teamId === t.teamId ? null : t; selCity = null; showSel(); draw(); }

async function clickCity(c) {
  if (selTeam && selTeam.status === 'idle' && canOrder(selTeam) && neighbors(selTeam.cityId).some(n => n.id === c.id)) {
    if (confirm(`Order ${selTeam.teamId} to march on ${c.name}?`)) {
      try { await api(`/teams/${selTeam.teamId.slice(3)}/move`, { method: 'POST', body: { destCityId: c.id } }); selTeam = null; await load(); } catch (e) { alert(e.message); }
    }
    return;
  }
  selCity = c.id; selTeam = null; showSel(); draw();
}

function showSel() {
  const d = document.getElementById('sel');
  if (selTeam) {
    const t = selTeam, c = cityById(t.cityId);
    d.innerHTML = `<b class="${t.faction}">${t.teamId}</b> — ${TLABEL[t.type]} · lvl ${t.equipLevel}<br>Strength: <b>${t.manpower}</b> · Vehicles: ${(t.vehicles || []).map(v => v.count + '× ' + v.key).join(', ') || 'none'}<br>
      Position: ${esc(c ? c.name : t.cityId)} · ${t.status === 'moving' ? `en route to <b>${esc(cityById(t.destCityId)?.name || '?')}</b> (arrives in ${fmtLeft(t.arrivalAt)})` : t.status === 'in_battle' ? `<a href="/battles.html#${t.battleId}">in combat (${t.battleId})</a>` : 'idle'}<br>
      ${t.ownerName ? 'General: ' + esc(t.ownerName) : ''}
      ${canOrder(t) && t.status === 'idle' ? `<p class="muted">Tap an adjacent city (highlighted road) to order the move.</p><button class="danger" onclick="dissolve('${t.teamId}')">Disband (50% refund)</button>` : ''}`;
  } else if (selCity) {
    const c = cityById(selCity), b = M.battles.find(x => x.cityId === c.id);
    d.innerHTML = `<b>${esc(c.name)}</b> <span class="tag ${c.owner}">${FNAME[c.owner] || '?'}</span> ${c.isCapital ? '<span class="tag">capital</span>' : ''} ${c.isFactory ? '<span class="tag">factory</span>' : ''}<br>
     Terrain: ${esc(c.terrain)} · mode ${esc(c.mode)} · garrison ${c.garrison}<br><span class="muted">Scenario: ${esc(c.scenarioId)}</span><br>
     Neighbours: ${neighbors(c.id).map(n => esc(cityById(n.id)?.name) + ' (' + n.km + ' km)').join(', ')}<br>
     ${b ? `<b style="color:#F0A830">Battle ${b.battleId} — ${STLABEL[b.status]}</b> <a href="/battles.html#${b.battleId}">view</a>` : ''}`;
  } else d.innerHTML = 'Tap a city or a team.';
}

async function dissolve(id) { if (!confirm('Disband ' + id + ' ?')) return; try { await api('/teams/' + id.slice(3), { method: 'DELETE' }); selTeam = null; await load(); } catch (e) { alert(e.message); } }

function renderPanels() {
  document.getElementById('wartitle').textContent = M.war.name + (M.war.status !== 'active' ? ` — OVER (winner ${M.war.winner})` : '');
  document.getElementById('res').innerHTML = ['US', 'GER'].map(f => { const x = M.factions[f] || {}; const n = M.cities.filter(c => c.owner === f).length;
    return `<div class="row"><b class="${f}" style="width:110px">${FNAME[f]}</b><span class="res">${x.resources ?? '?'}</span><span class="muted">+${M.income[f] || 0}/tick · ${n} cities</span></div>`; }).join('') +
    `<div class="muted">Next supply: ${fmtDate(M.nextIncomeAt)}</div>`;
  const gp = document.getElementById('genpanel');
  if (!(ME && ME.isGeneral)) { gp.style.display = 'none'; return; }
  gp.style.display = '';
  const facs = ME.role === 'admin' ? ['US', 'GER'] : [ME.faction];
  const buyCities = M.cities.filter(c => facs.includes(c.owner) && (c.isFactory || c.isCapital));
  document.getElementById('buy').innerHTML = `<div class="row"><select id="btype">${Object.entries(M.catalog).map(([k, s]) => `<option value="${k}">${s.Label} — ${s.Cost} res. (${s.Manpower} men)</option>`).join('')}</select></div>
   <div class="row"><select id="bcity">${buyCities.map(c => `<option value="${c.id}">${esc(c.name)} [${c.owner}]</option>`).join('')}</select><button onclick="buy()">Raise team</button></div>`;
  const mine = M.teams.filter(t => canOrder(t));
  document.getElementById('myteams').innerHTML = '<table><tr><th>AT</th><th>Type</th><th>Men</th><th>Position</th></tr>' + mine.map(t =>
    `<tr style="cursor:pointer" onclick="clickTeam(M.teams.find(x=>x.teamId==='${t.teamId}'))"><td class="${t.faction}">${t.teamId}</td><td>${t.type}</td><td>${t.manpower}</td><td>${esc(cityById(t.cityId)?.name)}${t.status === 'moving' ? ' → ' + esc(cityById(t.destCityId)?.name) : t.status === 'in_battle' ? ' ⚔' : ''}</td></tr>`).join('') + '</table>';
}

async function buy() {
  const city = cityById(document.getElementById('bcity').value);
  try { await api('/teams', { method: 'POST', body: { type: document.getElementById('btype').value, cityId: city.id, faction: city.owner } }); await load(); document.getElementById('gerr').textContent = ''; }
  catch (e) { document.getElementById('gerr').textContent = e.message; }
}

async function loadEvents() {
  const ev = await api('/events?limit=30');
  document.getElementById('events').innerHTML = ev.map(e => `<div class="list-item"><span class="muted">${fmtDate(e.createdAt)}</span> <span class="${e.faction}">${esc(e.message)}</span></div>`).join('') || 'Nothing to report.';
}

async function load() {
  M = await api('/map');
  if (selTeam) selTeam = M.teams.find(t => t.teamId === selTeam.teamId) || null;
  renderPanels(); draw(); showSel(); loadEvents();
}

initPage('index.html').then(() => { load(); setInterval(load, 15000); setInterval(() => { if (M) { draw(); if (selTeam) showSel(); } }, 2000); });
document.getElementById('map').addEventListener('click', e => { if (e.target.tagName === 'rect' && e.target.getAttribute('width') === '1000') { selTeam = null; selCity = null; showSel(); draw(); } });
