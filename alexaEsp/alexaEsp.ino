#include "debug.h"              // Serial debugger printing
//#include "WifiConnection.h"     // Wifi connection 
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <fauxmoESP.h>
#include "Shutters.h"
#include "ElectraAc.h"


//WifiConnection* wifi;           // wifi connection
ESP8266WiFiMulti wifi;


//SET YOUR WIFI CREDS 
//const char* myWifiSsid      = "finchouse_ext"; 
//const char* myWifiPassword  = "6352938635293";

//SET TO MATCH YOUR HARDWARE 
#define SERIAL_BAUD_RATE    115200

#define RADIO_PIN              D1
#define IR_PIN                 D2

fauxmoESP fauxmo;
Shutters  shutters(RADIO_PIN);
ElectraAc electraAc(IR_PIN);

const char * SHUTTERS_MAME = "bedroom shutters";
const char * AC_NAME_HEAT = "ac heat";
const char * AC_NAME_COOL = "ac cool";
const char * AC_NAME = "ac";

//const char* SUPPORTED_MODULES[] = {SHUTTERS_MAME, AC_MAME_HEAT, AC_MAME_COOL, AC_MAME}; //all modules
//const char* SUPPORTED_MODULES[] = {AC_NAME_HEAT, AC_NAME_COOL, AC_NAME};
const char* SUPPORTED_MODULES[] = {AC_NAME};


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
  //wifi.addAP("finchouse", "6352938635293");

  Serial.println("Connecting Wifi...");
  //wifi = new WifiConnection(myWifiSsid, myWifiPassword); 
  //wifi->begin(); 

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
    for(int i=0; i<sizeof(SUPPORTED_MODULES)/sizeof(SUPPORTED_MODULES[0]);i++){
      Serial.print(SUPPORTED_MODULES[i]);
      Serial.print(F(","));
      fauxmo.addDevice(SUPPORTED_MODULES[i]);
    }
    Serial.println();
    
    
    fauxmo.setPort(80); // required for gen3 devices
    fauxmo.enable(true);
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        if (strcmp(SHUTTERS_MAME,device_name) == 0){
          if (state){
              Serial.print(F("Setting shutters "));
              short openPercentage = value*100/255;
              shutters.setShutterState(openPercentage);
              Serial.printf(" %d\% UP\n",openPercentage);
          } else {
              shutters.closeShutter();
              Serial.println(F("DOWN"));

          }
        } else if (strcmp(AC_NAME_HEAT,device_name) == 0){
          electraAc.toggleHeat();
        } else if (strcmp(AC_NAME_COOL,device_name) == 0){
          electraAc.toggleCool();
        } else if (strcmp(AC_NAME,device_name) == 0){
          electraAc.toggleLastState();
        }      
    });
  }
}


// ************************************************************************************
// Runs constantly 
//
void loop() 
{    
  //let the wemulator listen for voice commands 
  if (WiFi.status() != WL_CONNECTED)
  {
    //blinkLed(1, 100);
    fauxmo.handle();
    if (shutters.isWrap()){
      shutters.wrap();
    }
    
  }

}
