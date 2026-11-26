#pragma once
#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define MQTT_HOST "broker.mqtty.com"
#define MQTT_CLIENT_ID "home-esp1-test"
#define MQTT_USER "esp1-test"
#define MQTT_PASSWORD "YOUR_MQTT_PASSWORD"
#define MQTT_TOPIC "home-test/esp1/telemetry"
// Insert official ISRG Root X1 PEM from https://letsencrypt.org/certs/isrgrootx1.pem
static const char ROOT_CA[] = R"PEM(
REPLACE_WITH_ISRG_ROOT_X1_PEM
)PEM";
