
#include <stdio.h>
#include <TimeLib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <Wire.h>  // Comes with Arduino IDE
#include <LiquidCrystal_I2C.h>
#include "ESP8266.h"
#include "DHT.h"


#define SSID        "finchouse1"
#define PASSWORD    "6352938635293"
//#define SSID        "Ozer"
//#define PASSWORD    "036353838"
#define HOST_NAME   "www.currentmillis.com"
#define HOST_PORT   (80)
#define DHTPIN 9     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321


SoftwareSerial wifiSerial(2, 3); // RX, TX
ESP8266 wifi(wifiSerial);
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
DHT dht(DHTPIN, DHTTYPE);
uint8_t buffer[128] = {0};
int buffer_length = 128;

int firstRun=1;


void setup(void)
{


    Serial.begin(9600);
//    Serial.print("setup begin\r\n");
    lcd.begin(20,4);         // initialize the lcd for 20 chars 4 lines and turn on backlight

      // ------- Quick 3 blinks of backlight  -------------
    for(int i = 0; i< 3; i++)
    {
      lcd.backlight();
      delay(250);
      lcd.noBacklight();
      delay(250);
    }
    lcd.backlight(); // finish with backlight on  

    lcd.home();
    lcd.print("Welcome!"); 
    lcd.setCursor(0,1);
    delay(500);
    lcd.print("connecting to wifi:");
    lcd.setCursor(0,2);
    lcd.print(SSID);
    lcd.setCursor(0,3);


//    Serial.print("FW Version:");
//    Serial.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
//        Serial.print("to station + softap ok\r\n");
    } else {
//        Serial.print("to station + softap err\r\n");
    }

    delay(500);

    if (wifi.joinAP(SSID, PASSWORD)) {
        Serial.println("Join AP success");
        lcd.print("Connected!");
        lcd.clear();
        lcd.home();
        lcd.print("Connected to SSID:");
        lcd.setCursor(0,1);
        lcd.print(SSID);
        lcd.setCursor(0,2);
//        lcd.print("Assigned IP adress:");
        lcd.setCursor(0,3);
        lcd.print(wifi.getLocalIP().c_str());
        Serial.print("IP:");
        Serial.println(wifi.getLocalIP().c_str());       
    } else {
//        Serial.print("Join AP failure\r\n");
//        lcd.print("Join AP failure");
    }
    
    if (wifi.enableMUX()) {
        Serial.println("multy ok");
    } else {
        Serial.println("multy err");
    }
    
    Serial.println("setup end");


    //get and set system time
    char* lastLine = getLastLine(HOST_NAME, HOST_PORT, "GET /time/seconds-since-unix-epoch.php HTTP/1.1 \r\nHost: www.currentmillis.com\r\n\r\n", 464);
    char *ptr;
    long int res = strtol(lastLine, &ptr, 10);
//    Serial.print("long converted string is");
    Serial.println(res);
    //Time zone - UTC+2
    setTime(res + 2*60*60);

     
        lcd.clear();
        lcd.setCursor(0,2);
        lcd.print("  Loading  Data...  ");
      dht.begin();
}

char* lastLine = "";
int lastTry = 0;
void loop(void)
{

 
  //read ewather info once every 5 minutes
  if (minute() % 5 == 0 || firstRun){
      char *lastFetched = "";
      int tries = 0;
      while(strstr(lastFetched,"*C") == NULL) {
        //update clock;
       if (++tries == 10){
          lcd.setCursor(0,3);
          lcd.print("Missing weather data");
        delay(1000);
        break;
       }
      lastFetched = getLastLine("10.0.0.101", 8080, "GET /getWeather.html HTTP/1.1 \r\nHost: 10.0.0.101\r\n\r\n",200);
     }
     if (strstr(lastFetched,"*C")){
      lastLine = lastFetched;
     }
  }
  

  if (second() % 60 == 0 || firstRun){
    lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(day());
    lcd.print(" ");
    lcd.print(monthStr(month()));
    lcd.print(" ");
    lcd.print(year()); 

  }
            
    lcd.home();
    lcd.print(hour());
    printDigits(minute());
    printDigits(second());
    lcd.print(" ");
    lcd.print(dayStr(weekday()));
   

    if (second()/30 == 0){
          if (strstr(lastLine,"*C") != NULL){
            lcd.setCursor(0,3);
            lcd.print(lastLine);
          } else {
            Serial.println("No weather data");
          }
          

    } else {
      //get tempratures:
      // Reading temperature or humidity takes about 250 milliseconds!
      // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      lcd.setCursor(0,3);
      if (isnan(h) || isnan(t)) {
        //lcd.print("Temprature read err ");
      } else {
        lcd.print("T:");
        lcd.print(t);
        lcd.print("*C   ");
        lcd.print("H:");
        lcd.print(h);
        lcd.print("%");
      }
    }
  firstRun=0;
  delay(1000);
}


void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if(digits < 10)
    lcd.print('0');
  lcd.print(digits);
}

char* getLastLine(char* host_name, int port, char* get_str, int dataStartIdx){
    static uint8_t mux_id = 0; 
    Serial.print("crearting TCP to ");
    Serial.print(host_name);
    Serial.print(":");
    Serial.println(port);
    if (wifi.createTCP(mux_id, host_name, port)) {
        Serial.print("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    wifi.send(mux_id, (const uint8_t*)get_str, strlen(get_str));

    uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 3000, dataStartIdx);
    if (len>buffer_length){
      buffer[buffer_length-1] = 0;
    }
    Serial.print("buffer:");
    Serial.println((char*)buffer);
    char* line = strtok((char*)buffer, "\n");
    char* lastLine;
    while(line)
    {
        Serial.print(line);
        lastLine=line;
        line = strtok(NULL, "\n");
    }
    Serial.print("string value: ");
    Serial.println(lastLine);

    if (wifi.releaseTCP(mux_id)) {
          Serial.println("release tcp ok");
      } else {
      }

      
    return lastLine;
}


