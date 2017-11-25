#include "EmonLib.h"                   // Include Emon Library
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include "LowPower.h"

EnergyMonitor emon_p1, emon_p2, emon_p3;    
int onboardLED = 13;
int BATTERY_SENSE_PIN = A3; 
char msg[40]; 
const char* deviceId = "E1";
double supplyVoltage;

//RH_ASK driver;
 RH_ASK driver; //transmit on pin 12
              

void setup() {
    analogReference(DEFAULT);
    Serial.begin(9600);
    if (!driver.init())
        Serial.println("init failed");
    pinMode(onboardLED, OUTPUT);  
    emon_p1.current(0, 90);             // Current: input pin, calibration.
    emon_p2.current(1, 90);             // Current: input pin, calibration.
    emon_p3.current(2, 90);             // Current: input pin, calibration.
    supplyVoltage = emon_p1.readVcc()/1000.0;
}

void doubleToString(double d, char* localBuffer, int bufferlen){
  memset(localBuffer, 0, sizeof(localBuffer));
  int whole = (int)d;
  int fraction = (int)((d - whole) * 100); // For two decimal points
  snprintf(localBuffer, 8, "%i.%i", whole, fraction);
}

void sendSensorData(char* deviceId){
    double watts_p1 = emon_p1.calcIrms(1480) * 230;  // Calculate Irms only nd convert to Watts
    double watts_p2 = emon_p2.calcIrms(1480) * 230;  // Calculate Irms only nd convert to Watts
    double watts_p3 = emon_p3.calcIrms(1480) * 230;  // Calculate Irms only nd convert to Watts
    
    char sensorDataBuffer[30];
    char sensor1StringData[8];
    char sensor2StringData[8];
    char sensor3StringData[8];

    doubleToString(watts_p1, sensor1StringData, 8);
    doubleToString(watts_p2, sensor2StringData, 8);
    doubleToString(watts_p3, sensor3StringData, 8);

    snprintf(sensorDataBuffer, 30, "%s", sensor1StringData);
    //send PH1 twice for gain reasons
    sendData(deviceId, "PH1", sensorDataBuffer);
    sendData(deviceId, "PH1", sensorDataBuffer);

    memset(sensorDataBuffer, 0, sizeof(sensorDataBuffer));

    snprintf(sensorDataBuffer, 30, "%s", sensor2StringData);
    sendData(deviceId, "PH2", sensorDataBuffer);
    memset(sensorDataBuffer, 0, sizeof(sensorDataBuffer));

    snprintf(sensorDataBuffer, 30, "%s", sensor3StringData);
    sendData(deviceId, "PH3", sensorDataBuffer);

//    snprintf(sensorDataBuffer, 30, "%s;%s;%s", sensor1StringData, sensor2StringData, sensor3StringData);
//    sendData(deviceId, "ENE", sensorDataBuffer);
}

void sendData(char* deviceId, char* type, char* data){
    memset(msg, 0, sizeof(msg));
    snprintf(msg, sizeof(msg), "%s:%s:%s", deviceId, type, data);
    Serial.print("Sending:");
    Serial.println(msg);
    digitalWrite(13, true); // Turn on a light to show transmitting
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
    digitalWrite(13, false); // Turn off a light after transmission
}

void sendBatteryData(char* deviceId){
    memset(msg, 0, sizeof(msg));
    char batStr[8];
    doubleToString(getBatteryLevel(), batStr, 8);
    sendData(deviceId, "BAT", batStr);
}

double getBatteryLevel(){
    int sensorValue = analogRead(BATTERY_SENSE_PIN);
    
    // 1M, 470K divider across battery and using internal ADC ref of 1.1V
    // Sense point is bypassed with 0.1 uF cap to reduce noise at that point
    // ((1e6+470e3)/470e3)*3.3 = Vmax = 10.41 Volts
    // 10.41/1023 = Volts per bit = 0.010180945
    
    double voltsPerBit = (3.12765957447 * supplyVoltage) / 1023;
    //double batteryV  = sensorValue * 0.010180945;
    double batteryV  = sensorValue * voltsPerBit;
    Serial.print("supplyVoltage:");
    Serial.println(supplyVoltage);
    Serial.print(F("Battery Voltage: "));
    Serial.print(batteryV);
    Serial.println(" V");
    return batteryV;
}


void loop() {

    sendSensorData(deviceId);
    sendBatteryData(deviceId);
    //delay(2000); 
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
}
