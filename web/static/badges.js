// P2-G combat badge slot in the barracks (web/badges.go). Uses the barracks globals `cur` and `api`.
(function () {
  let shown = null;
  async function render() {
    if (typeof cur === 'undefined' || !cur) return;
    if (shown === cur.id && document.getElementById('bdgpanel')) return;
    shown = cur.id;
    let d; try { d = await api(`/soldiers/${cur.id}/badges`); } catch (e) { return; }
    let p = document.getElementById('bdgpanel');
    if (!p) { p = document.createElement('div'); p.id = 'bdgpanel'; p.className = 'panel'; (document.getElementById('lmsg')?.closest('.panel') || document.querySelector('main')).after(p); }
    const lv = ['locked', 'bronze', 'silver', 'gold'];
    p.innerHTML = '<h3>Combat badge</h3><p class="muted">One badge slot. Badges unlock with their combat ribbon; the level follows the ribbon tier.</p>' +
      d.badges.map(b => `<label class="list-item" style="display:block"><input type="radio" name="bdg" value="${b.key}" ${b.level ? '' : 'disabled'} ${d.worn === b.key ? 'checked' : ''}> <b>${b.name}</b> (${lv[b.level]}) — ${b.effect} <span class="muted">[ribbon: ${b.ribbon}]</span></label>`).join('') +
      `<label class="list-item" style="display:block"><input type="radio" name="bdg" value="" ${d.worn ? '' : 'checked'}> None</label><p id="bdgmsg"></p>`;
    p.querySelectorAll('input[name=bdg]').forEach(r => r.onchange = async () => {
      try { await api(`/soldiers/${cur.id}/badge`, { method: 'PUT', body: { badge: r.value } }); bdgmsg.className = 'ok'; bdgmsg.textContent = 'Badge saved'; }
      catch (e) { bdgmsg.className = 'err'; bdgmsg.textContent = e.message; }
    });
  }
  setInterval(render, 1000);
})();
