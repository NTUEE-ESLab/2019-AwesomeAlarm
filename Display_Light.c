#include <Arduino.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 9
#define DIO 8
#define ledPin 7
#define laserPin 4


TM1637Display display(CLK, DIO);

void setup()
{
  display.setBrightness(0x0f);
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  display.setSegments(data);
  pinMode(ledPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
}

void loop()
{
  digitalWrite(ledPin, LOW);
  digitalWrite(laserPin, LOW);
  for(int i=5;i>=0;i--)
  {
      display.showNumberDec(i);
      delay(1000);
  }
  digitalWrite(ledPin, HIGH);
  digitalWrite(laserPin, HIGH);
  delay(1000);
  while(true){
    
  }
}
