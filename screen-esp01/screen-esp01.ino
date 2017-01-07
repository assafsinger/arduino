  // ESP8266 with 20x4 i2c LCD
  // Compatible with the Arduino IDE 1.6.4
  // Library https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
  // Bavensky :3
   
  #include <Wire.h> 
  #include <LiquidCrystal_I2C.h>
  
  LiquidCrystal_I2C lcd(0x20,20,4); 
  
  uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
  
  void setup()  {
    Wire.begin(0, 2);
    lcd.begin();                     
    // ------- Quick 3 blinks of backlight  -------------
  for(int i = 0; i< 3; i++)
  {
    lcd.backlight();
    delay(250);
    lcd.noBacklight();
    delay(250);
  }
  lcd.backlight(); // finish with backlight on  
    lcd.createChar(1, heart);
  }
  
  void loop()  {
    lcd.home();
    lcd.print("Welcome to my world!");
    lcd.setCursor(2, 1);
    lcd.write(byte(1));
    lcd.print(" ESP8266 with");
    lcd.setCursor(0, 2);
    lcd.print(" LiquidCrystal I2C  ");
    lcd.setCursor(0, 3);
    lcd.print("ChiangMai Maker Club"); 
  }
