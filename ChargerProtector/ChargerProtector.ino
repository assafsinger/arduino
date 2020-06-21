#include <TimerOne.h>
#include "TM1637.h"

#define ON 1
#define OFF 0

int8_t TimeDisp[] = {0x00,0x00,0x00,0x00};
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

#define CLK 2//pins definitions for TM1637 and can be changed to other ports        
#define DIO 3
TM1637 tm1637(CLK,DIO);

void setup()
{
  Serial.begin(9600);
  Serial.println("Start");
  pinMode(buttonPin, INPUT);
  pinMode(mosfetPin, OUTPUT);
  digitalWrite(mosfetPin, LOW); 

  tm1637.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
  tm1637.init();
  Timer1.initialize(10000);//timing for 10ms
  Timer1.attachInterrupt(TimingISR);//declare the interrupt serve routine:TimingISR  
  timeLeftSeconds = timerTimeSeconds;

  Update = ON;

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
    TimeUpdate();
    tm1637.display(TimeDisp);
    if (timeLeftSeconds > 0) {
      if (!mosfetStateHigh){
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
  Serial.println("1");
  if(ClockPoint)
    tm1637.point(POINT_ON);//POINT_ON = 1,POINT_OFF = 0;
  else tm1637.point(POINT_OFF); 
  Serial.println("2");

  int hours = timeLeftSeconds/3600;
  int mins = timeLeftSeconds%3600/60;
  int seconds = timeLeftSeconds%60;
    Serial.println("3");

  if (hours>0){
    TimeDisp[2] = mins / 10;
    TimeDisp[3] = mins % 10;
    TimeDisp[0] = hours / 10;
    TimeDisp[1] = hours % 10;
  } else {
    TimeDisp[2] = seconds / 10;
    TimeDisp[3] = seconds % 10;
    TimeDisp[0] = mins / 10;
    TimeDisp[1] = mins % 10;
  }
  Serial.print(TimeDisp[0]);
  Serial.print(TimeDisp[1]);
  Serial.print(":");
  Serial.print(TimeDisp[2]);
  Serial.println(TimeDisp[3]);
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
