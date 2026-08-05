// ESP8266 IR bridge for an air conditioner, exposed to Home Assistant as an
// MQTT HVAC (climate) entity via MQTT Discovery.
//
// - Sends IR commands to the A/C using IRremoteESP8266's IRac helper.
// - Listens on an IR receiver so unknown remotes/protocols can be captured
//   via Serial and the MQTT "learn" topic.

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <IRac.h>
#include <IRrecv.h>
#include <IRutils.h>

#include "config.h"
#include "secrets.h"

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

IRac ac(kIrLedPin);
IRrecv irrecv(kIrRecvPin, kCaptureBufferSize, kTimeoutMs, true);
decode_results irResults;

String deviceId;
String baseTopic;
String modeCommandTopic, modeStateTopic;
String tempCommandTopic, tempStateTopic;
String fanCommandTopic, fanStateTopic;
String availabilityTopic;
String learnTopic;
String discoveryTopic;

stdAc::opmode_t currentMode = stdAc::opmode_t::kOff;
float currentTemp = 24.0f;
stdAc::fanspeed_t currentFan = stdAc::fanspeed_t::kAuto;

unsigned long lastMqttAttempt = 0;

void publishState() {
  mqttClient.publish(modeStateTopic.c_str(),
                      IRac::opmodeToString(currentMode).c_str(), true);
  mqttClient.publish(tempStateTopic.c_str(),
                      String(currentTemp, 0).c_str(), true);
  mqttClient.publish(fanStateTopic.c_str(),
                      IRac::fanspeedToString(currentFan).c_str(), true);
}

void sendAcState() {
  ac.next.protocol = kAcProtocol;
  ac.next.model = kAcModel;
  ac.next.power = currentMode != stdAc::opmode_t::kOff;
  ac.next.mode = currentMode;
  ac.next.celsius = true;
  ac.next.degrees = currentTemp;
  ac.next.fanspeed = currentFan;
  ac.next.swingv = stdAc::swingv_t::kAuto;
  ac.next.swingh = stdAc::swingh_t::kAuto;
  ac.sendAc();
  publishState();
}

void publishDiscovery() {
  JsonDocument doc;
  doc["name"] = kHaDeviceName;
  doc["unique_id"] = deviceId + "_ac";
  doc["mode_command_topic"] = modeCommandTopic;
  doc["mode_state_topic"] = modeStateTopic;
  doc["temperature_command_topic"] = tempCommandTopic;
  doc["temperature_state_topic"] = tempStateTopic;
  doc["fan_mode_command_topic"] = fanCommandTopic;
  doc["fan_mode_state_topic"] = fanStateTopic;
  doc["availability_topic"] = availabilityTopic;
  doc["payload_available"] = "online";
  doc["payload_not_available"] = "offline";
  doc["min_temp"] = kAcMinTempC;
  doc["max_temp"] = kAcMaxTempC;
  doc["temp_step"] = kAcTempStepC;
  doc["temperature_unit"] = "C";

  JsonArray modes = doc["modes"].to<JsonArray>();
  modes.add("off");
  modes.add("auto");
  modes.add("cool");
  modes.add("heat");
  modes.add("dry");
  modes.add("fan_only");

  JsonArray fanModes = doc["fan_modes"].to<JsonArray>();
  fanModes.add("auto");
  fanModes.add("min");
  fanModes.add("low");
  fanModes.add("medium");
  fanModes.add("high");
  fanModes.add("max");

  JsonObject device = doc["device"].to<JsonObject>();
  device["identifiers"][0] = deviceId;
  device["name"] = kHaDeviceName;
  device["manufacturer"] = "DIY";
  device["model"] = "ESP8266 IR Bridge";

  String payload;
  serializeJson(doc, payload);
  mqttClient.publish(discoveryTopic.c_str(), payload.c_str(), true);
}

void handleModeCommand(const String &payload) {
  currentMode = IRac::strToOpmode(payload.c_str(), stdAc::opmode_t::kOff);
  sendAcState();
}

void handleTempCommand(const String &payload) {
  float temp = payload.toFloat();
  temp = constrain(temp, kAcMinTempC, kAcMaxTempC);
  currentTemp = temp;
  sendAcState();
}

void handleFanCommand(const String &payload) {
  currentFan = IRac::strToFanspeed(payload.c_str(), stdAc::fanspeed_t::kAuto);
  sendAcState();
}

void mqttCallback(char *topic, byte *payloadBytes, unsigned int length) {
  String payload;
  payload.reserve(length);
  for (unsigned int i = 0; i < length; i++) payload += (char)payloadBytes[i];
  String topicStr(topic);

  if (topicStr == modeCommandTopic) {
    handleModeCommand(payload);
  } else if (topicStr == tempCommandTopic) {
    handleTempCommand(payload);
  } else if (topicStr == fanCommandTopic) {
    handleFanCommand(payload);
  }
}

void setupTopics() {
  deviceId = String(ESP.getChipId(), HEX);
  baseTopic = String(kMqttBaseTopic) + "/" + deviceId;

  modeCommandTopic = baseTopic + "/mode/set";
  modeStateTopic = baseTopic + "/mode/state";
  tempCommandTopic = baseTopic + "/temp/set";
  tempStateTopic = baseTopic + "/temp/state";
  fanCommandTopic = baseTopic + "/fan/set";
  fanStateTopic = baseTopic + "/fan/state";
  availabilityTopic = baseTopic + "/availability";
  learnTopic = baseTopic + "/learn";
  discoveryTopic = String(kMqttDiscoveryPrefix) + "/climate/" + deviceId + "/config";
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(kWifiSsid, kWifiPassword);
  Serial.printf("Connecting to WiFi \"%s\"", kWifiSsid);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

bool mqttReconnect() {
  String clientId = "ha_ir_esp8266-" + deviceId;
  bool ok;
  if (strlen(kMqttUser) > 0) {
    ok = mqttClient.connect(clientId.c_str(), kMqttUser, kMqttPassword,
                             availabilityTopic.c_str(), 0, true, "offline");
  } else {
    ok = mqttClient.connect(clientId.c_str(), nullptr, nullptr,
                             availabilityTopic.c_str(), 0, true, "offline");
  }
  if (!ok) return false;

  mqttClient.publish(availabilityTopic.c_str(), "online", true);
  mqttClient.subscribe(modeCommandTopic.c_str());
  mqttClient.subscribe(tempCommandTopic.c_str());
  mqttClient.subscribe(fanCommandTopic.c_str());
  publishDiscovery();
  publishState();
  Serial.println("MQTT connected");
  return true;
}

void checkIrLearn() {
  if (!irrecv.decode(&irResults)) return;

  String protocol = typeToString(irResults.decode_type, irResults.repeat);
  String hexCode = resultToHexidecimal(&irResults);
  Serial.printf("IR received: protocol=%s code=%s bits=%d\n",
                protocol.c_str(), hexCode.c_str(), irResults.bits);

  if (mqttClient.connected()) {
    String payload = "{\"protocol\":\"" + protocol + "\",\"code\":\"" + hexCode +
                      "\",\"bits\":" + String(irResults.bits) + "}";
    mqttClient.publish(learnTopic.c_str(), payload.c_str());
  }
  irrecv.resume();
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  setupTopics();
  setupWifi();

  ac.next.clock = -1;
  irrecv.enableIRIn();

  mqttClient.setBufferSize(768);
  mqttClient.setServer(kMqttHost, kMqttPort);
  mqttClient.setCallback(mqttCallback);
}

void loop() {
  if (!mqttClient.connected()) {
    unsigned long now = millis();
    if (now - lastMqttAttempt > kMqttReconnectDelayMs) {
      lastMqttAttempt = now;
      mqttReconnect();
    }
  } else {
    mqttClient.loop();
  }

  checkIrLearn();
}
