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


LOCK = os.path.join(HERE, "orchestrator.pid")


def acquire_lock():
    """Une seule boucle a la fois : orchestrator.pid contient le PID du processus vivant."""
    if os.path.exists(LOCK):
        try:
            old = int(open(LOCK).read().strip())
            cl = cmdline(old).lower()
            if "orchestrator.py" in cl:
                log.error("orchestrateur deja actif (PID %s) : abandon", old); return False
        except (ValueError, OSError):
            pass
    with open(LOCK, "w") as f:
        f.write(str(os.getpid()))
    return True


def release_lock():
    try:
        if os.path.exists(LOCK) and open(LOCK).read().strip() == str(os.getpid()):
            os.remove(LOCK)
    except OSError:
        pass


def prune_server_logs(cfg, keep=20):
    """Rotation des logs serveur : garde les `keep` dossiers logs_* les plus recents par instance."""
    import shutil
    for inst in cfg["instances"]:
        d = os.path.join(inst["dir"], "profile", "logs")
        if not os.path.isdir(d):
            continue
        dirs = sorted((x for x in os.listdir(d) if x.startswith("logs_")), reverse=True)
        for old in dirs[keep:]:
            shutil.rmtree(os.path.join(d, old), ignore_errors=True)
            log.info("log serveur purge : %s", os.path.join(d, old))


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
    return "armareforgerserver.exe" in cl and os.path.join(inst["dir"], "profile").lower() in cl


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
    mh = {"m_sBattleId": battle["battleId"], "m_sApiUrl": api_url, "m_sToken": key}
    if cfg.get("timeLimitOverrideMin"):
        mh["m_iTimeLimitOverrideMin"] = int(cfg["timeLimitOverrideMin"])
    if cfg.get("testVehicles"):
        mh["m_sTestVehicles"] = ",".join(cfg["testVehicles"])
    g.setdefault("gameProperties", {})["missionHeader"] = mh
    return c


def cli_args(cfg, inst, conf):
    """Mode ligne de commande (-server/-addons, sans -config) : seul moyen de charger un addon
    local NON publie (F44_Core). Avec -config, le serveur interroge le Workshop pour chaque mod
    de game.mods et echoue ; -config et -addons sont incompatibles. Voir mod/README.md."""
    prof = os.path.join(inst["dir"], "profile")
    mods = list(dict.fromkeys([m["modId"] for m in cfg.get("localAddons", [])] + [m["modId"] for m in conf["game"].get("mods", [])]))
    dirs = [cfg["localAddonsDir"], os.path.join(prof, "addons")]
    return ["-addr", f"0.0.0.0:{inst['bindPort']}", "-bindPort", str(inst["bindPort"]), "-a2sPort", str(inst["a2s"]),
            "-profile", prof, "-addonsDir", ",".join(dirs), "-addons", ",".join(mods),
            "-server", conf["game"]["scenarioId"]]


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
                status = (self.api.req("GET", f"/battles/{bid}/detail") or {}).get("status")  # /battles/{id} = BattleConfig, no status
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
            if pid and status in ("starting", "running") and time.time() - run.get("hb", 0) > 300:
                self.heartbeat(bid, {"orchestrator": "alive", "instance": name})  # keeps the web director from auto-resolving
                run["hb"] = time.time()
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
        # Workshop (defaut) : -config genere par instance, F44_Core telecharge depuis le Workshop.
        # --local-addon : ancien mode CLI pour un build dev non publie (localAddonsDir).
        cli = bool(self.cfg.get("_localAddon") and self.cfg.get("localAddons"))
        if cli:
            args = [exe] + cli_args(self.cfg, inst, conf) + self.cfg.get("serverArgs", [])
        else:
            args = [exe, "-config", cpath, "-profile", prof] + self.cfg.get("serverArgs", [])
        deadline = time.time() + (int(battle.get("timeLimitMin", 45)) + self.cfg.get("graceMin", 15)) * 60
        if self.dry:
            safe = copy.deepcopy(conf); safe["game"]["gameProperties"]["missionHeader"]["m_sToken"] = "***"
            out = os.path.join(HERE, f"dryrun-{inst['name']}-config.json")
            save_json(out, safe)
            log.info("[dry] config ecrite (token masque) : %s ; lancerait : %s", out, " ".join(args))
            log.info("[dry] ecrirait %s (battleId/apiUrl/token)", os.path.join(prof, "profile", "F44", "server.json"))
            return
        if inst["name"] == self.cfg["instances"][0]["name"]:
            # ww2-1 sert de modele : on garde l'original
            bak = cpath + ".template.bak"
            if not os.path.exists(bak) and os.path.exists(cpath):
                import shutil; shutil.copy2(cpath, bak)
        # $profile: du moteur = <-profile>\profile (et non <-profile>)
        f44 = os.path.join(prof, "profile", "F44")
        os.makedirs(f44, exist_ok=True)
        save_json(cpath, conf)
        save_json(os.path.join(f44, "battle.json"), battle)
        if self.cfg.get("catalog"):
            import shutil; shutil.copy2(self.cfg["catalog"], os.path.join(f44, "catalog.json"))
        # toujours ecrit : en mode CLI pas de gameProperties, et en mode Workshop c'est un secours
        mh = conf["game"]["gameProperties"]["missionHeader"]
        save_json(os.path.join(f44, "server.json"), {"battleId": mh["m_sBattleId"], "apiUrl": mh["m_sApiUrl"],
                  "token": mh["m_sToken"], "timeLimitOverrideMin": mh.get("m_iTimeLimitOverrideMin", 0),
                  "testVehicles": mh.get("m_sTestVehicles", "")})
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
    ap.add_argument("--relaunch", metavar="BATTLE_ID", help="dev: arrete l'instance de cette bataille (PID verifie) et la relance (mod modifie)")
    ap.add_argument("--local-addon", action="store_true", help="dev: ancien mode CLI (-server/-addons) avec l'addon local non publie")
    ap.add_argument("--config", default=os.path.join(HERE, "orchestrator.json"))
    a = ap.parse_args()
    setup_log()
    cfg = load_json(a.config)
    cfg["_localAddon"] = a.local_addon
    # templateConfig : si ww2-1 a deja ete reecrit par nous, utiliser la sauvegarde
    bak = cfg["templateConfig"] + ".template.bak"
    if os.path.exists(bak):
        cfg["templateConfig"] = bak
    api = Api(a.api or os.environ.get("F44_API_URL") or cfg["apiUrl"], read_key(cfg["envFile"]))
    log.info("demarrage api=%s dry=%s once=%s mode=%s", api.base, a.dry_run, a.once, "local-addon" if a.local_addon else "workshop")
    ensure_instance_dirs(cfg, a.dry_run)
    o = Orchestrator(cfg, api, a.dry_run)
    loop = not a.once and not a.relaunch and not a.dry_run
    if loop and not acquire_lock():
        sys.exit(1)
    if a.relaunch:
        insts = {i["name"]: i for i in cfg["instances"]}
        for name, run in list(o.state.items()):
            if run["battleId"] == a.relaunch:
                if run.get("pid") and o.alive(run["pid"]):
                    stop_pid(run["pid"], insts[name])
                o.release(name, "relance")
                o.launch(insts[name], api.req("GET", f"/battles/{a.relaunch}"))
                return
        log.error("bataille %s absente de l'etat local", a.relaunch); return
    if loop:
        log.info("boucle continue (PID %s, poll %ss)", os.getpid(), cfg.get("pollSec", 15))
    last_prune = 0
    try:
        while True:
            o.tick()
            if a.once:
                break
            if time.time() - last_prune > 3600:
                last_prune = time.time()
                try:
                    prune_server_logs(cfg, cfg.get("keepServerLogs", 20))
                except Exception:
                    log.exception("purge des logs")
            time.sleep(cfg.get("pollSec", 15))
    except KeyboardInterrupt:
        log.info("arret demande (Ctrl+C)")
    finally:
        if loop:
            release_lock()


if __name__ == "__main__":
    main()
