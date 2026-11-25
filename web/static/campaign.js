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
