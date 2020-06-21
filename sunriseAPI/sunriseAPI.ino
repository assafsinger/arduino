#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "/Users/assafs/Documents/Arduino/alexaEsp/TimeOfDay.h"

const char *ssid     = "finchouse";
const char *password = "6352938635293";

struct timeEvent{
  int hours;
  int minutes;
  int seconds;
  bool initialized = false;
};

struct timeEvent sunrise;
struct timeEvent sunset;

#define SUNRISE_SUNSET_API_URL   "http://api.sunrise-sunset.org/json?lat=32.0853&lng=34.7818"


const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup(){
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( ".\n" );
  }

  timeClient.begin();
}

void getSunrise(){
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;  //Object of class HTTPClient
    http.begin(SUNRISE_SUNSET_API_URL);
    int httpCode = http.GET();
    //Check the returning code                                                                  
    if (httpCode > 0) {
        // Get the request response payload
        String payload = http.getString();
        const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(10) + 340;
        DynamicJsonDocument doc(capacity);
        deserializeJson(doc, payload);
        JsonObject results = doc["results"];
        const char* results_sunrise = results["sunrise"]; // "2:58:41 AM"
        const char* results_sunset = results["sunset"]; // "4:18:22 PM"

        char sunriseCpy[15];
        char sunsetCpy[15];

        strcpy(sunriseCpy,results_sunrise);
        strcpy(sunsetCpy,results_sunset);
      


        //split
        char * pch;
        pch = strtok (sunriseCpy,": ");
        if (pch != NULL) {
          sunrise.hours = atoi(pch);
          sunrise.initialized = true;
        }

        pch = strtok (NULL,": ");
        if (pch != NULL) {
          sunrise.minutes = atoi(pch);
        }

        pch = strtok (NULL,": ");
        if (pch != NULL) {
          sunrise.seconds = atoi(pch);
        }

        pch = strtok (NULL,": ");
        if (pch != NULL) {
         if(strcmp("PM", pch) == 0){
          sunrise.hours+=12;
         }
        }


        pch = strtok (sunsetCpy,": ");
        if (pch != NULL) {
          sunset.hours = atoi(pch);
          sunset.initialized = true;

        }
        pch = strtok (NULL,": ");
        if (pch != NULL) {
          sunset.minutes = atoi(pch);
        }

        pch = strtok (NULL,": ");
        if (pch != NULL) {
          sunset.seconds = atoi(pch);
        }

        pch = strtok (NULL,": ");
        if (pch != NULL) {
         if(strcmp("PM", pch) == 0){
          sunset.hours+=12;
         }
        }


        
        Serial.printf("Sunrise:%s, Sunset:%s\n", results_sunrise, results_sunset);
        Serial.printf("Contents of structure Sunset are %d, %d, %d\n", sunset.hours, sunset.minutes, sunset.seconds);
        
      } else {
        Serial.printf("Error getting sunrise API. Error: %d\n",httpCode);
      }
     http.end();   //Close connection
  } else {
    Serial.println("Wifi not connected");
  }
 }


void loop() {
  timeClient.update();
  getSunrise();

  Serial.print(daysOfTheWeek[timeClient.getDay()]);
  Serial.print(", ");
  Serial.print(timeClient.getHours());
  Serial.print(":");
  Serial.print(timeClient.getMinutes());
  Serial.print(":");
  Serial.println(timeClient.getSeconds());

  long currentTimeSinceDayStart = timeClient.getSeconds()+timeClient.getMinutes()*60+timeClient.getHours()*3600;
  long sunsetTimeSinceDayStart = sunset.seconds+sunset.minutes*60+sunset.hours*3600;
  long sunriseTimeSinceDayStart = sunrise.seconds+sunrise.minutes*60+sunrise.hours*3600;

  long timeToSunset = sunsetTimeSinceDayStart - currentTimeSinceDayStart;
  long timeToSunrise = sunriseTimeSinceDayStart - currentTimeSinceDayStart;

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
