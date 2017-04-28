#include "EmonLib.h"                   // Include Emon Library
#include <stdio.h>
#include "ESP8266.h"

#define SSID        "finchouse1"
#define PASSWORD    "6352938635293"
#define HOST_NAME   "10.0.0.101"
#define HOST_PORT   (8080)
#define CHPD_PIN    9

#if DEBUG_AS
#  define PRINT_AS(...)    Serial.print(__VA_ARGS__)
# define PRINT_ASLN(...)  Serial.println(__VA_ARGS__)
#else
# define PRINT_AS(...)
# define PRINT_ASLN(...)
#endif

EnergyMonitor emon1;                  
SoftwareSerial wifiSerial(2, 3); // RX, TX
ESP8266 wifi(wifiSerial);

String GET = "GET /logWattage.php";
static uint8_t mux_id = 0; 
uint8_t buffer[128] = {0};
int buffer_length = 128;
double measurements[10] = {0};
int measurements_size = 10;


 
void setup()
{  
  Serial.begin(9600);
  pinMode(CHPD_PIN, OUTPUT); //esp01 chip core
  emon1.current(0, 90);             // Current: input pin, calibration.
//  bool connected = connectToWifi();

  
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

void updateSensorData(String data){
  String GET_REQUEST = GET + "?w=" + data +"\r\n\r\n";
  updateData(HOST_NAME, HOST_PORT, &GET_REQUEST[0]);
}

void updateData(char* host_name, int port, char *get_str){
    Serial.print("crearting TCP to ");
    Serial.print(host_name);
    Serial.print(":");
    Serial.println(port);
    if (wifi.createTCP(mux_id, host_name, port)) {
        PRINT_ASLN("create tcp ok\r\n");
    } else {
        Serial.print("create tcp err\r\n");
    }

    wifi.send(mux_id, (const uint8_t*)get_str, strlen(get_str));

    uint32_t len = wifi.recv(mux_id, buffer, sizeof(buffer), 3000, 0);
    if (len>buffer_length){
      buffer[buffer_length-1] = 0;
    }
    PRINT_AS("buffer:");
    PRINT_ASLN((char*)buffer);
    
    if (wifi.releaseTCP(mux_id)) {
          PRINT_ASLN("release tcp ok");
      } else {
      }

      
}

void send_measurements(){
  digitalWrite(CHPD_PIN, HIGH);
  connectToWifi();
  String str = String("");
  for (int i = 0 ; i< measurements_size; i++){
    Serial.print("sending measurement ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(measurements[i]);
    if (i!=0){
      str +=String(",");
    }
    str += String(measurements[i]);    
  }
  updateSensorData(str);

//  Serial.println(str);
  digitalWrite(CHPD_PIN, LOW);
  delay(500);
}




void loop()
{
  static int measurements_count = 0;
  if (measurements_count == measurements_size){
    send_measurements();
    measurements_count = 0;
  }

  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  measurements[measurements_count] = Irms*220.0; 
  Serial.print(Irms*220.0);         // Apparent power
  Serial.print(" ");
  Serial.println(Irms);          // Irms
  measurements_count++;
  delay(10000);
}
  

  
//  static int i = 0;
//  static bool connected = 0;
//  int iterations = 100;
//  if (i>=iterations){
//    i = 0;
//  }
//  if (i==0){
//     digitalWrite(CHPD_PIN, HIGH);
//     connected = connectToWifi();
//     delay(100);
//  }
//  if (i==iterations/2){
//    digitalWrite(CHPD_PIN, LOW);
//
////    wifi.leaveAP();
//    connected = 0;
//    delay(100);
//  }
//  digitalWrite(CHPD_PIN, LOW);
//  delay(1000);
//  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
////  Serial.print("Connection state ");
////  Serial.print(connected);
////  Serial.print(", ");
//  Serial.print(Irms*220.0);         // Apparent power
//  Serial.print(" ");
//  Serial.println(Irms);          // Irms
//  //i++;
//  digitalWrite(CHPD_PIN, HIGH);
//  connectToWifi();
//  updateSensorData(Irms*220.0);
// //delay(1000);
//}


