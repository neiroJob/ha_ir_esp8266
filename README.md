# ha_ir_esp8266

ESP8266-based IR bridge that lets Home Assistant control an air conditioner
over MQTT (MQTT Discovery, exposed as a `climate` entity).

## Hardware

Target board: **TYSW_012_ZK_MAIN_V1.1** — a round Tuya universal IR remote
puck built around a bare **TYWE3S** module (ESP8266EX, 1MB flash). It matches
Tasmota's ["YTF IR Bridge"](https://tasmota.github.io/docs/devices/YTF-IR-Bridge/)
template, so pins in `include/config.h` are set accordingly:

- `GPIO14` — IR LED ring (via transistor `U2`), transmit
- `GPIO5` — onboard IR receiver
- `GPIO4` — onboard blue status LED (active low, not used yet)
- `GPIO13` — onboard button (not used yet)

The board exposes a 7-pin header (`3V3 RST TXD RXD IO0 IO2 GND`) for
flashing. Wire a 3.3V USB-TTL adapter:

| Board header | USB-TTL adapter |
|---|---|
| 3V3 | 3.3V |
| GND | GND |
| TXD | RX |
| RXD | TX |
| IO0 | GND (only while flashing — puts the ESP8266 in bootloader mode) |

Ground `IO0`, then power up / press `RST` to enter flash mode, run the
upload, then disconnect `IO0` from GND and reset to run the firmware
normally. `board_build.flash_size` in `platformio.ini` is set to 1MB per the
TYWE3S datasheet — if `pio run -t upload` reports a flash size mismatch,
adjust it there (or run `esptool.py flash_id` to confirm the real chip).

## Setup

1. Copy `include/secrets.h.example` to `include/secrets.h` and fill in your
   WiFi and MQTT broker credentials.
2. In `include/config.h`, set `kAcProtocol` to match your A/C unit's IR
   protocol (see the comment there — default is `COOLIX`, a common generic
   protocol). If you don't know your protocol, flash the firmware, point your
   original remote at the onboard IR receiver, and watch the Serial monitor
   / the `ha_ir_esp8266/<id>/learn` MQTT topic for the detected protocol and
   code.
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
