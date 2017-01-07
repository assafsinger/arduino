
#include <stdio.h>
#include <TimeLib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ESP8266.h"


#define SSID        "finchouse1"
#define PASSWORD    "6352938635293"
//#define SSID        "Ozer"
//#define PASSWORD    "036353838"
#define HOST_NAME   "10.0.0.101"
#define HOST_PORT   (8080)

SoftwareSerial wifiSerial(2, 3); // RX, TX
ESP8266 wifi(wifiSerial);

void setup(void)
{
    Serial.begin(9600);
    Serial.print("setup begin\r\n");

    Serial.print("FW Version:");
    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Serial.print("to station + softap ok\r\n");
    } else {
        Serial.print("to station + softap err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.print("Join AP success\r\n");

        Serial.print("IP:");
        Serial.println( wifi.getLocalIP().c_str());       
    } else {
        Serial.print("Join AP failure\r\n");
    }
    
    if (wifi.disableMUX()) {
        Serial.print("single ok\r\n");
    } else {
        Serial.print("single err\r\n");
    }
    
    Serial.print("setup end\r\n");
}
 
void loop(void)
{
    uint8_t buffer[256] = {0};

    if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    
    char *hello = "GET /getWeather.html HTTP/1.1 \r\nHost: 10.0.0.101\r\n\r\n";
    Serial.println("sending request");
    wifi.send((const uint8_t*)hello, strlen(hello));

    uint32_t len = wifi.recv(buffer, sizeof(buffer), 100000);
    Serial.print("Received response of length ");
    Serial.println(len);
    
    if (len>512){
      buffer[511] = 0;
    }
    if (len > 0) {
//        for(uint32_t i = 0; i < len; i++) {
//            Serial.print(i);
//            Serial.print("\t");
//            Serial.print(buffer[i]);
//            Serial.print("\n");
//         }
              Serial.print("buffer:");
              Serial.print((char*)buffer);
              char* line = strtok((char*)buffer, "\n");
//              Serial.print(line);
              char* lastLine;
              while(line)
              {
                  Serial.print(line);
                  lastLine=line;
                  line = strtok(NULL, "\n");
              }
              Serial.print("string value: ");
              Serial.print(lastLine);
              Serial.print("\n");
              char *ptr;
              long int res = strtol(lastLine, &ptr, 10);
              Serial.print("long converted string is");
              Serial.print(res);
              Serial.print("\n");
              //Time zone - UTC+2
              setTime(res + 2*60*60);
                Serial.print(hour());
            printDigits(minute());
            printDigits(second());
            Serial.print(" ");
            Serial.print(dayStr(weekday()));
            Serial.print(" ");
            Serial.print(day());
            Serial.print(" ");
            Serial.print(monthStr(month()));
            Serial.print(" ");
            Serial.print(year()); 
            Serial.println(); 

           }
//     }

    if (wifi.releaseTCP()) {
        Serial.print("release tcp ok\r\n");
    } else {
        Serial.print("release tcp err\r\n");
    }
    
    while(1);
    
}


void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
