#pragma once

#include <IRsend.h>

// ---------------------------------------------------------------------------
// Hardware pins — TYSW_012_ZK_MAIN_V1.1 (round Tuya IR puck, TYWE3S module).
// Matches the Tasmota "YTF IR Bridge" template for this board family:
// https://tasmota.github.io/docs/devices/YTF-IR-Bridge/
// ---------------------------------------------------------------------------
constexpr uint16_t kIrLedPin   = 14;  // GPIO14 - IR LED array, via transistor U2
constexpr uint16_t kIrRecvPin  = 5;   // GPIO5  - onboard IR receiver
constexpr uint16_t kStatusLedPin = 4; // GPIO4  - onboard blue LED (active low), unused for now
constexpr uint16_t kButtonPin  = 13;  // GPIO13 - onboard button, unused for now

constexpr uint16_t kCaptureBufferSize = 1024;
constexpr uint8_t  kIrRecvTimeoutMs   = 15;  // Suits most A/C units.

// ---------------------------------------------------------------------------
// Air conditioner protocol
// ---------------------------------------------------------------------------
// Unit: NEOLINE NAG/in-09HN1. Confirmed directly by pointing the original
// remote at the board's onboard IR receiver: pressing power on/off decodes
// as protocol MIRAGE, 120 bits (matches kMirageStateLength = 15 bytes).
// (The earlier guess of MIDEA, based on the WiFi accessory app, was wrong.)
// MIRAGE has two known remote model variants in IRremoteESP8266
// (mirage_ac_remote_model_t::kKKG9AC vs kKKG29AC) with slightly different
// state layouts. -1 uses the library default (kKKG9AC); if commands are
// accepted but do the wrong thing, try the other model.
constexpr decode_type_t kAcProtocol = decode_type_t::MIRAGE;
constexpr int16_t kAcModel = -1;

constexpr float kAcMinTempC = 16.0f;
constexpr float kAcMaxTempC = 30.0f;
constexpr float kAcTempStepC = 1.0f;

// ---------------------------------------------------------------------------
// MQTT / Home Assistant
// ---------------------------------------------------------------------------
constexpr char kHaDeviceName[] = "AC IR Bridge";
constexpr char kMqttDiscoveryPrefix[] = "homeassistant";
constexpr char kMqttBaseTopic[] = "ha_ir_esp8266";  // suffixed with the chip id at runtime

constexpr uint32_t kMqttReconnectDelayMs = 5000;
