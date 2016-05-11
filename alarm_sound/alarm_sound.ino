//Example Code for KY-006
 
int buzzer = 13 ;// setting controls the digital IO foot buzzer

void setup ()
{
  pinMode (buzzer, OUTPUT) ;// set the digital IO pin mode, OUTPUT out of Wen
}
void loop ()
{
  unsigned char i, j ;// define variables
  while (1)
  {
    for (i = 0; i <8000; i++) // Wen a frequency sound
    {
      digitalWrite (buzzer, HIGH) ;// send voice
      delayMicroseconds (i) ;// Delay 1ms
      digitalWrite (buzzer, LOW) ;// do not send voice
      delayMicroseconds (i) ;// delay ms
    }
    for (i = 8000; i >0; i--) // Wen Qie out another frequency sound
    {
      digitalWrite (buzzer, HIGH) ;// send voice
      delayMicroseconds (i) ;// delay 2ms
      digitalWrite (buzzer, LOW) ;// do not send voice
      delayMicroseconds (i) ;// delay 2ms
    }
  }
}
