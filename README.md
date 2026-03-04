# UNO-Q-GPS

Example application demonstrating how to read GNSS NMEA frames on the **Arduino UNO-Q**, process them on the MCU using a lightweight C++ parser, transmit the extracted data to the Linux MPU via **Bridge**, and display the information in a **web dashboard**.

This project highlights the **hybrid architecture of the UNO-Q**, combining:

- a real-time MCU
- a Linux MPU
- a web interface

---

# Architecture Overview

The GNSS receiver sends NMEA sentences through the hardware UART of the UNO-Q.

The MCU parses these sentences and extracts the useful fields.

The extracted data are sent to the Linux system through **Bridge RPC calls**, where a Python application stores them and exposes them through a **REST API**.

The HTML dashboard retrieves the data from this API and displays them in real time.

```
GNSS module
↓
MCU (UART + NMEA parser)
↓
Bridge RPC
↓
Linux MPU (Python)
↓
REST API
↓
JSON
↓
Web Browser Dashboard
```
---

# Data Flow

```
GPS NMEA sentences
        ↓
MCU parser (C++ library)
        ↓
Bridge.call()
        ↓
Python update_gps()
        ↓
_state dictionary
        ↓
WebUI API (/api/state)
        ↓
JSON response
        ↓
JavaScript fetch()
        ↓
HTML dashboard update
```
---

# MCU → Linux Communication
When a valid GPS fix is detected, the MCU sends the parsed values to the Linux side.

```
Bridge.call(
  "update_gps",
  gps.latitude,
  gps.longitude,
  gps.jour,
  gps.mois,
  gps.annee + 2000,
  (gps.heure) % 24,
  gps.minute,
  gps.seconde,
  gps.numSat,
  gps.altitude
);
```

# Python Application
The Python application receives these values through the Bridge interface and stores them in a shared dictionary.

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

# REST API
The WebUI exposes the GPS data through a REST endpoint.

```
web.expose_api("GET", "/api/state", api_state)
```

Example request :

```
curl http://localhost/api/state
```

Example response :

```
{
 "now_utc":"2026-03-03T08:39:00Z",
 "lat":36.123456,
 "long":0.754321,
 "numsat":10,
 "altitude":120.3
}
```

WebUI automatically handles :
- JSON serialization
- HTTP headers
- REST endpoint

# Web Dashboard
The HTML page periodically fetches the API and updates the interface.

```
const r = await fetch("/api/state");
const s = await r.json();
```

The dashboard updates every second :  

```
setInterval(refresh, 1000);
```

Displayed data include :
- local time
- latitude
- longitude
- GPS UTC date
- GPS UTC time
- number of satellites
- altitude

# NMEA Parsing Library
This project includes a lightweight C++ NMEA parser designed for :
- Arduino AVR boards
- Arduino UNO-Q
- low memory environments
- 
Supported sentences :
- $GPRMC
- $GPGGA
- $GPGSA
- 
The goal of this library is minimal decoding with no external dependencies, suitable for small  
microcontrollers.

Extracted data include :
- latitude
- longitude
- speed
- route
- UTC date/time
- satellite count
- altitude
- HDOP / PDOP / VDOP

# Hardware Connection

| GPS Module | UNO-Q |
| ---------- | ----- |
| TX         | RX    |
| RX         | TX    |
| VCC        | 3.3V  |
| GND        | GND   |

Example GPS module used :

https://www.gotronic.fr/art-module-gps-tel0094-25732.htm  

Default baud rate: **9600**  

# Purpose of This Project

This example demonstrates :

- use of the **UNO-Q hardware UART**
- parsing NMEA sentences on the MCU
- communication between MCU and Linux using **Bridge**
- building a **Python REST API**
- automatic JSON generation using **WebUI**
- displaying GPS data in a **browser dashboard**
- 
It also illustrates how to design applications for the **hybrid architecture of the Arduino UNO-Q.**

## Screenshots
![Serveur](/docs//screenshot.jpg)

# License
MIT


