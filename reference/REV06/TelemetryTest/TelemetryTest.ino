#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include "secrets.h"

// HOME CONTROLLER CLOUD - ESP1-TEST REV06
WiFiClientSecure tls;
PubSubClient mqtt(tls);
const uint16_t MQTT_PORT = 8883;
unsigned long lastTry = 0, lastSend = 0, lastStatus = 0;

bool testConnection() {
    Serial.println();
    Serial.println("======================");
    Serial.print("WiFi IP: "); Serial.println(WiFi.localIP());
    Serial.print("WiFi RSSI: "); Serial.println(WiFi.RSSI());
    Serial.print("Unix time: "); Serial.println((unsigned long)time(nullptr));
    Serial.println("TEST 1: DNS");
    IPAddress serverIP;
    if (WiFi.hostByName(MQTT_HOST, serverIP) != 1) {
        Serial.println("DNS FAILED"); return false;
    }
    Serial.print("DNS OK: "); Serial.println(serverIP);
    Serial.println("TEST 2: TCP port 8883");
    WiFiClient tcp;
    if (!tcp.connect(serverIP, MQTT_PORT)) {
        Serial.println("TCP FAILED"); tcp.stop(); return false;
    }
    Serial.println("TCP OK"); tcp.stop();
    delay(300);
    Serial.println("TEST 3: TLS certificate");
    tls.stop(); tls.setCACert(ROOT_CA);
    if (!tls.connect(MQTT_HOST, MQTT_PORT)) {
        Serial.println("TLS FAILED"); tls.stop(); return false;
    }
    Serial.println("TLS OK");
    Serial.println("TEST 4: MQTT login");
    if (!mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
        Serial.print("MQTT FAILED, state: "); Serial.println(mqtt.state());
        tls.stop(); return false;
    }
    Serial.println("MQTT OK");
    Serial.println("ALL TESTS PASSED");
    Serial.println("======================");
    return true;
}
void setup() {
    Serial.begin(115200); delay(500);
    Serial.println();
    Serial.println("HOME CONTROLLER CLOUD");
    Serial.println("ESP1-TEST REV06");
    Serial.println("======================");
    WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Connecting WiFi...");
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    tls.setCACert(ROOT_CA);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    mqtt.setSocketTimeout(5);
    Serial.println("Configuration ready");
}
void loop() {
    unsigned long now = millis();
    if (WiFi.status() != WL_CONNECTED) {
        if (now - lastStatus >= 5000) {
            lastStatus = now; Serial.println("Waiting for WiFi...");
        }
        delay(100); return;
    }
    if (time(nullptr) < 1700000000) {
        if (now - lastStatus >= 5000) {
            lastStatus = now; Serial.println("Waiting for NTP...");
        }
        delay(100); return;
    }
    if (!mqtt.connected()) {
        if (now - lastTry >= 15000) {
            testConnection(); lastTry = millis();
        }
        delay(20); return;
    }
    mqtt.loop();
    if (now - lastSend >= 10000) {
        lastSend = now;
        char payload[180];
        snprintf(payload, sizeof(payload),
            "{\"schema\":1,\"simulated\":true,\"timestamp\":%lu,\"levelCm\":328,\"pump\":false,\"sensor\":\"SIMULATION\"}",
            (unsigned long)time(nullptr));
        Serial.print("Sending: "); Serial.println(payload);
        if (mqtt.publish(MQTT_TOPIC, payload, false)) Serial.println("Published OK");
        else Serial.println("Publish FAILED");
    }
    delay(20);
}
