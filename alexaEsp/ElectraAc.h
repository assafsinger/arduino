#ifndef __ELECTRAAC_H__
#define __ELECTRAAC_H__

#include <Arduino.h> 
#include "IRremoteESP8266.h"
#include <IRsend.h>
#include <ESP8266WiFiMulti.h>



//cool 24 low  (off)
uint16_t coolRawData[181] = {3064, 3934,  1944, 972,  972, 1972,  1880, 1006,  938, 1010,  966, 978,  968, 974,  968, 976,  968, 1006,  940, 1974,  1912, 974,  968, 1946,  1966, 956,  968, 974,  936, 1010,  936, 1006,  992, 992,  926, 976,  962, 982,  966, 978,  936, 1010,  972, 970,  986, 954,  940, 1008,  972, 1002,  938, 1016,  928, 1000,  940, 976,  968, 1948,  1974, 974,  2912, 3980,  1914, 1004,  938, 1982,  1966, 974,  910, 1010,  970, 1002,  940, 980,  970, 998,  964, 978,  940, 1974,  1880, 1006,  968, 1948,  1964, 956,  970, 1002,  940, 1004,  940, 1002,  940, 974,  970, 974,  966, 978,  968, 976,  966, 976,  968, 980,  968, 1004,  938, 1006,  938, 976,  970, 1002,  906, 1006,  938, 1010,  970, 1942,  1942, 974,  3006, 3952,  1942, 974,  940, 1944,  1944, 974,  1000, 974,  938, 974,  998, 978,  940, 1000,  940, 976,  964, 1948,  1912, 1010,  972, 1974,  1910, 1006,  938, 1014,  936, 996,  936, 978,  938, 1004,  938, 1006,  936, 1010,  968, 972,  936, 1006,  940, 1008,  972, 1002,  938, 972,  998, 976,  938, 976,  968, 972,  970, 974,  940, 1978,  1940, 1000,  3920};  // UNKNOWN 2C7B176D

//Heat 26 low (off)
uint16_t heatRawData[181] = {3124, 3906,  1964, 1924,  1962, 954,  990, 958,  958, 986,  996, 980,  964, 980,  954, 952,  998, 978,  958, 1928,  1958, 1924,  994, 956,  1934, 986,  1000, 972,  968, 978,  964, 978,  966, 982,  966, 970,  960, 958,  988, 958,  986, 956,  988, 958,  986, 958,  994, 978,  964, 978,  932, 982,  998, 974,  960, 956,  988, 1932,  1972, 972,  2940, 3958,  1936, 1956,  1962, 956,  994, 976,  958, 954,  990, 958,  986, 956,  996, 950,  998, 970,  936, 1952,  2002, 1890,  1002, 972,  1932, 954,  990, 954,  996, 976,  956, 960,  960, 980,  966, 982,  964, 978,  990, 956,  988, 952,  998, 982,  964, 978,  962, 976,  958, 958,  994, 978,  958, 954,  964, 980,  990, 1928,  1970, 972,  2942, 3958,  1940, 1950,  1970, 978,  962, 948,  962, 986,  988, 952,  998, 978,  928, 984,  988, 956,  994, 1952,  1902, 1950,  966, 990,  1964, 978,  966, 974,  962, 952,  996, 982,  964, 974,  932, 982,  962, 984,  986, 960,  998, 974,  962, 956,  996, 974,  932, 984,  964, 982,  988, 950,  1000, 988,  962, 970,  958, 1928,  1994, 924,  3966};  // UNKNOWN 7DA891B0

class ElectraAc{

public:
	ElectraAc(short);
	void toggleHeat(void); 
	void toggleCool(void); 
	void toggleLastState(void);
	void begin(void);

	
private:
	bool isHeat;
	bool isOn;
	IRsend* irsend = NULL;	
};

ElectraAc::ElectraAc(short pin){
	irsend = new IRsend(pin);
	isHeat = false;
	isOn = false;
}

void ElectraAc::begin(){
	irsend->begin();
}



void ElectraAc::toggleHeat(){
	irsend->sendRaw(heatRawData, 181, 38);
	isOn = !isOn;
	isHeat = true;
	
}

void ElectraAc::toggleCool(){
	irsend->sendRaw(coolRawData, 181, 38);
	isOn = !isOn;
	isHeat = false;
}

void ElectraAc::toggleLastState(){
	if(isHeat){
		this->toggleHeat();
	} else {
		this->toggleCool();
	}
}
	
bool isOn(){
	return isOn;
}

bool isHeat(){
	return isHeat;
}



#endif 
