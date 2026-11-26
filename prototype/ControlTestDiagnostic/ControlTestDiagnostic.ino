#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <time.h>
#include <esp_system.h>
#include "secrets.h"
WiFiClientSecure tls;
PubSubClient mqtt(tls);
const char* commandTopic="home-app/esp1/command";
const char* stateTopic="home-test/esp1/control";
char session[33];
unsigned long lastTry=0,lastSend=0;
bool attempted=false;
unsigned long lastId=0;
bool output=false;
void report(){
 char msg[220];
 snprintf(msg,sizeof(msg),"{\"schema\":1,\"simulated\":true,\"session\":\"%s\",\"id\":%lu,\"output\":%s,\"timestamp\":%lu}",session,lastId,output?"true":"false",(unsigned long)time(nullptr));
 mqtt.publish(stateTopic,msg,false);
}
void receive(char* topic,byte* payload,unsigned int len){
 if(strcmp(topic,commandTopic)||len==0||len>=100)return;
 char buf[100]; memcpy(buf,payload,len);buf[len]=0;
 if(strlen(buf)!=len)return;
 char token[33],action[4]; unsigned long id,expiry; int end=0;
 if(sscanf(buf,"%32[0-9a-f] %lu %lu %3s%n",token,&id,&expiry,action,&end)!=4||end!=(int)len)return;
 if(strcmp(token,session)||id==0||id<=lastId)return;
 unsigned long now=(unsigned long)time(nullptr);
 if(now<1700000000UL||expiry<now||expiry-now>15)return;
 if(strcmp(action,"ON")&&strcmp(action,"OFF"))return;
 output=!strcmp(action,"ON");lastId=id;report();
 Serial.printf("Virtual output: %s, command %lu\n",action,id);
}
bool connectDiagnostic(){
 Serial.println("REV07.1: connection diagnostic");
 Serial.print("WiFi RSSI: "); Serial.println(WiFi.RSSI());
 Serial.print("Unix time: "); Serial.println((unsigned long)time(nullptr));
 Serial.print("MQTT user: "); Serial.println(MQTT_USER);
 Serial.print("Client ID: "); Serial.println(MQTT_CLIENT_ID);
 Serial.print("Password length: "); Serial.println(strlen(MQTT_PASSWORD));
 IPAddress ip;
 if(WiFi.hostByName(MQTT_HOST,ip)!=1){Serial.println("DNS FAILED");return false;}
 Serial.print("DNS OK: ");Serial.println(ip);
 tls.stop();tls.setCACert(ROOT_CA);
 if(!tls.connect(MQTT_HOST,8883)){Serial.println("TLS FAILED (before MQTT login)");tls.stop();return false;}
 Serial.println("TLS OK");
 if(!mqtt.connect(MQTT_CLIENT_ID,MQTT_USER,MQTT_PASSWORD)){
  Serial.print("MQTT FAILED state: ");Serial.println(mqtt.state());
  tls.stop();return false;
 }
 Serial.println("MQTT OK");return true;
}
void setup(){
 Serial.begin(115200);Serial.println("HOME CONTROLLER REV07.1");WiFi.mode(WIFI_STA);WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
 configTime(0,0,"pool.ntp.org","time.google.com");
 tls.setCACert(ROOT_CA);mqtt.setServer(MQTT_HOST,8883);mqtt.setSocketTimeout(5);mqtt.setCallback(receive);
}
void loop(){
 unsigned long now=millis();
 if(WiFi.status()!=WL_CONNECTED||time(nullptr)<1700000000){if(tls.connected())tls.stop();delay(100);return;}
 if(!mqtt.connected()){
  if(!attempted||now-lastTry>=60000){
   attempted=true;
   if(connectDiagnostic()){
    snprintf(session,sizeof(session),"%08lx%08lx%08lx%08lx",(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random(),(unsigned long)esp_random());
    lastId=0;output=false;
    if(mqtt.subscribe(commandTopic,0)){report();Serial.println("REV07 connected");}else{mqtt.disconnect();tls.stop();}
   }
   lastTry=millis();
  }
  delay(20);return;
 }
 mqtt.loop();
 if(now-lastSend>=10000){
  lastSend=now;
  char msg[180];snprintf(msg,sizeof(msg),"{\"schema\":1,\"simulated\":true,\"timestamp\":%lu,\"levelCm\":328,\"pump\":false,\"sensor\":\"SIMULATION\"}",(unsigned long)time(nullptr));
  mqtt.publish(MQTT_TOPIC,msg,false);report();
 }
 delay(20);
}
