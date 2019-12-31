#include <dht.h>
#include "TM1637.h"
 
//{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
//0~9,A,b,C,d,E,F
 
 
 
#define dht_pin 4 // Pin sensor is connected to
#define CLK 2//Pins for TM1637       
#define DIO 3
 
TM1637 tm1637(CLK,DIO);
dht DHT;
 
 
void setup(){
  
  tm1637.init();
  tm1637.set(BRIGHT_TYPICAL); 
  //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
 
  delay(1500);//Delay to let system boot
 
 
}//end "setup()"
 
 
 
void loop(){
 
  //Start of Program 
 
  DHT.read22(dht_pin);
 
 
 int temp = DHT.temperature;
 int humidity = DHT.humidity;
 
 int digitoneT = temp / 10;
 int digittwoT = temp % 10;
 
 int digitoneH = humidity / 10;
 int digittwoH = humidity % 10;
  
    tm1637.display(0,0x7f); 
    tm1637.display(1,digitoneT); 
    tm1637.display(2,digittwoT);
    tm1637.display(3,12);  // put a C at the end
    
    delay (3000);

    tm1637.displayRaw(0,0x76); 
    tm1637.display(1,0x7f); 
    tm1637.display(2,digitoneH); 
    tm1637.display(3,digittwoH);
 
    
  //Wait 3 seconds before accessing sensor again.
 
  //Fastest should be once every two seconds.
    delay(3000);
 
}// end loop() 
