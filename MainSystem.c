#include <Arduino.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <Wire.h>

// Module connection pins (Digital Pins)
#define CLK 9
#define DIO 8
#define ledPin 7
#define laserPin 4
int CDSPin = A0; // 光敏電阻接在A0接腳
int CDSVal = 0; // 設定初始光敏電阻值為0

TM1637Display display(CLK, DIO);


int LED = 13;
SoftwareSerial I2CBT(10,11); //定義Arduino PIN10及PIN11分別為RX及TX腳位
//請注意Arduino的TX要接藍牙模組的RX, 反之亦然

void setup()
{
  display.setBrightness(0x0f);
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  display.setSegments(data);
  pinMode(ledPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(CDSPin, INPUT);
  Serial.begin(9600);
  I2CBT.begin(9600);
}

void loop()
{
  byte cmmd[20];
  int insize;
  while(1){
    //read message from bluetooth
    if ((insize=(I2CBT.available()))>0){
      Serial.print("input size = "); 
      Serial.println(insize);
      for (int i=0; i<insize; i++){
        Serial.print(cmmd[i]=char(I2CBT.read()));
        Serial.print("\n"); 
      }
     }
    } //Switch
  
    Serial.println("START");  
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
    Serial.println("RING");  
  while(true){
    CDSVal = analogRead(CDSPin);
    Serial.println("READ");  
    Serial.println(CDSVal);  
    if (CDSVal<100){ break;}
    delay(2000); //每2s讀取一次   
    
  }
}
