#include "DHT.h"
#include "ESP8266.h"

// Set DEBUG to 1 for lots of lovely debug output
//
#define DEBUG_AS  0

//------------------------------------------------------------------------------
// Debug directives
//
#if DEBUG_AS
#  define PRINT_AS(...)    Serial.print(__VA_ARGS__)
# define PRINT_ASLN(...)  Serial.println(__VA_ARGS__)
#else
# define PRINT_AS(...)
# define PRINT_ASLN(...)
#endif

//AC
#define ACOFFPIN 6
#define ACHEATPIN  7
#define ACCOOLNPIN  8


//DHT
#define DHTPIN 5
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//PHOTOCELL
#define PHOTOCELLPIN 7

//BUZZER
#define BUZZERPIN 13 // setting controls the digital IO foot buzzer

//thingspeak.com
//#define APIKEY1      "6FEX2MDKMCMHWRRM"
//#define APIKEY2      "1SH04KRGFTNZ86US"
#define HOST_NAME   "api.thingspeak.com"
#define HOST_PORT   (80)
#define SSID        "finchouse1"
#define PASSWORD    "6352938635293"
uint8_t buffer[128] = {0};
int buffer_length = 128;
static uint8_t mux_id = 0; 

String GET = "GET /update?key=6FEX2MDKMCMHWRRM";


    
DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial wifiSerial(4, 3); // RX, TX
ESP8266 wifi(wifiSerial);

struct DATA {
  String temp;
  String humidity;
  String light;
} data;

typedef enum ac_mode { OFF, HEAT, COOL};


void setup() {
  Serial.begin(9600);
  pinMode(ACOFFPIN, OUTPUT);
  pinMode(ACHEATPIN, OUTPUT);
  pinMode(ACCOOLNPIN, OUTPUT);
  pinMode(ACCOOLNPIN, OUTPUT);



  Serial.println("Station 1");
  bool connected = connectToWifi();
  startServer();
  pinMode (BUZZERPIN, OUTPUT) ;
  dht.begin();
}

long lastSensorRun = 0;

void loop() {  
  //run every 1 minute
  long iterationLength=5*60L*1000;
  if (millis() - lastSensorRun > iterationLength || lastSensorRun==0){
    lastSensorRun = millis();
    //stopServer();
    Serial.println("running sensors");
      readTemprature();
      photocellReading();
      updateSensorData();
    //startServer();
    Serial.print("Actual time taken:");
    Serial.println(millis()-lastSensorRun);
  }

  receiveServer();

}

bool connectToWifi(){
    delay(1000);
    Serial.println("Connecting to Wifi");
    if (wifi.setOprToStationSoftAP()) {
    } else {
        Serial.println("to station + softap err");
        return 0;
    }
    if (wifi.joinAP(SSID, PASSWORD)) {
        PRINT_ASLN("Join AP success");
    } else {
        Serial.print("Join AP failure\r\n");
        return 0;
    }

        if (wifi.enableMUX()) {
        PRINT_ASLN("multy ok");
    } else {
        Serial.println("multy err");
        return 0;
    }
    
    Serial.println("setup end");
    return 1;
}

void startServer(){
  if (wifi.startTCPServer(80)) {
        Serial.println("server ok");
    } else {
        Serial.println("server err");
    }
    
    if (wifi.setTCPServerTimeout(10)) { 
    } else {
        Serial.print("timout err");
    }
}

void stopServer(){
  if (wifi.stopTCPServer()) {
        Serial.println("stop server ok");
    } else {
        Serial.println("stop server err");
    }
}

void soundAlarm(int iterations){
  unsigned int i, j ;// define variables
  int iter;
  for (iter = 0; iter<iterations*10; iter++)
  {
    for (i = 0; i <300; i++) // Wen a frequency sound
    {
      digitalWrite (BUZZERPIN, HIGH) ;// send voice
      delayMicroseconds (i) ;// Delay 1ms
      digitalWrite (BUZZERPIN, LOW) ;// do not send voice
      delayMicroseconds (i) ;// delay ms
//      Serial.print(iter);
//      Serial.print("\t");
//      Serial.println(i);
    }
    
//    for (i = 8000; i >0; i--) // Wen Qie out another frequency sound
//    {
//      digitalWrite (BUZZERPIN, HIGH) ;// send voice
//      delayMicroseconds (i) ;// delay 2ms
//      digitalWrite (BUZZERPIN, LOW) ;// do not send voice
//      delayMicroseconds (i) ;// delay 2ms
//    }
  }
}

void photocellReading(){
  int photocellReading = analogRead(PHOTOCELLPIN); 
  data.light = photocellReading;
//  String GET_REQUEST_LIGHT = GET + "&field3=" + String(photocellReading)+"\r\n\r\n";
//  updateData(HOST_NAME, HOST_PORT, &GET_REQUEST_LIGHT[0]);
  PRINT_ASLN("photocellReading = ");
  PRINT_ASLN(photocellReading);     // the raw analog reading
}

void readTemprature(){

  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    PRINT_ASLN("Failed to read from DHT sensor!");
    return;
  }

  char buffer[10];
  String tempStr = dtostrf(t, 4, 1, buffer);
  String humiditystr = dtostrf(h, 4, 1, buffer);
  data.temp = tempStr;
  data.humidity = humiditystr;
  //String GET_REQUEST_TEMP = GET + "&field1="+tempStr+"\r\n\r\n";
  //updateData(HOST_NAME, HOST_PORT, &GET_REQUEST_TEMP[0]);
  
//  String GET_REQUEST_HUM = GET + "&field2=" + humiditystr+"\r\n\r\n";
//  updateData(HOST_NAME, HOST_PORT, &GET_REQUEST_HUM[0]);
  
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
}

void updateSensorData(){
  String GET_REQUEST = GET + "&field1=" + data.temp +"&field2=" + data.humidity+"&field3=" + data.light+"\r\n\r\n";
  updateData(HOST_NAME, HOST_PORT, &GET_REQUEST[0]);
}

void receiveServer(){
      uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 100, 0);
      if (len > 0) {
        for(uint32_t i = 0; i < len; i++) {
            Serial.print((char)buffer[i]);
        }
        wifi.send(mux_id, buffer, len);
        wifi.releaseTCP(mux_id);
        Serial.print("release");
        if(strstr((char*)buffer, "op=acHEAT")){
          acSend(HEAT);
        }
        if(strstr((char*)buffer, "op=acCOOL")){
          acSend(COOL);
        }
        if(strstr((char*)buffer, "op=acOFF")){
          acSend(OFF);
        }      
      }
}

void acSend(ac_mode mode){
  Serial.print("got signal:");
  Serial.println(mode);
  int pin = ACOFFPIN;
  switch(mode) {
     case OFF     : pin = ACOFFPIN; break;
     case HEAT    : pin = ACHEATPIN; break;
     case COOL    : pin = ACCOOLNPIN; break;
     default      :return;
  }
     digitalWrite(pin, HIGH);
     Serial.print("Pin:");
     Serial.println(pin);
     delay(1000);
     digitalWrite(pin, LOW);
}


void updateData(char* host_name, int port, char *get_str){
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

    uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 3000, 0);
    if (len>buffer_length){
      buffer[buffer_length-1] = 0;
    }
    Serial.print("buffer:");
    Serial.println((char*)buffer);
    
    if (wifi.releaseTCP(mux_id)) {
          Serial.println("release tcp ok");
      } else {
      }

      
}

