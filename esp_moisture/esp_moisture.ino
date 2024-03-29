
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


const char* wifi_ssid = "finchouse";             // SSID
const char* wifi_password = "6352938635293";         // WIFI
const char* apiKeyIn = "T15DMCYOWL9443CR";      // API KEY IN
const unsigned long READ_TIMEOUT_MS = 10000;

// ASKSENSORS config.
String host = "http://api.thingspeak.com";         // thingspeak API host name

ESP8266WiFiMulti WiFiMulti;

int numOfSensors=3;

#define DONE_PINE D1 //D1 or D2 are the safe pins. See https://rabbithole.wwwdotorg.org/2017/03/28/esp8266-gpio.html
unsigned long startTime = 0;

#define DEBUG false



void setup() {
  pinMode(DONE_PINE, OUTPUT);
  digitalWrite(DONE_PINE, LOW);
  if (DEBUG){
    pinMode(LED_BUILTIN, OUTPUT);
    blinkLED(10,false);
  }
  startTime = millis();
  Serial.begin(115200);
  swSer.begin(38400);
  Serial.println("*****************************************************");
  Serial.println("********** Program Start : Soil Moisture monitoring using ESP8266 and AskSensors IoT cloud");
  Serial.println("Wait for WiFi... ");
  Serial.print("********** connecting to WIFI : ");
  Serial.println(wifi_ssid);
  delay(500);
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("-> WiFi connected");
  Serial.println("-> IP address: ");
  Serial.println(WiFi.localIP());
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

void blinkLED (int times, bool longPulse){
  int delayTime = longPulse?200:50;
  for (int i=0;i<times;i++){
     digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
     delay(delayTime);                      // Wait for a second
     digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
     delay(delayTime);                      // Wait for two seconds (to demonstrate the active low LED)
  }
}

float getBatteryLevel(){
  // read the analog in value
  long sensorValue = 0;
  for (int i=0;i<30;i++){
    sensorValue += analogRead(A0);
    delay(5);
  }

  return 4.2*(sensorValue/30)/1023;
  //return 4.2*sensorValue/1023
  // map it to the range of the PWM out
  //int outputValue = map(sensorValue, 0, 1023, 0, 420);
  //return outputValue/100.0;
}

void signalDone(){
  while (1) {
    digitalWrite(DONE_PINE, HIGH);
    delay(1);
    digitalWrite(DONE_PINE, LOW);
    delay(1);
  }
}



int getBattryPercentage(float fVoltage){
  float fVoltageMatrix[24][2] = {
    {4.2,  100},
    {4.15, 99},
    {4.11, 97},
    {4.08, 95},
    {4.02, 90},
    {3.98, 85},
    {3.95, 80},
    {3.91, 75},
    {3.87, 70},
    {3.85, 65},
    {3.84, 60},
    {3.82, 55},
    {3.80, 50},
    {3.79, 45},
    {3.77, 40},
    {3.75, 35},
    {3.73, 30},
    {3.71, 25},
    {3.69, 20},
    {3.61, 15},
    {3.54, 10},
    {3.45, 5},
    {3.27, 0},
    {0, 0}
  };

  int i, perc;

  perc = 100;

  for(i=20; i>=0; i--) {
    if(fVoltageMatrix[i][0] >= fVoltage) {
      perc = fVoltageMatrix[i + 1][1];
      break;
    }
  }
  return perc;
}

float roundf(float var) 
{ 
    // 37.66666 * 100 =3766.66 
    // 3766.66 + .5 =3767.16    for rounding off value 
    // then type cast to int so value is 3767 
    // then divided by 100 so the value converted into 37.67 
    float value = (int)(var * 100 + .5); 
    return (float)value / 100; 
} 

int getReadingAndValidate(int port, int retryCount=5){
  int reading = 0;
  for (int i=0;i<retryCount;i++){
    reading = getReadingSerial(port);
    if (reading){
      return reading;
    }
    delay(50); 
  }
}

void loop() {

    if (DEBUG){
      pinMode(LED_BUILTIN, OUTPUT);
      blinkLED(5,true);
    }
    // wait for WiFi connection
  if (WiFi.status() == WL_CONNECTED){
        //get sensor data
        int sensorData[numOfSensors];
        for (int i=0 ; i<numOfSensors ; i++){
          sensorData[i] = getReadingAndValidate(i);
        }
        delay (1000);
        float temp = getReadingAndValidate(8)/10.0;
        float pressure = getReadingAndValidate(4)/100.0;
        delay (1000);
        float humidity = getReadingAndValidate(9)/10.0;
        float tempV2 = getReadingAndValidate(5)/10.0;
       // float battrtyVoltage = getReadingAndValidate(3)/100.0;
       float battrtyVoltage = getBatteryLevel();

        
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
        Serial.printf("Battery level is %.2fV and %d%%\n", battrtyVoltage, getBattryPercentage(battrtyVoltage));



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

        //third field is battery
        url += "&field3=";
        url += roundf(battrtyVoltage);

        for (int i=0 ; i<numOfSensors ; i++){
          url += "&field";
          url += i+4; //offset for two first occupied fields
          url += '=';
          url += sensorData[i];
        }

        //pressure and second temp
        url += "&field7=";
        url += pressure;
        url += "&field8=";
        url += tempV2;

        
        
        Serial.print("********** requesting URL: ");
        Serial.println(url);
        http.begin(url); //HTTP
        
        Serial.println("> Soil moisture level and state were sent to thingspeak");

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        if (DEBUG){
          Serial.println("Debug mode. skipping sending the data");
        } else {
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
        }

        http.end();
        float runtimeSeconds = (millis() - startTime)/1000.0;
        Serial.printf("program runtime: %.2f\n",runtimeSeconds);
        Serial.println("********** End ");
        Serial.println("*****************************************************");
    }
    if (DEBUG){
      pinMode(LED_BUILTIN, OUTPUT);
      blinkLED(2,true);
    }

    //send the done signal so tpl5110 can put us back to sleep
    signalDone();
}
