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

//#define MEASUREMENT_INTERVAL_S 30
#define MEASUREMENT_INTERVAL_S 30
//#define WIFI_UODATE_INTERVAL_M 30
#define WIFI_UODATE_INTERVAL_M 10
#define MEASUREMENTS_SIZE (int)(1.05*(60/MEASUREMENT_INTERVAL_S)*WIFI_UODATE_INTERVAL_M)


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



// watchdog
//volatile int f_wdt=1;
//
//ISR(WDT_vect)
//{
//  if(f_wdt == 0)
//  {
//    f_wdt=1;
//  }
//  else
//  {
//    //Serial.println("WDT Overrun!!!");
//  }
//}
//
//void enterSleep(unsigned int seconds)
//{
//  f_wdt = 0;
//  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   /* EDIT: could also use SLEEP_MODE_PWR_DOWN for lowest power consumption. */
//  sleep_enable();
//  
//  /* Now enter sleep mode. */
//  for (int i=0;i<seconds;i++){
//    sleep_mode();
//  }
//  
//  /* The program will continue from here after the WDT timeout*/
//  sleep_disable(); /* First thing to do is disable sleep. */
//  
//  /* Re-enable the peripherals. */
//  power_all_enable();
//}
//
//void initializeWatchdog(){
//    /*** Setup the WDT ***/
//  
//  /* Clear the reset flag. */
//  MCUSR &= ~(1<<WDRF);
//  
//  /* In order to change WDE or the prescaler, we need to
//   * set WDCE (This will allow updates for 4 clock cycles).
//   */
//  WDTCSR |= (1<<WDCE) | (1<<WDE);
//
//  /* set new watchdog timeout prescaler value */
//  WDTCSR = 1<<WDP1 | 1<<WDP2; /* 1.0 seconds */
//  
//  /* Enable the WD interrupt (note no reset). */
//  WDTCSR |= _BV(WDIE);
//}
//


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
  analogReference(INTERNAL);
// configure the watchdog
  configure_wdt();
  Serial.begin(9600);
  Serial.print(F("Measurement size "));
  Serial.println(measurements_size);
  pinMode(CHPD_PIN, OUTPUT); //esp01 chip core
  pinMode(onboardLED, OUTPUT);     
  emon1.current(0, 90);             // Current: input pin, calibration.
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
  Serial.print("Sent data in ");
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

//void lowPowerSleep(int seconds){
//  Serial.print("sleep ");
//  Serial.println(seconds);
//  for (int i=0; i<seconds; i++){
//    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
//  }
//}

  
  long millisOffset(){
    return millis() + millisOffest_l;
  }

  float getBatteryLevel(){
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
    
    // 1M, 470K divider across battery and using internal ADC ref of 1.1V
    // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
    // ((1e6+470e3)/470e3)*1.1 = Vmax = 3.44 Volts
    // 3.44/1023 = Volts per bit = 0.003363075

    float batteryV  = sensorValue * 0.003363075;
    Serial.print("Battery Voltage: ");
    Serial.print(batteryV);
    Serial.println(" V");

    return batteryV;
  }


void loop()
// send the first sample right away
// 5 next one minute away from each other
// get to normal pace

{
  static int measurements_count = 0;
  static unsigned long time = millis();
  static unsigned long measurementTime = 0;
  static unsigned short iteration = 0;



  
  
  if ((millisOffset() - time)/(1000*60)  >= WIFI_UODATE_INTERVAL_M || measurements_count >= measurements_size || iteration>0 && iteration<=1){
    Serial.print(F("Measurements. Time:"));
    Serial.println(millisOffset() - time);
    send_measurements(measurements_count, millisOffset()-time);
    measurements_count = 0;
    Serial.print(F("Cycle. Time:"));
    Serial.println(millisOffset() - time);
    time = millisOffset();
  } 

  //blink(2);
  measurementTime = millisOffset();
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  measurements[measurements_count] = Irms*220.0; 
  intervals[measurements_count] = (measurementTime-time)/1000.0; //time diff from cycle start
  //Serial.println((measurementTime-time)/1000.0);
  //Serial.print(Irms*220.0);         // Apparent power
  //Serial.print(" ");
  //Serial.println(Irms);          // Irms
  measurements_count++;
//  lowPowerSleep(MEASUREMENT_INTERVAL_S - ((millis()-measurementTime)/1000));
    digitalWrite(onboardLED, LOW);
    sleep(MEASUREMENT_INTERVAL_S);
    digitalWrite(onboardLED, HIGH);
//delay(MEASUREMENT_INTERVAL_S*1000 - ((millis()-measurementTime)));
  if (iteration<=3){
      iteration++;
  }

    Serial.print(F("Took "));
    Serial.print(measurements_count);
    Serial.print(F(" measurements out of "));
    Serial.println(measurements_size);
    Serial.print(F("Update will be sent in "));
    Serial.print(WIFI_UODATE_INTERVAL_M*60 - (millisOffset() - time)/(1000));
    Serial.println(F(" seconds"));
}

