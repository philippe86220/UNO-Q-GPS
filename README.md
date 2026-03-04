# UNO-Q-GPS
Application pour lire les Trames d'un capteur GNSS et les afficher sur une page HTML  
Permet de mettre en évidence l'utilisation de l'UART matériel de l'UNO-Q

## 1. Principe de fonctionnement :

- Une librairie permet d'extraire les champs necessaires au bon fonctionnement du programme
- Bridge transmet les champs de cette façon au MPU :

```
Bridge.call("update_gps", gps.latitude, gps.longitude, gps.jour, gps.mois, gps.annee+2000,(gps.heure) % 24, gps.minute, gps.seconde, gps.numSat, gps.altitude);
```

- Le coeur LINUX récupère ces champs : 

```
def update_gps(lat,long, jour, mois, annee, heure, minute,seconde, numsat, altitude):
    with _lock:
        _state["lat"] = float(lat)
        _state["long"] = float(long)
        _state["jour"] = int(jour)
        _state["mois"] = int(mois)
        _state["annee"] = int(annee)
        _state["heure"] = int(heure)
        _state["minute"] = int(minute)
        _state["seconde"] = int(seconde)
        _state["numsat"] = int(numsat)
        _state["altitude"] = float(altitude)

Bridge.provide("update_gps", update_gps)
```

## 🔄 Transformation Python → JSON → WebUI

### 1. Le dictionnaire Python `payload`

```python
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
```

`payload`= dictionnaire Python natif (types Python : `str`, `float`, `int`)

### 2. Sérialisation automatique par WebUI

```
web.expose_api("GET", "/api/state", api_state)  
```

`WebUI` fait AUTOMATIQUEMENT :


```
payload (dict Python) 
    ↓ json.dumps() interne
{"now_utc":"2026-03-03T08:39:00Z","lat":46.123456,...} (JSON string)
    ↓ HTTP response
Content-Type: application/json

```

### 3. Flux complet

```
1. GPS → update_gps() → _state (Python dict)
2. fetch("/api/state") → api_state() → payload (Python dict) 
3. WebUI → JSON automatique → HTTP response
4. JavaScript → r.json() → objet JS
5. DOM → affichage HTML
```

### 4. Types : Python → JSON → JavaScript

| Python | JSON    | JavaScript |
| ------ | ------- | ---------- |
| str    | "texte" | "texte"    |
| float  | 36.123  | 36.123     |
| int    | 3       | 3          |
| None   | null    | null       |

### 5. Test API

```
curl http://localhost/api/state
# → {"now_utc":"2026-03-03T08:39:00Z","lat":36.123456,...}
```

WebUI gère TOUT : sérialisation JSON, headers HTTP, CORS, etc. ✨


### Schéma visuel pour README

```
graph LR
    GPS[GPS NMEA] -->|update_gps| State[_state dict]
    Req[fetch("/api/state")] --> API[api_state()]
    API --> Payload[payload dict]
    Payload -->|WebUI auto| JSON["JSON string"]
    JSON -->|HTTP 200| JS[r.json()]
    JS --> DOM[HTML DOM]
```

Avantage : Zéro code JSON manuel - WebUI s'occupe de tout ! ✅

## 2. Librairie pour parser les TrameNMEA – Décodage minimaliste des trames GPS RMC / GGA / GSA

Librairie C++ pour cartes **Arduino AVR et UNO-Q** (ATmega328, Arduino UNO…) permettant  
d’extraire rapidement les informations clés des phrases NMEA `$GPRMC`,
`$GPGGA` et `$GPGSA`.

> **Objectif** : fournir un parseur léger 
> sans dépendance externe, capable de tourner sur un Nano / Uno R3 et UNO-Q.

---

## Connexion matérielle

| GPS module pin | UNO-Q pin |
|----------------|------------|
| TX (GPS)       |  RX UNO-Q
| RX (GPS)       |  TX UNO-Q
| VCC / GND      | 3.3 V / GND |

Assurez‑vous que le module émet en 9600 bauds (valeur par défaut de la plupart
des récepteurs).

Type de module utilisé pour la mise en oeuvre de la librairie :
https://www.gotronic.fr/art-module-gps-tel0094-25732.htm


## API

| Méthode                       | Effet                                                     |
| -----------------------------  | -------------------------------------------------------- |
| `Trame(char* uneTrame)`        | Constructeur (une seule instance recommandée).           |
| `void setSentence(char* nmea)` | Pointeur vers la nouvelle phrase NMEA.                   |
| `bool extrait()`               | Identifie et parse ; renvoie `false` si échec.           |
| `bool estValide() const`       | `true` si `valid == 'A'` **et** `fixQuality > 0`.        |
| `Type type() const`            | Enum `GPRMC / GPGGA / GPGSA / INCONNU`.                  |


## Champs publics mis à jour

- latitude, longitude, vitesse, route
- jour, mois, annee, heure, minute, seconde
- fixQuality, numSat, hdop, altitude, geoidSeparation
- selMode, fixType, satID[12], pdop, hdop_gsa, vdop

## Structure interne
```
buffers statiques  ─┬─ _bufRMC[12][20]
                    ├─ _bufGGA[14][20]
                    └─ _bufGSA[17][20]

Trame::extrait()  → détecte type → choisit buffer → découpe → parseRMC/GGA/GSA
```
