#include <ESP8266WiFi.h>
#include <RCSwitch.h>
#include "Gsender.h"

#pragma region Globals
const char* ssid = "singer-family";                           // WIFI network name
const char* password = "036352938";                       // WIFI network password
uint8_t connection_state = 0;                    // Connected to WIFI or not
uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
#pragma endregion Globals

//GPIO bug issue: http://www.esp8266.com/viewtopic.php?f=32&t=11492
//static const uint8_t D0   = 16;
//static const uint8_t D1   = 5;
//static const uint8_t D2   = 4;
//static const uint8_t D3   = 0;
//static const uint8_t D4   = 2;
//static const uint8_t D5   = 14;
//static const uint8_t D6   = 12;
//static const uint8_t D7   = 13;
//static const uint8_t D8   = 15;
//static const uint8_t D9   = 3;
//static const uint8_t D10  = 1;

RCSwitch mySwitch = RCSwitch();

uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    if(nSSID) {
        WiFi.begin(nSSID, nPassword);  
        Serial.println(nSSID);
    } else {
        WiFi.begin(ssid, password);
        Serial.println(ssid);
    }

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

void Awaits()
{
    uint32_t ts = millis();
    while(!connection_state)
    {
        delay(50);
        if(millis() > (ts + reconnect_interval) && !connection_state){
            connection_state = WiFiConnect();
            ts = millis();
        }
    }
}

void sendNotification(){
    Serial.println("Notifying...");
    if (WiFi.status()!= WL_CONNECTED){
      Serial.println("Not connected. Reconnecting");
      connection_state = WiFiConnect();
    }
    if(!connection_state)  // if not connected to WIFI
        Awaits();          // constantly trying to connect

    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
    String subject = "Door open notification";
    if(gsender->Subject(subject)->Send("singerfam@googlegroups.com", "This email forms a notification for the opening of the house door.")) {
        Serial.println("Message sent.");
    } else {
        Serial.print("Error sending message: ");
        Serial.println(gsender->getError());
    }
}

void setup()
{
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);

    mySwitch.enableReceive(D1);  // Receiver on interrupt 0 => that is pin #2
    connection_state = WiFiConnect();
}


void loop() {
  static unsigned long lastNotify = 0;
  
  if (mySwitch.available()) {
    
    int value = mySwitch.getReceivedValue();
    
    if (value == 0) {
      Serial.print("Unknown encoding");
    } else {
      Serial.print("Received ");
      Serial.print( mySwitch.getReceivedValue() );
      Serial.print(" / ");
      Serial.print( mySwitch.getReceivedBitlength() );
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println( mySwitch.getReceivedProtocol() );
//      if (15578921 == mySwitch.getReceivedValue()){
      if (15663273 == mySwitch.getReceivedValue()){
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("ON");
      if (millis() - lastNotify > 5000){
        sendNotification();
      }
      lastNotify = millis(); 
      }
    }
  }
    
  mySwitch.resetAvailable();
  if (millis() - lastNotify > 5000){
      digitalWrite(LED_BUILTIN, HIGH);
    } 
}
