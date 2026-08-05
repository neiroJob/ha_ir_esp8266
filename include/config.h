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
// Unit: NEOLINE NAG/in-09HN1. Its official WiFi accessory uses the
// "NetHome Plus" app (Midea's app, package com.midea.aircondition), and the
// remote's feature set (iClean, Turbo, Health, Quiet, H/V-Sweep) matches
// Midea-OEM firmware, so this is a Midea unit rebadged as NEOLINE.
// Confirm/correct via the IR receiver + Serial monitor (see the "learn"
// topic in main.cpp) if commands don't behave as expected.
constexpr decode_type_t kAcProtocol = decode_type_t::MIDEA;
constexpr int16_t kAcModel = -1;  // Not used by the Midea protocol.

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
