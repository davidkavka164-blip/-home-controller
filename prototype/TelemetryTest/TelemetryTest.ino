#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include "secrets.h"
WiFiClientSecure tls;
PubSubClient mqtt(tls);
unsigned long lastTry=0,lastSend=0;
void setup(){
 Serial.begin(115200);
 WiFi.mode(WIFI_STA);
 WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
 configTime(0,0,"pool.ntp.org","time.google.com");
 tls.setCACert(ROOT_CA);
 mqtt.setServer(MQTT_HOST,8883);
 mqtt.setSocketTimeout(5);
}
void loop(){
 if(WiFi.status()!=WL_CONNECTED || time(nullptr)<1700000000){delay(100);return;}
 unsigned long now=millis();
 if(!mqtt.connected()){
  if(now-lastTry>=15000){lastTry=now;Serial.println(mqtt.connect(MQTT_CLIENT_ID,MQTT_USER,MQTT_PASSWORD)?"MQTT OK":"MQTT failed");}
  delay(20);return;
 }
 mqtt.loop();
 if(now-lastSend>=10000){
  lastSend=now;
  char payload[180];
  snprintf(payload,sizeof(payload),"{\"schema\":1,\"simulated\":true,\"timestamp\":%lu,\"levelCm\":328,\"pump\":false,\"sensor\":\"SIMULATION\"}",(unsigned long)time(nullptr));
  Serial.println(mqtt.publish(MQTT_TOPIC,payload,false)?"Published":"Publish failed");
 }
 delay(20);
}
