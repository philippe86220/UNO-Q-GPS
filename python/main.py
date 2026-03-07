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
    "numsat": None, 
    "altitude": None,
}

def now_utc_iso():
    #ISO 8601 UTC, ex: 2026-03-01T08:22:29Z
    return datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")

def update_gps(lat,long, jour, mois, annee, heure, minute,seconde, numsat, altitude):
    
    with _lock:
        _state["lat"] = lat
        _state["long"] = long
        _state["jour"] = jour
        _state["mois"] = mois
        _state["annee"] = annee
        _state["heure"] = heure
        _state["minute"] = minute
        _state["seconde"] = seconde
        _state["numsat"] = numsat
        _state["altitude"] = altitude

Bridge.provide("update_gps", update_gps)


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
            "numsat": _state["numsat"],
            "altitude": _state["altitude"],
        }
    return payload

web.expose_api("GET", "/api/state", api_state)

App.run()
