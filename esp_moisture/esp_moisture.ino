
/*
 * AskSensors IoT Platform. 
 * Description: Soil moisture monitoring using ESP8266 and the AskSensors IoT cloud.
 *  Author: https://asksensors.com, 2018 - 2019
 *  github: https://github.com/asksensors/AskSensors-ESP8266-Moisture
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>

SoftwareSerial swSer(14, 12, false, 128); //SoftwareSerial(rxPin, txPin, inverse_logic, buffer size);

// user config: TODO
#define  MOISTURE_THRESHOLD     55   // moisture alert threshold

const char* wifi_ssid = "finchouse";             // SSID
const char* wifi_password = "6352938635293";         // WIFI
const char* apiKeyIn = "T15DMCYOWL9443CR";      // API KEY IN
const unsigned int writeInterval = 30000; // write interval (in ms)
const unsigned long READ_TIMEOUT_MS = 10000;

// ASKSENSORS config.
String host = "http://api.thingspeak.com";         // thingspeak API host name

ESP8266WiFiMulti WiFiMulti;

int moisture_Pin= 0; // Soil Moisture Sensor input at Analog PIN A0
int numOfSensors=2;





void setup() {

  Serial.begin(115200);
  swSer.begin(38400);
  Serial.println("*****************************************************");
  Serial.println("********** Program Start : Soil Moisture monitoring using ESP8266 and AskSensors IoT cloud");
  Serial.println("Wait for WiFi... ");
  Serial.print("********** connecting to WIFI : ");
  Serial.println(wifi_ssid);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> WiFi connected");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());


  for (int i=0 ; i<numOfSensors ; i++){
      pinMode(sensorPins[i], OUTPUT); //initialize sensors
  }

 
}

int getReadingSerial(int sensorNumber){
  unsigned long startMs = millis();
  String data = "";
  boolean received=false;
  int moisture_value= 0;
  Serial.print("DATA LEVEL FOR SENSOR ");
  Serial.print(sensorNumber);
  Serial.print(":");
  swSer.print(sensorNumber);
//Serial.println(" writing for serial");
   while(!received){
//   Serial.println("!received");
      while (swSer.available() > 0) {  //wait for data at software serial
       char character = swSer.read();
//       Serial.print("got:");
//       Serial.print(character);
        if (character == '\n'){
          received = true;
//          Serial.println("breaking");
          break;
        }
        data += character;
      }
      if (received || (millis() - startMs> READ_TIMEOUT_MS)){
        Serial.println(data);
        Serial.print("received:");
        Serial.println(received);
        Serial.print("timeout:");
        Serial.println((millis() - startMs> READ_TIMEOUT_MS));
//        Serial.println("returning");
        return data.toInt();
      }
   }
}

void loop() {

    
    // wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED){
    
        //get sensor data
        int sensorData[numOfSensors];
        for (int i=0 ; i<numOfSensors ; i++){
          sensorData[i] = getReadingSerial(i);
        }
        delay (1000);
        float temp = getReadingSerial(8)/10.0;
        float pressure = getReadingSerial(4)/100.0;
        delay (1000);
        float humidity = getReadingSerial(9)/10.0;
        float tempV2 = getReadingSerial(5)/10.0;
        
        Serial.print("Temperature: ");
        Serial.print(temp);
        Serial.println("°C");
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.println("%");
        Serial.print("Pressure: ");
        Serial.print(pressure);
        Serial.println(" inHg");
        Serial.print("TemperatureV2: ");
        Serial.print(tempV2);
        Serial.println("°C");



        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        
        // Create a URL for the request
        //first two fields are temp, humidity and then the moisture
        String url = "";
        url += host;
        url += "/update?api_key=";
        url += apiKeyIn;

        //first two fields are temp, humidity and then the moisture
        url += "&field1=";
        url += temp;
        url += "&field2=";
        url += humidity;

        for (int i=0 ; i<numOfSensors ; i++){
          url += "&field";
          url += i+3; //offset for two first occupied fields
          url += '=';
          url += sensorData[i];
        }
        
        Serial.print("********** requesting URL: ");
        Serial.println(url);
        http.begin(url); //HTTP
        
        Serial.println("> Soil moisture level and state were sent to thingspeak");

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = -1;
        httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.println(payload);
            }
        } else {
            Serial.printf("[HTTP] GET... failed, %d error: %s.\n", httpCode, http.errorToString(httpCode).c_str());
        }

        http.end();

        Serial.println("********** End ");
        Serial.println("*****************************************************");
    }

    delay(writeInterval);
}
