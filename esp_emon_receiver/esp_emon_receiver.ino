// ask_receiver.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to receive messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) receiver with an Rx-B1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include<string.h>
#include<stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define DEBUG false

//RH_ASK driver;
RH_ASK driver(2000, 2, 4, 5); // ESP8266: do not use pin 11

const char* ssid = "finchouse";
const char* password = "6352938635293";

String host = "http://api.thingspeak.com";         // thingspeak API host name
const char* apiKeyIn = "TI828H1DL3NMQJRA";      // API KEY IN



struct emon {
   char  id[5];
   double  phase1;
   double  phase2;
   double  phase3;
   double  battery;
   unsigned long phase1Update;
   unsigned long phase2Update;
   unsigned long phase3Update;
   unsigned long battUpdate;
   unsigned long oldestUpdate;
   unsigned long newestUpdate;
} emon;  

void setup()
{
    Serial.begin(9600);	// Debugging only
    if (!driver.init())
         Serial.println("init failed");
     WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {       
      delay(1000);
      Serial.print("Connecting..");
  }
}

void tokenize(char* input){
    char *p = strtok(input, ":");
    char type[10];
    char data[30];

    //device id
    if (p != NULL) {
      Serial.println(p);
      p = strtok(NULL, ":");
    }

    //type
    if (p != NULL) {
      Serial.println(p);
      strcpy(type, p);
      p = strtok(NULL, ":");
    }

    //data
    if (p != NULL) {
      Serial.println(p);
      strcpy(data, p);
      p = strtok(NULL, ":");
    }

    if (strcmp(type, "PH1") == 0){
      emon.phase1 = atof(data);
      emon.phase1Update = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
      emon.newestUpdate = MAX(MAX(emon.phase1Update, emon.phase2Update),MAX(emon.phase3Update, emon.battUpdate));
    }
    if (strcmp(type, "PH2") == 0){
      emon.phase2 = atof(data);
      emon.phase2Update = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
      emon.newestUpdate = MAX(MAX(emon.phase1Update, emon.phase2Update),MAX(emon.phase3Update, emon.battUpdate));

    }    
    if (strcmp(type, "PH3") == 0){
      emon.phase3 = atof(data);
      emon.phase3Update = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
      emon.newestUpdate = MAX(MAX(emon.phase1Update, emon.phase2Update),MAX(emon.phase3Update, emon.battUpdate));
    }
    if (strcmp(type, "BAT") == 0){
      emon.battery = atof(data);
      emon.battUpdate = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
      emon.newestUpdate = MAX(MAX(emon.phase1Update, emon.phase2Update),MAX(emon.phase3Update, emon.battUpdate));
    }        
}

void printStatus(){
  Serial.println("---------------------");
  Serial.print("phase1:");
  Serial.println(emon.phase1);
  Serial.print("phase1 last update:");
  Serial.println((millis() - emon.phase1Update)/1000);
  Serial.print("phase2:");
  Serial.println(emon.phase2);
  Serial.print("phase2 last update:");
  Serial.println((millis() - emon.phase2Update)/1000);
  Serial.print("phase3:");
  Serial.println(emon.phase3);
  Serial.print("phase3 last update:");
  Serial.println((millis() - emon.phase3Update)/1000);
  Serial.println();
  Serial.print("battery:");
  Serial.println(emon.battery);
  Serial.print("battery last update:");
  Serial.println((millis() - emon.battUpdate)/1000);
  Serial.println();
  Serial.print("oldest update");
  Serial.println((millis() - emon.oldestUpdate)/1000);
  Serial.println("---------------------");
  
}

void sendDataThingspeak(){
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
          
  // Create a URL for the request
  String url = "";
  url += host;
  url += "/update?api_key=";
  url += apiKeyIn;

  url += "&field1=";
  url += emon.battery;
  
  int wattage  = emon.phase1 + emon.phase2 + emon.phase3;
  url += "&field2=";
  url += wattage;
  
  url += "&field3=";
  url += emon.phase1;

  url += "&field4=";
  url += emon.phase2;

  url += "&field5=";
  url += emon.phase3;

  url += "&field6=";
  url += (millis() - emon.newestUpdate)/1000;

  url += "&field7=";
  url += (millis() - emon.oldestUpdate)/1000;
    
  Serial.print("********** requesting URL: ");
  Serial.println(url);
  http.begin(url); //HTTP
  

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  if (DEBUG){
    Serial.println("Debug mode. skipping sending the data");
  } else {
    int httpCode = -1;
    httpCode = http.GET();
    Serial.println("> power levels and state were sent to thingspeak");
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
}

void sendData(){
    int wattage  = emon.phase1 + emon.phase2 + emon.phase3;

    String str = "http://singer.org.il:8080/logging.php";
    str += "?m_bt=" + String(emon.battery);
    str += "&d_bt=" + String(millis() - emon.battUpdate);
    str += "&m_wattage=" + String(wattage);
    str += "&d_wattage=" + String(millis() - emon.oldestUpdate);
    str += "&m_phase1=" + String(emon.phase1);
    str += "&d_phase1=" + String(millis() - emon.phase1Update);
    str += "&m_phase2=" + String(emon.phase2);
    str += "&d_phase2=" + String(millis() - emon.phase2Update);
    str += "&m_phase3=" + String(emon.phase3);
    str += "&d_phase3=" + String(millis() - emon.phase3Update);
    
    str += "&type=EMON&dId=1";


    Serial.println(str);

    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
        Serial.println("wifi connected");
        HTTPClient http;  //Declare an object of class HTTPClient
     
        http.begin(str);  //Specify request destination
        int httpCode = http.GET();                                                                  //Send the request
        Serial.print("got code:");
        Serial.println(httpCode);
        if (httpCode > 0) { //Check the returning code
     
          String payload = http.getString();   //Get the request response payload
          Serial.println(payload);                     //Print the response payload
     
        }
   
    http.end();   //Close connection
 
  }
}

void loop(){
    static unsigned long lastSent = millis();
    uint8_t buf[RH_ASK_MAX_MESSAGE_LEN];
    uint8_t buflen = sizeof(buf);

    if (driver.recv(buf, &buflen)){ // Non-blocking
      
    	// Message with a good checksum received, dump it.
    	//driver.printBuffer("Got:", buf, buflen);
      Serial.print("Got:");
      for (int i=0;i<buflen;i++){
        Serial.print((char)buf[i]);
      }
      Serial.println();
      buf[buflen] = 0;
      tokenize((char*)buf);
      printStatus();
      Serial.println();
    }

    if (millis() - lastSent >=1000*60){
      Serial.println("sending data");
      sendDataThingspeak();
      lastSent = millis();
    }
}
