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
void setup(){
 Serial.begin(115200);WiFi.mode(WIFI_STA);WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
 configTime(0,0,"pool.ntp.org","time.google.com");
 tls.setCACert(ROOT_CA);mqtt.setServer(MQTT_HOST,8883);mqtt.setSocketTimeout(5);mqtt.setCallback(receive);
}
void loop(){
 unsigned long now=millis();
 if(WiFi.status()!=WL_CONNECTED||time(nullptr)<1700000000){if(tls.connected())tls.stop();delay(100);return;}
 if(!mqtt.connected()){
  if(now-lastTry>=15000){
   tls.stop();
   if(mqtt.connect(MQTT_CLIENT_ID,MQTT_USER,MQTT_PASSWORD)){
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
