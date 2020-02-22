#include "IRremoteESP8266.h"
#include "IRelectra.h"
#include <IRsend.h>

  
  IRsend irsend(D2);
  IRelectra e(&irsend);
  uint16_t rawDataHighHeat[181] = {3098, 2974,  934, 990,  996, 1948,  1942, 1946,  1942, 966,  908, 1014,  1000, 970,  960, 950,  938, 1012,  992, 1952,  1942, 1948,  1880, 1008,  1000, 972,  968, 972,  908, 1002,  1000, 978,  906, 1008,  998, 978,  932, 978,  936, 1006,  992, 978,  908, 1010,  998, 976,  934, 978,  940, 1006,  938, 1006,  936, 1008,  936, 1004,  936, 1984,  1910, 1012,  3006, 2972,  970, 970,  936, 1954,  1972, 1944,  1910, 976,  970, 1006,  940, 1002,  966, 988,  938, 1006,  964, 1942,  1934, 1928,  1970, 972,  942, 1008,  904, 1004,  938, 1004,  998, 976,  906, 1004,  938, 1012,  968, 1002,  906, 1004,  996, 948,  938, 1004,  998, 944,  940, 1010,  1000, 978,  906, 998,  938, 1010,  998, 978,  940, 1974,  1938, 972,  2968, 2946,  1030, 954,  970, 1942,  1908, 1952,  1972, 974,  908, 1008,  966, 970,  998, 954,  936, 1008,  938, 1982,  1970, 1946,  1942, 972,  968, 968,  960, 960,  936, 1004,  996, 950,  936, 998,  1026, 928,  936, 1010,  968, 1006,  968, 960,  968, 988,  970, 978,  940, 996,  906, 1002,  992, 960,  938, 1004,  938, 1012,  996, 1944,  1906, 978,  3974};  // UNKNOWN C1DB777C
  uint16_t rawDataLowHeat[181] = {3034, 3938,  1882, 1972,  1910, 1006,  936, 1008,  936, 1012,  938, 1006,  996, 976,  908, 1004,  992, 952,  938, 1974,  1910, 1978,  1958, 962,  998, 974,  908, 1002,  936, 1010,  996, 978,  906, 1004,  940, 1002,  938, 1006,  938, 1004,  936, 1010,  940, 1000,  938, 1006,  938, 1008,  998, 976,  908, 1006,  996, 974,  908, 1006,  938, 1982,  1972, 974,  2914, 3980,  1960, 1928,  1912, 1004,  938, 1006,  938, 1008,  1000, 972,  908, 1008,  936, 1006,  938, 1004,  998, 1948,  1880, 1980,  1962, 950,  938, 1008,  998, 976,  966, 976,  906, 1006,  938, 1006,  936, 1008,  938, 1006,  934, 1006,  938, 1006,  938, 1010,  998, 972,  970, 974,  908, 1006,  1000, 972,  908, 1006,  936, 1012,  934, 1978,  1910, 1004,  3008, 3952,  1942, 1942,  1882, 1008,  998, 974,  968, 974,  906, 1008,  1000, 976,  966, 974,  908, 1006,  938, 1978,  1910, 1978,  1912, 1006,  998, 976,  964, 978,  970, 972,  906, 1008,  938, 1006,  990, 952,  936, 1012,  936, 1004,  988, 956,  936, 1010,  994, 976,  908, 1008,  998, 974,  908, 1006,  936, 1008,  938, 1006,  934, 1982,  1970, 974,  3884};  // UNKNOWN 368E86FA




void setup() {
  Serial.begin(115200);
  irsend.begin();

}


void loop()
{
  if(Serial.available() > 0)  {
    int incomingData= Serial.read(); // can be -1 if read error
    switch(incomingData) {
        case '1':
           e.sendElectra(POWER_OFF, IRElectraModeCool, IRElectraFanLow, 24, 0, 0);
           Serial.println("Sending OFF");
           break;

        case '2': //turning on and off
           irsend.sendRaw(rawDataLowHeat, 181, 38);
           Serial.println("Sending rawDataLowHeat");
           break;

        case '3': //keeping on
           irsend.sendRaw(rawDataHighHeat, 181, 38);
           Serial.println("Sending rawDataHighHeat");
           break;

        default:
           // handle unwanted input here
           break;
   }
 }
   // do other stuff here
}
