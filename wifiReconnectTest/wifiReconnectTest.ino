#include <stdio.h>
#include "ESP8266.h"
#define SSID        "finchouse1"
#define PASSWORD    "6352938635293"
#define HOST_NAME   "10.0.0.101"
#define HOST_PORT   (8080)
#define CHPD_PIN    9

// send results once every 30 minutes
// sample once a minute
// send the first sample right away
// 5 next one minute away from each other
// get to normal pace

SoftwareSerial wifiSerial(2, 3); // RX, TX
ESP8266 wifi(wifiSerial);

String GET = "GET /logWattageEntry.php";
static uint8_t mux_id = 0; 
uint8_t buffer[36] = {0};
int buffer_length = 36;

int onboardLED = 13;

void setup()
{  
  blink(5);
// configure the watchdog
  Serial.begin(9600);
  pinMode(onboardLED, OUTPUT);     
//  digitalWrite(CHPD_PIN, HIGH);
//  bool connected = connectToWifi();
//  digitalWrite(CHPD_PIN, LOW);
}

bool connectToWifi(){
    delay(1000);
    Serial.println(F("Connecting to Wifi"));
    if (wifi.setOprToStationSoftAP()) {
    } else {
        Serial.println(F("to station + softap err"));
        return 0;
    }
    if (wifi.joinAP(SSID, PASSWORD)) {
    } else {
        Serial.print(F("Join AP failure\r\n"));
        return 0;
    }

        if (wifi.enableMUX()) {
    } else {
        Serial.println(F("multy err"));
        return 0;
    }
    
    Serial.println(F("setup end"));
    return 1;
}

void updateSensorData(String data, unsigned long referrenceTime){
  String GET_REQUEST = GET + "?rt=" + referrenceTime + "&w=" + data +"\r\n\r\n";
  updateData(HOST_NAME, HOST_PORT, &GET_REQUEST[0]);
}

void updateData(char* host_name, int port, char *get_str){
    Serial.print(F("crearting TCP to "));
    Serial.print(host_name);
    Serial.print(":");
    Serial.println(port);
    if (wifi.createTCP(mux_id, host_name, port)) {
    } else {
        Serial.print(F("create tcp err\r\n"));
    }

    wifi.send(mux_id, (const uint8_t*)get_str, strlen(get_str));

    uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 3000, 0);
    if (len>buffer_length){
      buffer[buffer_length-1] = 0;
    }
    
    if (wifi.releaseTCP(mux_id)) {
      } else {
      }

      
}


void blink(int times){
  for (int i=0; i<times;i++){
    digitalWrite(onboardLED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(onboardLED, LOW);    // turn the LED off by making the voltage LOW
    delay(100); 
  }
}


void loop()
{
  digitalWrite(CHPD_PIN, HIGH);
//  bool connected = connectToWifi();
  Serial.print("IP: ");       
  Serial.println(wifi.getLocalIP().c_str());
  digitalWrite(CHPD_PIN, LOW);
  for (int i = 1;i<=10;i++){
    Serial.println(i);
    delay(10000);
    digitalWrite(CHPD_PIN, HIGH);
    unsigned long time = millis();
//    bool connected = connectToWifi();
    Serial.print((millis()-time)/1000);
    Serial.println(" S");

    delay(6000);
    Serial.print("IP: ");       
    Serial.println(wifi.getIPStatus().c_str());
    Serial.println(wifi.getLocalIP().c_str());

    Serial.println();
    digitalWrite(CHPD_PIN, LOW);
  }
}

