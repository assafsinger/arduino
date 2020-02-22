/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 2 (connect to TX of other device)
 * TX is digital pin 3 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.
s
 */
#include <SoftwareSerial.h>
#include <DHT.h>
#include <Wire.h>
#include "SparkFunBME280.h"

#define  DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
int DHTPin = 9; 
// Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);  
BME280 mySensor;

SoftwareSerial mySerial(2, 3); // RX, TX


void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  Serial.println("Station start");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(38400);

  pinMode(DHTPin, INPUT);
  dht.begin();      

  Wire.begin();
  myBME280Sensor.setI2CAddress(0x76); //The default for the SparkFun Environmental Combo board is 0x77 (jumper open).
  if (myBME280Sensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("The BME280Sensor did not respond. Please check wiring.");
  }
}

void loop()
{
  while (mySerial.available())
    {
        char character = mySerial.read(); // Receive a single character from the software serial port
        Serial.print("received:");
        Serial.println(character);
        if (!((character<='5' and character>='0') or character=='8' or character=='9')){
          Serial.println("invalid input");
          continue;
        }
        int pin = character - '0';
        
        //DHT (8=temp, 9=humidity)
        if (pin == 9){
          float humidity = dht.readHumidity();
          Serial.print("HUMIDITY LEVEL FOR SENSOR ");
          Serial.print(pin);
          Serial.print(" :");
          Serial.println(humidity);
          mySerial.println((int)(humidity*10));
          continue;
        }
        if (pin == 8){
          float temp = dht.readTemperature();
          Serial.print("TEMPRATURE LEVEL FOR SENSOR ");
          Serial.print(pin);
          Serial.print(" :");
          Serial.println(temp);
          mySerial.println((int)(temp*10));
          continue;
        }
        //bose BME280 mySensor pressure
        if (pin == 4){
          float pressure = myBME280Sensor.readFloatPressure() * 0.00029530
          Serial.print("TEMPRATURE LEVEL FOR SENSOR ");
          Serial.print(pin);
          Serial.print(" :");
          Serial.println(pressure);
          mySerial.println((int)(pressure*100));
          continue;
        }
        //bose BME280 mySensor temprature
        if (pin == 5){
          float temp = myBME280Sensor.readFloatPressure() * 0.00029530
          Serial.print("TEMPRATURE LEVEL FOR SENSOR ");
          Serial.print(pin);
          Serial.print(" :");
          Serial.println(temp);
          mySerial.println((int)(temp*10));
          continue;
        }

        //else - analogue
        int moisture_value=0;
        moisture_value= analogRead(pin);
  
        //calibrate
        //map(rawValue, dryValue, wetValue, friendlyDryValue, friendlyWetValue);

        moisture_value = map(moisture_value, 1020, 270, 0, 100);
  
        Serial.print("MOISTURE LEVEL FOR SENSOR ");
        Serial.print(pin);
        Serial.print(" :");
        Serial.println(moisture_value);
        mySerial.println(moisture_value);
    }
}

  
 


