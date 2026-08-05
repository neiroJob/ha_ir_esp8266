#pragma once

#include <IRsend.h>

// ---------------------------------------------------------------------------
// Hardware pins (Wemos D1 mini labels)
// ---------------------------------------------------------------------------
constexpr uint16_t kIrLedPin   = D2;  // IR LED (transmit), through a transistor driver
constexpr uint16_t kIrRecvPin  = D5;  // IR receiver module (e.g. TSOP38238)

constexpr uint16_t kCaptureBufferSize = 1024;
constexpr uint8_t  kTimeoutMs         = 15;  // Suits most A/C units.

// ---------------------------------------------------------------------------
// Air conditioner protocol
// ---------------------------------------------------------------------------
// Set this to the protocol your A/C uses. See the full list of supported
// protocols in IRsend.h / IRac.cpp (e.g. COOLIX, GREE, MITSUBISHI_AC,
// FUJITSU_AC, DAIKIN, PANASONIC_AC, TOSHIBA_AC, HITACHI_AC, SAMSUNG_AC...).
// COOLIX is a common generic protocol used by many cheap split A/C units,
// so it's a reasonable starting default. Use the IR receiver + Serial
// monitor (see the "learn" topic in main.cpp) to identify your actual unit.
constexpr decode_type_t kAcProtocol = decode_type_t::COOLIX;
constexpr int16_t kAcModel = -1;  // Only needed for a few protocols (e.g. Fujitsu, Kelvinator).

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
