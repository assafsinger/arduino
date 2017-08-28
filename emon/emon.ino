#include "EmonLib.h"                   // Include Emon Library
#include <stdio.h>
#include "ESP8266.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#define SSID        "finchouse1"
#define PASSWORD    "6352938635293"
#define HOST_NAME   "10.0.0.101"
#define HOST_PORT   (8080)
#define CHPD_PIN    9

// send results once every 30 minutes
// sample once a minute
// send the first sample right away
// 5 next one minute away from each other
// get to normal pace

#define MEASUREMENT_INTERVAL_S 30
//#define WIFI_UODATE_INTERVAL_M 15
#define MAX_TIME_BETWEEN_MEASUREMENTS_S 120
#define MAX_MEASUREMENT_DIFF 20 //in Watts
#define WIFI_UODATE_INTERVAL_M 20
//#define MEASUREMENTS_SIZE (int)(1.05*(60/MEASUREMENT_INTERVAL_S)*WIFI_UODATE_INTERVAL_M)
#define MEASUREMENTS_SIZE 10


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
bool saveEnergy = 1;

String GET = "GET /logWattageEntry.php";
static uint8_t mux_id = 0; 
uint8_t buffer[36] = {0};
int buffer_length = 36;
int measurements_size = MEASUREMENTS_SIZE;
double measurements[MEASUREMENTS_SIZE] = {0};
unsigned int intervals[MEASUREMENTS_SIZE] = {0};
unsigned long millisOffest_l = 0;


int onboardLED = 13;
int BATTERY_SENSE_PIN = A1; 
int oldBatteryPcnt = 0;

// how many times remain to sleep before wake up
int nbr_remaining; 

// interrupt raised by the watchdog firing
// when the watchdog fires during sleep, this function will be executed
// remember that interrupts are disabled in ISR functions
ISR(WDT_vect)
{
        // not hanging, just waiting
        // reset the watchdog
        wdt_reset();
}

// function to configure the watchdog: let it sleep 8 seconds before firing
// when firing, configure it for resuming program execution
void configure_wdt(void)
{
 
  cli();                           // disable interrupts for changing the registers

  MCUSR = 0;                       // reset status register flags

                                   // Put timer in interrupt-only mode:                                       
  WDTCSR |= 0b00011000;            // Set WDCE (5th from left) and WDE (4th from left) to enter config mode,
                                   // using bitwise OR assignment (leaves other bits unchanged).
  WDTCSR =  0b01000000 | 0b000110; // set WDIE: interrupt enabled
                                   // clr WDE: reset disabled
                                   // and set delay interval (right side of bar) to 8 seconds

  sei();                           // re-enable interrupts

  // reminder of the definitions for the time before firing
  // delay interval patterns:
  //  16 ms:     0b000000
  //  500 ms:    0b000101
  //  1 second:  0b000110
  //  2 seconds: 0b000111
  //  4 seconds: 0b100000
  //  8 seconds: 0b100001
 
}

// Put the Arduino to deep sleep. Only an interrupt can wake it up.
void sleep(int ncycles)
{  
  nbr_remaining = ncycles; // defines how many cycles should sleep

  // Set sleep to full power down.  Only external interrupts or
  // the watchdog timer can wake the CPU!
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 
  // Turn off the ADC while asleep.
  power_adc_disable();
 
  while (nbr_remaining > 0){ // while some cycles left, sleep!

  // Enable sleep and enter sleep mode.
  sleep_mode();

  // CPU is now asleep and program execution completely halts!
  // Once awake, execution will resume at this point if the
  // watchdog is configured for resume rather than restart
 
  // When awake, disable sleep mode
  sleep_disable();
  //update offset (sletp for 1s)
  millisOffest_l+=1000;
  
  // we have slept one time more
  nbr_remaining = nbr_remaining - 1;
 
  }
 
  // put everything on again
  power_all_enable();
 
}

 
void setup()
{  
  blink(5);
  wifi.restart();
  analogReference(DEFAULT);
// configure the watchdog
  configure_wdt();
  Serial.begin(9600);
  Serial.print(F("Measurement size "));
  Serial.println(measurements_size);
  pinMode(CHPD_PIN, OUTPUT); //esp01 chip core
  pinMode(onboardLED, OUTPUT);     
  emon1.current(2, 90);             // Current: input pin, calibration.
  digitalWrite(CHPD_PIN, HIGH);
  bool connected = connectToWifi();
  if (!saveEnergy){
  } else {
    digitalWrite(CHPD_PIN, LOW);
  }
}

bool connectToWifi(){
    delay(500);
    Serial.println(F("Connecting to Wifi"));
    if (wifi.setOprToStationSoftAP()) {
    } else {
        Serial.println(F("to station + softap err"));
        return 0;
    }
    if (wifi.joinAP(SSID, PASSWORD)) {
        PRINT_ASLN("Join AP success");
    } else {
        Serial.print(F("Join AP failure\r\n"));
        return 0;
    }

        if (wifi.enableMUX()) {
        PRINT_ASLN("multy ok");
    } else {
        Serial.println(F("multy err"));
        return 0;
    }
    
    Serial.println(F("setup end"));
    return 1;
}

void updateSensorData(String data, unsigned long referrenceTime){
  float batteryLevel = getBatteryLevel();
  String GET_REQUEST = GET + "?bt=" + batteryLevel + "&rt=" + referrenceTime + "&w=" + data +"\r\n\r\n";
  updateData(HOST_NAME, HOST_PORT, &GET_REQUEST[0]);
}

void updateData(char* host_name, int port, char *get_str){
    Serial.print(F("crearting TCP to "));
    Serial.print(host_name);
    Serial.print(":");
    Serial.println(port);
    if (wifi.createTCP(mux_id, host_name, port)) {
        PRINT_ASLN("create tcp ok\r\n");
    } else {
        Serial.print(F("create tcp err\r\n"));
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

void send_measurements(int measurements_count, unsigned long referrenceTime){
  unsigned long startTime = millis();
  unsigned long startTimeLocal = millis();
  int maxSamplesPerRequest = 20;
  int pointer = 0;

  digitalWrite(CHPD_PIN, HIGH);
  connectToWifi();
//  delay(6000);
//  Serial.print("IP: ");       
//  Serial.println(wifi.getLocalIP().c_str());
  Serial.print(F("Connected to wifi in "));
  Serial.println(millis() - startTimeLocal);
  startTimeLocal = millis();
  while (pointer < measurements_count){
    String str = String("");
//    for (int i = 0 ; i< measurements_count; i++){
      for(int i = 0; i<maxSamplesPerRequest;i++){
      if (pointer >= measurements_count)
        break;
//      Serial.print("sending measurement ");
      Serial.print(pointer);
      Serial.print(": ");
      Serial.println(measurements[pointer]);
      if (i!=0){
        str +=String(",");
      }
      str += String(measurements[pointer]);    
      str += String(";");
      str += String(intervals[pointer]);
      pointer++;
    }
    updateSensorData(str, referrenceTime);
  }

//  Serial.println(str);
  if (saveEnergy){
    digitalWrite(CHPD_PIN, LOW);
  }
  Serial.print(F("Sent data in "));
  Serial.println(millis() - startTimeLocal);
  startTimeLocal = millis();
  unsigned long totalTime = millis() - startTime;
  Serial.print(F("Total send time was:"));
  Serial.println(totalTime);
  delay(500);
}


void blink(int times){
  for (int i=0; i<times;i++){
    digitalWrite(onboardLED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(onboardLED, LOW);    // turn the LED off by making the voltage LOW
    delay(100); 
  }
}

long millisOffset(){
    return millis() + millisOffest_l;
}

  float getBatteryLevel(){
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
    
    // 1M, 470K divider across battery and using internal ADC ref of 1.1V
    // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
    // ((1e6+470e3)/470e3)*3.3 = Vmax = 10.41 Volts
    // 10.41/1023 = Volts per bit = 0.010180945

    float batteryV  = sensorValue * 0.010180945;
    Serial.print(F("Battery Voltage: "));
    Serial.print(batteryV);
    Serial.println(" V");

    return batteryV;
  }

//debug
//void loop(){
//  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
// Serial.print("IRMS:");
// Serial.println(Irms*230.0); 
// //Serial.print("VOlts:");
// //Serial.println(getBatteryLevel());
// 
// delay(500);
//
//}



void loop(){

  static int measurements_count = 0;
  static unsigned long lastMeasurementTS = -1000000;
  static unsigned long time = millis();
  static short firstRun = 1;
  static long WIFI_UODATE_INTERVAL_MS = WIFI_UODATE_INTERVAL_M*60*1000;


  //take one measurement
  digitalWrite(onboardLED, HIGH);
  double watts = emon1.calcIrms(1480) * 230;  // Calculate Irms only nd convert to Watts
  unsigned long measurementTime = millisOffset();

  
  Serial.print(F("Took measurement of "));
  Serial.print(watts);
  Serial.println(F(" watts."));

  Serial.print(F("measurementTime - lastMeasurementTS = "));
  Serial.println(measurementTime - lastMeasurementTS);

  Serial.print(F("measurements_count="));
  Serial.println(measurements_count);

  if (measurements_count>0){
    Serial.print(F("last measurement was = "));
    Serial.print(measurements[measurements_count-1]);
    Serial.print(F(" so diff is "));
    Serial.println(measurements[measurements_count-1] - watts);
  }
  
  
  
  // if too long has passed since last measurement, or measrement diff from last measurement is too big, save to buffer
  if (((measurementTime - lastMeasurementTS)/1000 > MAX_TIME_BETWEEN_MEASUREMENTS_S) || measurements_count==0 || (measurements_count>0 && measurements[measurements_count-1] - watts > MAX_MEASUREMENT_DIFF)){
          measurements[measurements_count] = watts;
          intervals[measurements_count] = (measurementTime-time)/1000.0; //time diff from cycle start
          measurements_count++; 
          lastMeasurementTS = measurementTime;
          Serial.println(F("Measurement added to buffer"));
      }


    Serial.print(F("Buffer utilization is "));
    Serial.print(measurements_count);
    Serial.print(F("/"));
    Serial.println(measurements_size);

    
    Serial.print(F("Time since last flush="));   
    Serial.print((millisOffset() - time)/(1000));   
    Serial.println(F(" seconds."));   
    Serial.print(F("Next flush schaduled in "));   
    Serial.print((long) (WIFI_UODATE_INTERVAL_M*60 - (millisOffset() - time)/(1000))); 
    Serial.println(F(" seconds."));   

    Serial.flush();



   //if buffer is full, or too long has passed since last send, send buffer
   long timeLeftToSync_M = (millisOffset() - time)/(1000*60L);
   if (firstRun || (timeLeftToSync_M  >= WIFI_UODATE_INTERVAL_M) || measurements_count >= measurements_size){
    Serial.println(F("Fluhing now!"));   
    send_measurements(measurements_count, millisOffset()-time);
    measurements_count = 0;
    time = millisOffset();
    firstRun = 0;
  }

  Serial.flush();

  unsigned long sleepTime_s = MEASUREMENT_INTERVAL_S - (millisOffset() - measurementTime)/1000;
  Serial.print(F("Going to sleep for: "));   
  Serial.print(sleepTime_s);   
  Serial.println(F(" seconds."));   
  digitalWrite(onboardLED, LOW);
  sleep(sleepTime_s);
}


//void loop()
//// send the first sample right away
//// 5 next one minute away from each other
//// get to normal pace
//
//{
//  static int measurements_count = 0;
//  static unsigned long time = millis();
//  static unsigned long measurementTime = 0;
//  static unsigned short iteration = 0;
//
//
//
//  
//  
//  if ((millisOffset() - time)/(1000*60)  >= WIFI_UODATE_INTERVAL_M || measurements_count >= measurements_size || iteration>0 && iteration<=1){
//    Serial.print(F("Measurements. Time:"));
//    Serial.println(millisOffset() - time);
//    send_measurements(measurements_count, millisOffset()-time);
//    measurements_count = 0;
//    Serial.print(F("Cycle. Time:"));
//    Serial.println(millisOffset() - time);
//    time = millisOffset();
//  } 
//
//  //blink(2);
//  measurementTime = millisOffset();
//  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
//  measurements[measurements_count] = Irms*230.0; 
//  intervals[measurements_count] = (measurementTime-time)/1000.0; //time diff from cycle start
//  measurements_count++;
//    digitalWrite(onboardLED, LOW);
//    sleep(MEASUREMENT_INTERVAL_S);
//    digitalWrite(onboardLED, HIGH);
//  if (iteration<=3){
//      iteration++;
//  }
//
//    Serial.print(F("Took "));
//    Serial.print(measurements_count);
//    Serial.print(F(" measurements out of "));
//    Serial.println(measurements_size);
//    Serial.print(F("Update will be sent in "));
//    Serial.print(WIFI_UODATE_INTERVAL_M*60 - (millisOffset() - time)/(1000));
//    Serial.println(F(" seconds"));
//}

