# UNO-Q GPS Application --- Data Flow Explanation

This document explains the exact data flow used in the UNO‑Q GPS
demonstration application.\
The goal is to clarify how data travels from the GNSS sensor to the web
dashboard.

------------------------------------------------------------------------

# 1. The Two Main Threads

In the Python application there are two important threads created
internally by the runtime.

## Bridge Thread (`Bridge.read_loop`)

This thread:

-   receives data sent by the MCU
-   calls the Python function:

``` python
update_gps(...)
```

This function:

-   updates the shared dictionary:

``` python
_state
```

Therefore this thread **writes to `_state`**.

------------------------------------------------------------------------

## HTTP Thread (AnyIO worker thread)

When the browser sends an HTTP request:

    GET /api/state

the web server calls:

``` python
api_state()
```

This function:

-   reads the dictionary:

``` python
_state
```

Therefore this thread **reads `_state`**.

------------------------------------------------------------------------

# 2. Concurrent Access to the Dictionary

We therefore have:

    Bridge thread  → writes to _state
    HTTP thread    → reads from _state

Both threads can be active **concurrently**.

This means their execution can **interleave in time**.

This is exactly the situation where a lock is required:

``` python
_lock = threading.Lock()
```

The lock guarantees that:

    update_gps()  and  api_state()

cannot access `_state` **at the same time**.

------------------------------------------------------------------------

# 3. Role of the Browser

The web page contains the following JavaScript code:

``` javascript
const r = await fetch("/api/state", { cache: "no-store" });
const s = await r.json();
```

This means:

1.  the browser sends an HTTP request

    GET /api/state

2.  the Python server executes

``` python
api_state()
```

3.  the function returns a Python dictionary:

``` python
payload
```

------------------------------------------------------------------------

# 4. Automatic Conversion to JSON

The WebUI framework automatically converts the Python dictionary:

``` python
payload
```

into **JSON**.

Example response:

``` json
{
  "lat": 48.123456,
  "long": 2.456789,
  "numsat": 10
}
```

------------------------------------------------------------------------

# 5. Reading Data in the Browser

Then the browser executes:

``` javascript
const s = await r.json();
```

This converts the JSON response into a **JavaScript object**.

The values can then be accessed as:

``` javascript
s.lat
s.long
s.numsat
```

These values are displayed in the HTML page.

------------------------------------------------------------------------

# 6. Complete Data Flow

The complete data flow of the system is:

    MCU
     ↓
    update_gps()
     ↓
    _state
     ↑
    api_state()
     ↑
    HTTP request (fetch)
     ↑
    Browser

------------------------------------------------------------------------

# 7. Summary

The application works as follows:

-   The MCU sends GNSS data through the Bridge.
-   The Bridge thread calls `update_gps()` which updates `_state`.
-   The browser periodically requests `/api/state`.
-   The HTTP thread calls `api_state()` which reads `_state`.
-   The dictionary is converted automatically to JSON.
-   The browser receives the JSON and displays the values.

Because two threads access `_state`, a **threading lock (`_lock`)** is
used to guarantee data consistency.

