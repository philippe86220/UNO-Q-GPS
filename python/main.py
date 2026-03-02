import datetime
import threading
from arduino.app_utils import App, Bridge
from arduino.app_bricks.web_ui import WebUI

print("Python ready", flush=True)

web = WebUI()

_lock = threading.Lock()
_state = {
    "lat": None,
    "long": None,
    "jour": None,
    "mois": None,   
    "annee": None,
    "heure": None,
    "minute": None,
    "seconde": None, 
}

def now_utc_iso():
    #ISO 8601 UTC, ex: 2026-03-01T08:22:29Z
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

def update_gps(lat,long, jour, mois, annee, heure, minute,seconde):
    with _lock:
        _state["lat"] = float(lat)
        _state["long"] = float(long)
        _state["jour"] = int(jour)
        _state["mois"] = int(mois)
        _state["annee"] = int(annee)
        _state["heure"] = int(heure)
        _state["minute"] = int(minute)
        _state["seconde"] = int(seconde)

#def presence_mm(mm):
   # with _lock:
       # _state["last_presence_utc"] = now_utc_iso()
       # _state["last_presence_mm"] = int(mm)
    #return True

Bridge.provide("update_gps", update_gps)
#Bridge.provide("presence_mm", presence_mm)

def api_state(_req=None):
    with _lock:
        payload = {
            "now_utc": now_utc_iso(),
            "lat": _state["lat"],
            "long": _state["long"],
            "jour": _state["jour"],
            "mois": _state["mois"],
            "annee": _state["annee"],
            "heure": _state["heure"],
            "minute": _state["minute"],
            "seconde": _state["seconde"],
        }
    return payload

web.expose_api("GET", "/api/state", api_state)

App.run()
