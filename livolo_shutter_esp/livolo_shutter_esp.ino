#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <ArduinoJson.h>
#include <LivoloTx.h>


#define MICROSEC              1000000L
#define MILLISEC              1000L
#define SEC                   1L
#define MINUTE                (unsigned int) 60L*SEC
#define HOUR                  (unsigned int) 60L*MINUTE
#define DAY                   (unsigned long) 24L*HOUR

/* 
 * ======================================
 *      User defined constants
 * ======================================
*/
#define DEFAULT_POLLING_RATE  1*MINUTE
#define SSID                  "finchouse_ext"
#define PASSWORD              "6352938635293"
#define SCRIPT_ID             "AKfycbzbdaMN0DswFp4gnN5S_WgS41d8Gxxpt9Ti2xiFC2mU0BH_EUQV"
//#define SCRIPT_ID             "AKfycbyrkpJz3-QnSgouqUqzetOBHeIZwHbWkPgTXPKpf7SO"
#define DEBUG                 1

#define PASSCODE              "ori"
// ======================================

//livolo
#define LIVOLO_REMOTE_ID    6400
#define LIVOLO_OPEN_BUTTON  96
#define LIVOLO_CLOSE_BUTTON 0

// Pins allocation
#define TX_PIN              D1

// HTTPS parameters
#define HTTPS_PORT            443
#define HOST                  "script.google.com"
#define URL                   "https://" HOST "/macros/s/" SCRIPT_ID "/exec"
#define SUCCESS               "\"status\":\"success\""
#define MAX_WIFI_ATTEMPTS     60
#define MAX_HTTPS_ATTEMPTS    5
#define HTTPS_REINTENT_DELAY  2*SEC

// Deep-sleep time limit
#define MAX_SLEEP_TIME        71L*MINUTE

// Global variables
//ESP8266WebServer server(8080);
HTTPSRedirect* client = NULL;
LivoloTx gLivolo(TX_PIN);
long _pollingRate = DEFAULT_POLLING_RATE;
int _attempts = 0;

void setup() {
#if DEBUG
  //initSerial
  Serial.begin(9600);
  Serial.println();
#endif
  //initIO()
  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);
  initWiFi();
}

void reportError(String errStr){
  #if DEBUG
    Serial.println(errStr);
  #endif
}

void loop() {
  if (_attempts <= MAX_WIFI_ATTEMPTS) {
    _attempts = 0;
    String response = httpsGet();
    if (response != "") {
      process(response);
    } else {
#if DEBUG
     Serial.println("Got empty response");
#endif

    }
  } else {
#if DEBUG
     Serial.println("Exceeded max wifi attempts");
#endif
  }
  sleep();
}

void process(String response) {
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, response);

  if (!error) {
    String status = doc["status"];
    if (status == "success") {
      //_pollingRate = root["data"]["nextCheckIn"];
      if (_pollingRate > MAX_SLEEP_TIME) _pollingRate = MAX_SLEEP_TIME;
      if (doc["data"]["openShutter"]) openShutter();
      if (doc["data"]["closeShutter"]) closeShutter();
    } else {
#if DEBUG
      Serial.print(F("Unsuccessful response: ")); Serial.println(status);
#endif
    }
  } else {
#if DEBUG
    Serial.println(F("Failed to parse JSON"));
#endif
  }
}

void sleep() {
#if DEBUG
  Serial.print(F("Go to sleep for "));
  Serial.print(_pollingRate);
  Serial.println(F("s."));
#endif  //DEBUG
  if (_pollingRate == 0) _pollingRate = 1;
  //ESP.deepSleep(_pollingRate * MICROSEC);
  delay(MILLISEC*_pollingRate);
}
