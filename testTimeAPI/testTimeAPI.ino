#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "/Users/assafs/Documents/Arduino/alexaEsp/TimeOfDay.h"

const char *ssid     = "finchouse";
const char *password = "6352938635293";

TimeOfDay timeOfDay;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( ".\n" );
  }
}

void loop() {
  long timeToSunset = timeOfDay.timeToFromSunset();
  long timeToSunrise = timeOfDay.timeToFromSunrise();
  
  if (timeToSunset > 0) { //sunset in the future
    Serial.printf("Sunset will be in %dH:%dM%dS\n", timeToSunset/3600,(timeToSunset%3600)/60,timeToSunset%3600%60);
  } else {//sunset in the past
    Serial.printf("Sunset was %dH:%dM%dS ago\n", timeToSunset/3600,(timeToSunset%3600)/60,timeToSunset%3600%60);
  }

  if (timeToSunrise > 0) { //sunrise in the future
    Serial.printf("sunrise will be in %dH:%dM%dS\n", timeToSunrise/3600,(timeToSunrise%3600)/60,timeToSunrise%3600%60);
  } else {//sunrise in the past
    Serial.printf("sunrise was %dH:%dM%dS ago\n", timeToSunrise/3600,(timeToSunrise%3600)/60,timeToSunrise%3600%60);
  }


  //Serial.println(timeClient.getFormattedTime());

  delay(60*1000);
}
