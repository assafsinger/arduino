#include <TimerOne.h>
#include "TM1637.h"

#define ON 1
#define OFF 0

int8_t DISPLAY_VALUE[] = {0x00,0x00,0x00,0x00};
unsigned char ClockPoint = 1;
unsigned char Update;
unsigned char microsecond_10 = 0;
unsigned char _microsecond_10 = 0;
unsigned int eepromaddr;
boolean Flag_ReadTime;
long timerTimeSeconds = 0; //1:05h
long timeLeftSeconds = 0;
const int  buttonPin = A2;    // the pin that the pushbutton is attached to
const int  mosfetPin = 8;    // the pin that the pushbutton is attached to
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button
bool mosfetStateHigh = false;
unsigned long lastBottonClick = 0;

//current sensor
const int currentSensingPin = A4;
int sensitivity = 185;
int offsetVoltage = 2500;
int currentSensingAttampts = 1000;
int currentUpdateFrequency = 10*1000; //10 seconds
long nextCurrentPrint = 0;
double referenceCurrent = 0;



#define CLK 2//pins definitions for TM1637 and can be changed to other ports        
#define DIO 3
TM1637 tm1637(CLK,DIO);

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(buttonPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  pinMode(currentSensingPin, INPUT);

  digitalWrite(mosfetPin, LOW); 

  tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  tm1637.init();
  Timer1.initialize(10000);//timing for 10ms
  Timer1.attachInterrupt(TimingISR);//declare the interrupt serve routine:TimingISR  
  timeLeftSeconds = timerTimeSeconds;
  nextCurrentPrint = millis();
  Update = ON;

}

void setReferenceCurrent(){
  referenceCurrent = getOutputCurrent();
  Serial.print("Reference current set to:");
  Serial.println(referenceCurrent);
}

double getOutputCurrent(){
  long value = 0;
  for (int i=0;i<currentSensingAttampts;i++){
    value+=analogRead(currentSensingPin);
  }
  double adcValue = (double)value/currentSensingAttampts;

  //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
  //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
  //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
  //you must change the offset according to the input voltage)
  //0.185v(185mV) is rise in output voltage when 1A current flows at input
  double AcsValueF = (2.5 - (adcValue * (5.0 / 1024.0)) )/0.185;
  int signedReference = AcsValueF>0?-1:1;
  AcsValueF = AcsValueF + signedReference*abs(referenceCurrent);
  
  Serial.print("ADC Value =     ");
  Serial.print(adcValue);


  Serial.print("\t Current = ");
  Serial.println(AcsValueF,3);
  return AcsValueF;
}

boolean currentUpdate(){

  long now_t = millis();
    if (nextCurrentPrint - now_t > 0){
       Serial.print("outputCurrent:");
       double outputCurrent = getOutputCurrent();
       Serial.println(outputCurrent,3);
       int displayValue = (int)(abs(outputCurrent)*1000) % 1000;
       DISPLAY_VALUE[0] = displayValue/1000;
       DISPLAY_VALUE[1] = displayValue / 100 % 100;
       DISPLAY_VALUE[2] = displayValue / 10 % 10;
       DISPLAY_VALUE[3] = displayValue % 10;
       tm1637.point(POINT_OFF); 
       Update = OFF;
       return true;
    } else{ 
      if (abs(nextCurrentPrint-now_t)>currentUpdateFrequency){
        nextCurrentPrint = now_t+currentUpdateFrequency*2;
      }
      //update time;
      return false;
    } 
  }

void loop(){
 buttonState = digitalRead(buttonPin);
 if (buttonState != lastButtonState) {
    Serial.println("button");
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      //filter noise if multiple clicks in under 500ms
      if (millis() - lastBottonClick>500){ 
        // if the current state is HIGH then the button went from off to on:
        timerTimeSeconds+=3600; //increase one hour
        timeLeftSeconds+=3600;
        lastBottonClick = millis();
      }
    }
  }
  lastButtonState = buttonState;
  if(Update == ON)
  {
    if (!currentUpdate()){
      TimeUpdate();
    }
    tm1637.display(DISPLAY_VALUE);
    if (timeLeftSeconds > 0) {
      if (!mosfetStateHigh){
        setReferenceCurrent();
        digitalWrite(mosfetPin, HIGH); 
        mosfetStateHigh = true;
      }
    } else {
      if (mosfetStateHigh){
        digitalWrite(mosfetPin, LOW); 
        mosfetStateHigh = false;
      }
    }
  }
}
//************************************************
void TimingISR()
{
  microsecond_10 ++;
  if(microsecond_10 == 100 && timeLeftSeconds>0){
    timeLeftSeconds --;
    Update = ON;
    microsecond_10 = 0;
    ClockPoint = (~ClockPoint) & 0x01;  
  }
}

void TimeUpdate(void)
{
  if(ClockPoint)
    tm1637.point(POINT_ON);//POINT_ON = 1,POINT_OFF = 0;
  else tm1637.point(POINT_OFF); 

  int hours = timeLeftSeconds/3600;
  int mins = timeLeftSeconds%3600/60;
  int seconds = timeLeftSeconds%60;
    Serial.println("3");

  if (hours>0){
    DISPLAY_VALUE[2] = mins / 10;
    DISPLAY_VALUE[3] = mins % 10;
    DISPLAY_VALUE[0] = hours / 10;
    DISPLAY_VALUE[1] = hours % 10;
  } else {
    DISPLAY_VALUE[2] = seconds / 10;
    DISPLAY_VALUE[3] = seconds % 10;
    DISPLAY_VALUE[0] = mins / 10;
    DISPLAY_VALUE[1] = mins % 10;
  }
  Serial.print(DISPLAY_VALUE[0]);
  Serial.print(DISPLAY_VALUE[1]);
  Serial.print(":");
  Serial.print(DISPLAY_VALUE[2]);
  Serial.println(DISPLAY_VALUE[3]);
  Update = OFF;
}

void stopwatchStart()//timer1 on
{
  Flag_ReadTime = 0;
  TCCR1B |= Timer1.clockSelectBits; 
}

void stopwatchPause()//timer1 off if [CS12 CS11 CS10] is [0 0 0].
{
  TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
}

void stopwatchReset()
{
  stopwatchPause();
  Flag_ReadTime = 0;
  _microsecond_10 = 0;
  microsecond_10 = 0;
  int timeLeftSeconds = timerTimeSeconds;
  Update = ON;
}
