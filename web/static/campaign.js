// campaign.js — GAP_PLAN P2/P3 widgets (team doctrines; later: economy, rewards)
async function showUpgrades(teamId, canBuy) {
  const box = document.getElementById('upg'); if (!box) return;
  try {
    const d = await api('/teams/' + teamId + '/upgrades');
    box.innerHTML = `<p class="muted" style="margin:6px 0">Command points: <b>${d.cpUsed}/${d.cpBudget}</b> (team level + commander rank / 3). Tier-2 doctrines of a branch exclude each other.</p>` +
      d.upgrades.map(x => { const u = x.upgrade;
        return `<div class="card" style="padding:6px 8px;margin:4px 0;border-left:3px solid ${x.owned ? '#7BC67B' : x.blocked ? 'var(--line)' : 'var(--amber)'}">
          <b>T${u.tier} · ${esc(u.label)}</b> <span class="muted">${u.cp} CP · ${u.cost} war funds</span><br><span class="muted">${esc(u.desc)}</span>
          ${x.owned ? ' — <b style="color:#7BC67B">adopted</b>' : x.blocked ? ` — <span class="muted">${esc(x.blocked)}</span>` : canBuy ? `<br><button onclick="buyUpgrade('${teamId}','${u.key}',${u.cost})">Adopt</button>` : ''}</div>`; }).join('');
  } catch (e) { box.textContent = e.message; }
}
async function buyUpgrade(teamId, key, cost) {
  if (!confirm('Adopt ' + key + ' for ' + cost + ' war funds ?')) return;
  try { await api('/teams/' + teamId + '/upgrades/' + key, { method: 'POST' }); showUpgrades(teamId, true); } catch (e) { alert(e.message); }
}

// ---- barracks: service record (P3 career, rewards, cosmetics, weapon upgrades) ----
const SVC = {};
function svcSection(id, html) { SVC[id] = html; const b = document.getElementById('svc'); if (b) b.innerHTML = Object.values(SVC).join(''); }
async function svcCareer() {
  try {
    const d = await api('/me/career');
    const opts = Object.values(d.careers).map(c => `<option value="${c.key}" ${c.key === d.career ? 'selected' : ''}>${esc(c.label)} — ${esc(c.desc)}</option>`).join('');
    const cur = d.career ? `<b>${esc(d.careers[d.career].label)}</b> <span class="muted">(change possible after ${new Date(d.lockedUntil).toLocaleDateString()}, ${d.switchPrice} credits)</span>` : '<span class="muted">none yet — first pick is free</span>';
    svcSection('career', `<h3 style="margin:4px 0">Career</h3><p style="margin:2px 0">${cur}</p><p class="muted" style="margin:2px 0">Your career decides which specialist classes you can enlist and which specialist seats you may take in battle. Infantry is always open.</p>
      <select id="carsel">${opts}</select> <button onclick="svcSetCareer()">Confirm career</button>`);
  } catch (e) { svcSection('career', ''); }
}
async function svcSetCareer() {
  const v = document.getElementById('carsel').value;
  if (!confirm('Choose career ' + v + ' ? It is locked for a week.')) return;
  try { await api('/me/career', { method: 'POST', body: { career: v } }); svcCareer(); } catch (e) { alert(e.message); }
}
if (document.getElementById('svc')) addEventListener('load', () => { svcCareer(); });
