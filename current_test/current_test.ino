const int currentPin = A0;
int sensitivity = 185;
int adcValue= 0;
int offsetVoltage = 2500;
double adcVoltage = 0;
double currentValue = 0;
 
void setup() 
{
  Serial.begin(9600);
  Serial.print(" Current Sensor ");
  Serial.println("  with Arduino  ");
}

double getADC(int pin, int count){
  long value = 0;
  for (int i=0;i<count;i++){
    value+=analogRead(pin);
  }
  Serial.println(value);
  return (double)value/count;
}
 
void loop()
{
  adcValue = getADC(currentPin, 1000);
  //adcVoltage = (adcValue / 1024.0) * 5000;
  //currentValue = ((adcVoltage - offsetVoltage) / sensitivity);

  //((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
  //2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
  //out to be 2.5 which is out offset. If your arduino is working on different voltage than 
  //you must change the offset according to the input voltage)
  //0.185v(185mV) is rise in output voltage when 1A current flows at input
  double AcsValueF = (2.5 - (adcValue * (5.0 / 1024.0)) )/0.190;
  
  Serial.print("ADC Value =     ");
  Serial.print(adcValue);


  Serial.print("\t Voltage(mV) = ");
  Serial.print(adcVoltage,3);
  
 
  Serial.print("\t Current = ");
  Serial.println(AcsValueF,3);
 
  delay(2500);
}
