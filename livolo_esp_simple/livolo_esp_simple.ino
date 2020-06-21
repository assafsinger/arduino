#include <LivoloTx.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <livolo.h>

static const uint16_t LIVOLO_REMOTE_ID = 6400;

static const int TX_PIN = D1;

  Livolo livolo = Livolo(TX_PIN);
  unsigned int ID = 6400;           // ID of Remote - type your own ID/ f.e. 6400; 19303; 10550; 8500; 7400
  unsigned int SHUTTER_UP = 0;        // Number of remote's button #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
  unsigned int SHUTTER_DOWN = 96;     // Number of remote's button #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
  unsigned int SHUTTER_OFF = 106;


void setup()
{
  Serial.begin(115200);
  Serial.println();

  pinMode(TX_PIN, OUTPUT);
  digitalWrite(TX_PIN, LOW);
}

void loop(){
    int incomingByte = 0; // for incoming serial data
    if (Serial.available() > 0) {
      // read the incoming byte:
      incomingByte = Serial.read();
      livolo.sendButton(ID, SHUTTER_DOWN);
      Serial.println("close");
    }
}
