#include <Arduino.h>
#include <Wire.h>

#include <LiquidCrystal_I2C.h>



#define SDA_PIN 12 /* Define the SDA pin here  */
#define SCL_PIN 0 /* Define the SCL Pin here */
LiquidCrystal_I2C lcd(0x27, 16, 2);
void setup() {

Wire.begin(SDA_PIN, SCL_PIN);  /* Initialize I2C communication */
  lcd.init();                 /* initialize the lcd  */
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Hello, helo");   
  lcd.setCursor(0,1);
  lcd.print("dispenser!");

}

void loop() {



}

