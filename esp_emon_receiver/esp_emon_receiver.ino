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

//RH_ASK driver;
RH_ASK driver(2000, 2, 4, 5); // ESP8266: do not use pin 11

const char* ssid = "finchouse1";
const char* password = "6352938635293";

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
    }
    if (strcmp(type, "PH2") == 0){
      emon.phase2 = atof(data);
      emon.phase2Update = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
    }    
    if (strcmp(type, "PH3") == 0){
      emon.phase3 = atof(data);
      emon.phase3Update = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
    }
    if (strcmp(type, "BAT") == 0){
      emon.battery = atof(data);
      emon.battUpdate = millis();
      emon.oldestUpdate = MIN(MIN(emon.phase1Update, emon.phase2Update),MIN(emon.phase3Update, emon.battUpdate));
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
      sendData();
      lastSent = millis();
    }
}
