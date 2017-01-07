#include<stdlib.h>
#include <SoftwareSerial.h>
#include "ESP8266.h"

#define SSID "finchouse1"
#define PASS "6352938635293"
#define HOST_NAME   "www.timeapi.org"
#define HOST_PORT   (80)
char* TIME_URL = "GET / HTTP/1.1\r\nHost: http://www.timeapi.org/eet/now?format=%25A,%20%25B%20%25d%20%25Y$%20%25H%3A%25M%3A%25S%20\r\nConnection: close\r\n\r\n";
SoftwareSerial wifiSerial(2, 3); // RX, TX
ESP8266 wifi(wifiSerial);

void setup()
{
  wifiSerial.begin(9600);
  Serial.begin(9600);
  sendDebug("AT");
  delay(1000);
  if(wifiSerial.find("OK")){
    Serial.println("RECEIVED: wifi board is alive");
    if (connectWiFi()){
        
      
    }
  }
}

void loop(void) {
    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }
    
    uint8_t buffer[512] = {0};

    wifi.send((const uint8_t*)TIME_URL, strlen(TIME_URL));

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 10000);
    if (len > 0) {
        Serial.print("Received:[");
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        Serial.print("]\r\n");
    }

    delay(1000);
    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }
    
    delay(1000);
//    
}

void sendDebug(String cmd){
  Serial.print("SEND: ");
  Serial.println(cmd);
  wifiSerial.println(cmd);
} 

boolean connectWiFi(){
//  wifi.println("AT+CWMODE=1");
//  delay(2000);
//  String cmd="AT+CWJAP=\"";
//  cmd+=SSID;
//  cmd+="\",\"";
//  cmd+=PASS;
//  cmd+="\"";
//  sendDebug(cmd);
//  delay(5000);
//  if(wifi.find("OK")){
//    Serial.println("RECEIVED: OK");
//    return true;
//  }else{
//    Serial.println("RECEIVED: Error");
//    return false;
//  }

    if (wifi.joinAP(SSID, PASS)) {
        Serial.print("Join AP success\r\n");
        Serial.print("IP: ");       
        Serial.println(wifi.getLocalIP().c_str());
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    Serial.print("setup end\r\n");
}
