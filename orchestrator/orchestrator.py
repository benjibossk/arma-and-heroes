"""Orchestrateur Arma & Heroes (V2) - Python 3 stdlib, Windows.
Usage : python orchestrator.py [--once] [--dry-run] [--api URL] [--setup]
"""
import argparse, copy, json, logging, logging.handlers, os, subprocess, sys, time, urllib.request, urllib.error

HERE = os.path.dirname(os.path.abspath(__file__))
STATE = os.path.join(HERE, "state.json")
log = logging.getLogger("orch")


def setup_log():
    h = logging.handlers.RotatingFileHandler(os.path.join(HERE, "orchestrator.log"), maxBytes=2_000_000, backupCount=5, encoding="utf-8")
    fmt = logging.Formatter("%(asctime)s %(levelname)s %(message)s")
    h.setFormatter(fmt); s = logging.StreamHandler(); s.setFormatter(fmt)
    log.addHandler(h); log.addHandler(s); log.setLevel(logging.INFO)


def load_json(p, default=None):
    try:
        with open(p, encoding="utf-8-sig") as f:
            return json.load(f)
    except FileNotFoundError:
        return default


def save_json(p, obj):
    tmp = p + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        json.dump(obj, f, indent=2, ensure_ascii=False)
    os.replace(tmp, p)


def read_key(env_file):
    for line in open(env_file, encoding="utf-8-sig"):
        line = line.strip()
        if line.startswith("F44_SERVER_KEY="):
            return line.split("=", 1)[1].strip().strip('"').strip("'")
    raise RuntimeError("F44_SERVER_KEY absente de " + env_file)


class Api:
    def __init__(self, base, key):
        self.base, self.key = base.rstrip("/"), key

    def req(self, method, path, body=None, auth=False):
        data = json.dumps(body).encode() if body is not None else None
        r = urllib.request.Request(self.base + "/api/v1" + path, data=data, method=method)
        r.add_header("Content-Type", "application/json")
        if auth:
            r.add_header("Authorization", "Bearer " + self.key)
        with urllib.request.urlopen(r, timeout=15) as resp:
            txt = resp.read().decode("utf-8") or "null"
            return json.loads(txt)


# ---------- process helpers (ne touche QUE nos PID) ----------
def ps(cmd):
    return subprocess.run(["powershell", "-NoProfile", "-Command", cmd], capture_output=True, text=True).stdout.strip()


def cmdline(pid):
    return ps(f"(Get-CimInstance Win32_Process -Filter 'ProcessId={int(pid)}').CommandLine")


def is_ours(pid, inst):
    cl = cmdline(pid).lower()
    return "armareforgerserver.exe" in cl and os.path.join(inst["dir"], "config.json").lower() in cl


def stop_pid(pid, inst):
    if not is_ours(pid, inst):
        log.warning("PID %s n'est pas l'instance %s (ligne de commande differente) : NON tue", pid, inst["name"])
        return
    ps(f"Stop-Process -Id {int(pid)} -Force")
    log.info("instance %s : PID %s arrete", inst["name"], pid)


# ---------- preparation des instances ----------
def ensure_instance_dirs(cfg, dry):
    base = os.path.join(cfg["instances"][0]["dir"], "profile", "addons")
    for inst in cfg["instances"][1:]:
        prof = os.path.join(inst["dir"], "profile")
        link = os.path.join(prof, "addons")
        if dry:
            log.info("[dry] creerait %s + jonction %s -> %s", prof, link, base); continue
        os.makedirs(prof, exist_ok=True)
        if not os.path.exists(link):
            subprocess.run(["cmd", "/c", "mklink", "/J", link, base], capture_output=True)
            log.info("jonction %s -> %s", link, base)


def build_config(cfg, inst, battle, api_url, key):
    c = copy.deepcopy(load_json(cfg["templateConfig"]))
    c["bindPort"] = c["publicPort"] = inst["bindPort"]
    c["a2s"]["port"] = inst["a2s"]
    c["rcon"]["port"] = inst["rcon"]
    g = c["game"]
    g["scenarioId"] = battle["scenario"]
    g["visible"] = True
    city = battle.get("cityName") or battle.get("cityId") or battle["battleId"]
    g["name"] = f"Arma & Heroes - {city}"
    g.setdefault("gameProperties", {})["missionHeader"] = {
        "m_sBattleId": battle["battleId"], "m_sApiUrl": api_url, "m_sToken": key}
    return c


# ---------- coeur ----------
class Orchestrator:
    def __init__(self, cfg, api, dry):
        self.cfg, self.api, self.dry = cfg, api, dry
        self.state = load_json(STATE, {}) or {}

    def save(self):
        if not self.dry:
            save_json(STATE, self.state)

    def alive(self, pid):
        return str(pid) in ps(f"(Get-Process -Id {int(pid)} -ErrorAction SilentlyContinue).Id")

    def release(self, name, why):
        log.info("instance %s liberee (%s)", name, why)
        self.state.pop(name, None); self.save()

    def monitor(self):
        insts = {i["name"]: i for i in self.cfg["instances"]}
        for name, run in list(self.state.items()):
            inst, bid, pid = insts.get(name), run["battleId"], run.get("pid")
            if not inst:
                self.release(name, "instance inconnue"); continue
            status = None
            try:
                status = (self.api.req("GET", f"/battles/{bid}") or {}).get("status")
            except Exception as e:
                log.warning("statut %s indisponible : %s", bid, e)
            if pid and not self.alive(pid):
                if status not in ("finished", "cancelled"):
                    log.error("CRASH instance %s bataille %s (PID %s mort sans resultat)", name, bid, pid)
                    self.heartbeat(bid, {"orchestrator": "crash", "instance": name})
                self.release(name, "process termine"); continue
            if status in ("finished", "cancelled"):
                if pid: stop_pid(pid, inst)
                self.release(name, "bataille " + status); continue
            if time.time() > run["deadline"]:
                log.warning("timeout bataille %s sur %s", bid, name)
                self.heartbeat(bid, {"orchestrator": "timeout", "instance": name})
                if pid: stop_pid(pid, inst)
                self.release(name, "timeout")

    def heartbeat(self, bid, body):
        if self.dry:
            log.info("[dry] heartbeat %s %s", bid, body); return
        try:
            self.api.req("POST", f"/battles/{bid}/heartbeat", body, auth=True)
        except Exception as e:
            log.warning("heartbeat %s echoue : %s", bid, e)

    def dispatch(self):
        free = [i for i in self.cfg["instances"] if i["name"] not in self.state]
        if not free:
            return
        pending = self.api.req("GET", "/battles?status=pending") or []
        if isinstance(pending, dict):
            pending = pending.get("battles") or pending.get("items") or []
        for b in pending:
            if not free:
                break
            bid = b.get("battleId") or b.get("id")
            inst = free.pop(0)
            if self.dry:
                battle = self.api.req("GET", f"/battles/{bid}")
                log.info("[dry] claim %s sur %s (pas de POST)", bid, inst["name"])
            else:
                battle = self.api.req("POST", f"/battles/{bid}/claim", {"serverInstance": inst["name"]}, auth=True)
            battle = battle.get("config", battle) if isinstance(battle, dict) else battle
            self.launch(inst, battle)

    def launch(self, inst, battle):
        key = self.api.key
        conf = build_config(self.cfg, inst, battle, self.api.base, key)
        prof = os.path.join(inst["dir"], "profile")
        cpath = os.path.join(inst["dir"], "config.json")
        bid = battle["battleId"]
        exe = self.cfg["serverExe"]
        args = [exe, "-config", cpath, "-profile", prof] + self.cfg.get("serverArgs", [])
        deadline = time.time() + (int(battle.get("timeLimitMin", 45)) + self.cfg.get("graceMin", 15)) * 60
        if self.dry:
            safe = copy.deepcopy(conf); safe["game"]["gameProperties"]["missionHeader"]["m_sToken"] = "***"
            out = os.path.join(HERE, f"dryrun-{inst['name']}-config.json")
            save_json(out, safe)
            log.info("[dry] config ecrite (token masque) : %s ; lancerait : %s", out, " ".join(args))
            return
        if inst["name"] == self.cfg["instances"][0]["name"]:
            # ww2-1 sert de modele : on garde l'original
            bak = cpath + ".template.bak"
            if not os.path.exists(bak) and os.path.exists(cpath):
                import shutil; shutil.copy2(cpath, bak)
        os.makedirs(os.path.join(prof, "F44"), exist_ok=True)
        save_json(cpath, conf)
        save_json(os.path.join(prof, "F44", "battle.json"), battle)
        p = subprocess.Popen(args, cwd=os.path.dirname(exe), creationflags=subprocess.CREATE_NEW_PROCESS_GROUP | subprocess.DETACHED_PROCESS)
        self.state[inst["name"]] = {"battleId": bid, "pid": p.pid, "started": time.time(), "deadline": deadline}
        self.save()
        log.info("bataille %s lancee sur %s (PID %s, port %s)", bid, inst["name"], p.pid, inst["bindPort"])

    def tick(self):
        try:
            self.monitor()
            self.dispatch()
        except urllib.error.URLError as e:
            log.warning("API injoignable : %s", e)
        except Exception:
            log.exception("erreur tick")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--once", action="store_true", help="un seul cycle")
    ap.add_argument("--dry-run", action="store_true", help="aucun claim, aucun serveur lance, aucun fichier d'instance modifie")
    ap.add_argument("--api", help="surcharge apiUrl")
    ap.add_argument("--config", default=os.path.join(HERE, "orchestrator.json"))
    a = ap.parse_args()
    setup_log()
    cfg = load_json(a.config)
    # templateConfig : si ww2-1 a deja ete reecrit par nous, utiliser la sauvegarde
    bak = cfg["templateConfig"] + ".template.bak"
    if os.path.exists(bak):
        cfg["templateConfig"] = bak
    api = Api(a.api or os.environ.get("F44_API_URL") or cfg["apiUrl"], read_key(cfg["envFile"]))
    log.info("demarrage api=%s dry=%s once=%s", api.base, a.dry_run, a.once)
    ensure_instance_dirs(cfg, a.dry_run)
    o = Orchestrator(cfg, api, a.dry_run)
    while True:
        o.tick()
        if a.once:
            break
        time.sleep(cfg.get("pollSec", 15))


if __name__ == "__main__":
    main()
