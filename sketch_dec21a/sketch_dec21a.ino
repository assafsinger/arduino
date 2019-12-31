
#include <livolo.h>

byte trport = D1;              // port of 433-transmitter
Livolo livolo(trport);        // transmitter connected to pin trport
int onoff = 10;               // data from Serial
unsigned int ID = 6400;       // ID of Remote - type your own ID/ f.e. 6400; 19303; 10550; 8500; 7400
unsigned int IDB = 0;         // Number of remote's button #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
unsigned int SCENE1 = 90;     // Number of scene (available 90, 114, 10, 18)

void setup() {
Serial.begin(9600);  // serial init
}

void loop() {

// Serial reading
//
if (Serial.available() > 0)
{
onoff = Serial.read()-48;
Serial.println(onoff);
}

if (onoff == 1)
{
  livolo.sendButton(ID, IDB);
  Serial.println("Button");
  delay (1000);
}

if (onoff == 2)
{
  //livolo.sendButton(ID, SCENE1);
  livolo.sendButton(ID, 96);

  Serial.println("button 2");
  delay (1000);
}

if (onoff == 0)
{
 
  livolo.sendButton(ID, 106);
  Serial.println("off");
  delay (1000);
}
}
