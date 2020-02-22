#ifndef __SHUTTERS_H__
#define __SHUTTERS_H__

#include <Arduino.h> 
#include <livolo.h>


#define SWITCH_FULL_OPERATION_TIME_MS 15500
#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))


class Shutters{

public:
	Shutters(short);
	void openShutter(void); 
	void closeShutter(void); 
	void resetSwitchState(void); 
	void setShutterState(short state); 
	void wrap();
	bool isWrap();
	
private:
  bool politeTrack(unsigned long);
	unsigned int ID = 6400;       		// ID of Remote - type your own ID/ f.e. 6400; 19303; 10550; 8500; 7400
	unsigned int SHUTTER_UP = 0;        // Number of remote's button #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
	unsigned int SHUTTER_DOWN = 96;     // Number of remote's button #1: 0, #2: 96, #3: 120, #4: 24, #5: 80, #6: 48, #7: 108, #8: 12, #9: 72; #10: 40, #OFF: 106
	unsigned int SHUTTER_OFF = 106;
	unsigned int openPercentage = 101;
	unsigned long operationTime = 0;
  unsigned long politeTrackWait = 0;
  unsigned long politeTrackLastCheck = 0;
	Livolo livolo = NULL;	
};

Shutters::Shutters(short pin){
	livolo = Livolo(pin);
}


void Shutters::openShutter(void){
	setShutterState(100);
}

void Shutters::closeShutter(void){
	setShutterState(0);
}

void Shutters::resetSwitchState(void){
	livolo.sendButton(ID, SHUTTER_OFF);
	delay(1000);
}

void Shutters::setShutterState(short state){
	 int desiredPercentage = state;
	 int operation = openPercentage - desiredPercentage;
	 if (abs(operation)<5) //if the change is too small do nothing.
	 	operation = 0;
	 else if (abs(operation)>95) //if big enough go all the way
	 	operation = 100 * sgn(operation);
	 operationTime = SWITCH_FULL_OPERATION_TIME_MS * abs(operation)/100.0;
	 Serial.printf("operation:%d oprtationTime:%d\n",operation,operationTime);
	if (operation > 0){ //close a bit
		livolo.sendButton(ID, SHUTTER_DOWN);
	 } else if(operation<0) { //open a bit
	 	livolo.sendButton(ID, SHUTTER_UP);
	 }
	 openPercentage = desiredPercentage;
}

bool Shutters::isWrap(){
	return operationTime>0;
}

void Shutters::wrap(){
	if (isWrap()){
		if (politeTrack(operationTime)){
		  operationTime = 0;
		  resetSwitchState();
		}
	}
}

bool Shutters::politeTrack(unsigned long waitTimeMS){
  if (!politeTrackWait){ //on first iteration
      politeTrackLastCheck = millis();
  }
  
  if (waitTimeMS > politeTrackWait){
    delay(10);
    politeTrackWait += millis() - politeTrackLastCheck;
    politeTrackLastCheck = millis();
    return false;
  }
  politeTrackLastCheck = 0;
  politeTrackWait = 0;
  return true;
}

#endif 

