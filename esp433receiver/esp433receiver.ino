#include <RH_ASK.h>
#include <SPI.h>

RH_ASK driver;
int onboardLED = 13;



void setup() {
    Serial.begin(9600);
    Serial.print("Start listening");
    pinMode(onboardLED, OUTPUT);  
}

void loop() {
    uint8_t buf[30];
    uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      digitalWrite(13, true); // Turn on a light to show transmitting
      int i;
      // Message with a good checksum received, dump it.
      Serial.print("Message: ");
      Serial.println((char*)buf);   
      digitalWrite(13, false); // Turn on a light to show transmitting
      
    }
}
