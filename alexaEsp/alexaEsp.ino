#include "debug.h"              // Serial debugger printing
#include <ESP8266WiFiMulti.h>
//#include <ESP8266WiFi.h>
//#include <fauxmoESP.h>
#include <Espalexa.h>
#include "Shutters.h"
#include "ElectraAc.h"


//WifiConnection* wifi;           // wifi connection
ESP8266WiFiMulti wifi;


//SET TO MATCH YOUR HARDWARE 
#define SERIAL_BAUD_RATE    115200

#define RADIO_PIN              D1
#define IR_PIN                 D2

//fauxmoESP fauxmo;
Shutters  shutters(RADIO_PIN);
ElectraAc electraAc(IR_PIN);
Espalexa espalexa;


const char * SHUTTERS_MAME = "bedroom shutters";
const char * AC_NAME_HEAT = "heat";
const char * AC_NAME_COOL = "cool";
const char * AC_NAME = "ac";

//const char* SUPPORTED_MODULES[] = {SHUTTERS_MAME, AC_MAME_HEAT, AC_MAME_COOL, AC_MAME}; //all modules

void ac(uint8_t level);
void acHeat(uint8_t level);
void acCool(uint8_t level);


// ************************************************************************************
//Runs once, when device is powered on or code has just been flashed 
//
void setup()
{
  //if set wrong, your serial debugger will not be readable 
  Serial.begin(SERIAL_BAUD_RATE);

  //initialize wifi connection 
  WiFi.mode(WIFI_STA);
  wifi.addAP("finchouse_ext", "6352938635293");
  wifi.addAP("finchouse", "6352938635293");

  Serial.println("Connecting Wifi...");

  //connect to wifi 
  while(wifi.run() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    Serial.println(F("adding support for the folowing modules:"));
    Serial.print(AC_NAME_HEAT);
    Serial.print(F(","));
    espalexa.addDevice(AC_NAME_HEAT, acHeat);
    
    Serial.print(AC_NAME_COOL);
    Serial.print(F(","));
    espalexa.addDevice(AC_NAME_COOL, acCool);
    Serial.print(AC_NAME);
    Serial.print(F(","));
    espalexa.addDevice(AC_NAME, ac);
    Serial.println();
    
    
    espalexa.begin();
    electraAc.begin();
    
//    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
//        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
//        if (strcmp(SHUTTERS_MAME,device_name) == 0){
//          if (state){
//              Serial.print(F("Setting shutters "));
//              short openPercentage = value*100/255;
//              shutters.setShutterState(openPercentage);
//              Serial.printf(" %d\% UP\n",openPercentage);
//          } else {
//              shutters.closeShutter();
//              Serial.println(F("DOWN"));
//
//          }
//        } else if (strcmp(AC_NAME_HEAT,device_name) == 0){
//          electraAc.toggleHeat();
//        } else if (strcmp(AC_NAME_COOL,device_name) == 0){
//          electraAc.toggleCool();
//        } else if (strcmp(AC_NAME,device_name) == 0){
//          electraAc.toggleLastState();
//        }      
//    });
//  }
  }
}


// ************************************************************************************
// Runs constantly 
//
void loop() 
{
  if (wifi.run() != WL_CONNECTED) {
    Serial.println("WiFi not connected!");
    delay(1000);
  }  
    espalexa.loop();
    if (shutters.isWrap()){
      shutters.wrap();
    }
    
  }

void ac(uint8_t level){
    Serial.println("ac toggled");
    electraAc.toggleLastState();
}

void acHeat(uint8_t level){
  Serial.println("heat toggled");
  electraAc.toggleHeat();
}

void acCool(uint8_t level){
  Serial.println("cool toggled");
  electraAc.toggleCool();
}

