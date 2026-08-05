# ha_ir_esp8266

ESP8266-based IR bridge that lets Home Assistant control an air conditioner
over MQTT (MQTT Discovery, exposed as a `climate` entity).

## Hardware

- ESP8266 board (default config targets a Wemos D1 mini, see `platformio.ini`)
- IR LED (+ transistor driver) on `D2` for transmitting
- IR receiver module (e.g. TSOP38238) on `D5` for learning/debugging

## Setup

1. Copy `include/secrets.h.example` to `include/secrets.h` and fill in your
   WiFi and MQTT broker credentials.
2. In `include/config.h`, set `kAcProtocol` to match your A/C unit's IR
   protocol (see the comment there — default is `COOLIX`, a common generic
   protocol). If you don't know your protocol, flash the firmware, open the
   point your original remote at the IR receiver, and watch the Serial
   monitor / the `ha_ir_esp8266/<id>/learn` MQTT topic for the detected
   protocol and code.
3. Build and upload with PlatformIO:

   ```
   pio run -t upload
   ```

4. The device will publish an MQTT Discovery config for a `climate` entity;
   it should appear automatically in Home Assistant.

## MQTT topics

All topics are prefixed with `ha_ir_esp8266/<chip-id>/`:

- `mode/set`, `mode/state` — off/auto/cool/heat/dry/fan_only
- `temp/set`, `temp/state` — target temperature in °C
- `fan/set`, `fan/state` — auto/min/low/medium/high/max
- `availability` — online/offline (LWT)
- `learn` — JSON dump of any IR code the receiver captures (debugging)

## Status

Initial skeleton: WiFi + MQTT connectivity, Home Assistant MQTT Discovery,
IR transmit via `IRac`, and IR receive for protocol learning. Not yet
flashed/tested on real hardware.
