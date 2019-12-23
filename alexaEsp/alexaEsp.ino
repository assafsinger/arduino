#include "debug.h"              // Serial debugger printing
#include "WifiConnection.h"     // Wifi connection 
#include <fauxmoESP.h>
#include "Shutters.h"


WifiConnection* wifi;           // wifi connection

//SET YOUR WIFI CREDS 
const char* myWifiSsid      = "finchouse_ext"; 
const char* myWifiPassword  = "6352938635293";

//SET TO MATCH YOUR HARDWARE 
#define SERIAL_BAUD_RATE    115200

#define RADIO_PIN              D1

fauxmoESP fauxmo;
Shutters shutters(RADIO_PIN);
const char * SHUTTERS_MAME = "bedroom shutters";

// ************************************************************************************
//Runs once, when device is powered on or code has just been flashed 
//
void setup()
{
  //if set wrong, your serial debugger will not be readable 
  Serial.begin(SERIAL_BAUD_RATE);

  //initialize wifi connection 
  wifi = new WifiConnection(myWifiSsid, myWifiPassword); 
  wifi->begin(); 

  //connect to wifi 
  if (wifi->connect())
  {
    fauxmo.addDevice(SHUTTERS_MAME);
    fauxmo.setPort(80); // required for gen3 devices
    fauxmo.enable(true);
    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
        if (strcmp(SHUTTERS_MAME,device_name) == 0){
          if (state){
              Serial.print("Setting shutters ");
              short openPercentage = value*100/255;
              shutters.setShutterState(openPercentage);
              Serial.printf(" %d\% UP\n",openPercentage);
          } else {
              shutters.closeShutter();
              Serial.println("DOWN");

          }
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
  if (wifi->isConnected)
  {
    //blinkLed(1, 100);
    fauxmo.handle();
    if (shutters.isWrap()){
      shutters.wrap();
    }
    
  }

}
