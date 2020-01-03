#include <Arduino.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include<Servo.h>


SoftwareSerial I2CBT(5, 6); 
int setting_hour = 0;
int setting_min = 0;
int setting_count = 0;
int insize; 
byte cmmd[20]; 

// Module connection pins (Digital Pins)
#define CLK 9
#define DIO 8
#define ledPin 7
#define laserPin 4
#define SERVO_PIN 2
int CDSPin = A0; // 光敏電阻接在A0接腳
int CDSVal = 0; // 設定初始光敏電阻值為0

TM1637Display display(CLK, DIO);
Servo servoVer;


int buzzerPin=10;
int trigPin = 12;                  //Trig Pin
int echoPin = 11;                  //Echo Pin
long duration, cm, inches;
int my_dist[21];
void ringTone(int pin) {
  for (int i=0; i<10; i++) { //repeat 10 times
    tone(pin, 1000);
    delay(50);
    tone(pin, 500);
    delay(50);
    }
  noTone(pin);
  delay(2000);
}

int remain = 5;
//請注意Arduino的TX要接藍牙模組的RX, 反之亦然
const int start_pos = 120;
const int end_pos = 80;

void distance()
{
//  for(int x = 50; x <= 150; x+=5){
//    servoVer.write(x);
//    delay(500);
//    
//    digitalWrite(trigPin, LOW);
//    delayMicroseconds(5);
//    digitalWrite(trigPin, HIGH);     // 給 Trig 高電位，持續 10微秒
//    delayMicroseconds(10);
//    digitalWrite(trigPin, LOW);
//  
//    pinMode(echoPin, INPUT);             // 讀取 echo 的電位
//    duration = pulseIn(echoPin, HIGH);   // 收到高電位時的時間
//   
//    cm = (duration/2) / 29.1;         // 將時間換算成距離 cm 或 inch  
//    inches = (duration/2) / 74; 
//    Serial.print("Angle : "); 
//    Serial.print(x); 
//    Serial.print(", Distance : ");  
//    Serial.print(inches);
//    Serial.print("in,   ");
//    Serial.print(cm);
//    Serial.print("cm");
//    Serial.println();
//    
//    delay(500);
//  }
    int t = 0;
    for(int x = start_pos; x >= end_pos; x-=5){
    servoVer.write(x);
    delay(500);
    
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);     // 給 Trig 高電位，持續 10微秒
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
  
    pinMode(echoPin, INPUT);             // 讀取 echo 的電位
    duration = pulseIn(echoPin, HIGH);   // 收到高電位時的時間
   
    cm = (duration/2) / 29.1;         // 將時間換算成距離 cm 或 inch  
    inches = (duration/2) / 74; 
    Serial.print("Angle : "); 
    Serial.print(x); 
    Serial.print(", Distance : ");  
    Serial.print(inches);
    Serial.print("in,   ");
    Serial.print(cm);
    Serial.print("cm");
    Serial.println();
    my_dist[t] = cm;
    t += 1;
    
    delay(500);
  }
}

void find_lowest()
{
    int t = 0;
    int new_dist = 0;
    int maximum = 0;
    int maximum_t = 0;
    int max_angle = 0;
    for(int x = start_pos; x >= end_pos; x-=5){
      servoVer.write(x);
      Serial.print(x); 
      delay(500);
      
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);     // 給 Trig 高電位，持續 10微秒
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
    
      pinMode(echoPin, INPUT);             // 讀取 echo 的電位
      duration = pulseIn(echoPin, HIGH);   // 收到高電位時的時間
     
      cm = (duration/2) / 29.1;         // 將時間換算成距離 cm 或 inch  
      inches = (duration/2) / 74; 
      Serial.print("Angle : "); 
      Serial.print(x); 
      Serial.print(", Distance : ");  
      Serial.print(inches);
      Serial.print("in,   ");
      Serial.print(cm);
      Serial.print("cm,    org:");
      Serial.print(my_dist[t]);
      Serial.println();
      new_dist = my_dist[t] - cm;
      if (new_dist > maximum && new_dist < 20){
        maximum = new_dist;
        maximum_t = t;
        max_angle = x;
      }
      t += 1;
      
      delay(500);
    }
    t = 0;
    for(int x = start_pos; x >= end_pos; x-=5){
      servoVer.write(x);
      delay(500);
      if (t==maximum_t) break;
      t += 1;
    }
    Serial.print("max_t: ");
    Serial.print(maximum_t);
    Serial.print("max: ");
    Serial.print(maximum);
    Serial.println();
}  

void check_setting_time(){
  if ((insize=(I2CBT.available()))>0)
  {  
    Serial.print("input size = "); 
    Serial.println(insize);  
    for (int i=0; i<insize; i++) {
      Serial.print(cmmd[i]=char(I2CBT.read()));
      Serial.println(" ");  
    }
    if(insize==3){
      setting_hour = (int)cmmd[0] - 48 + 10;
      setting_count = (int)cmmd[1] - 48;
      setting_min = (int)cmmd[2] -48;
      Serial.println(setting_hour);Serial.println(setting_min); 
    }
    if(insize==4){
      setting_hour = (int)cmmd[0] -48 + 10;
      setting_count = (int)cmmd[1] - 48;
      setting_min = ((int)cmmd[2] -48)*10 + (int)cmmd[3] - 48;
      Serial.println(setting_hour);Serial.println(setting_min); 
    }
  } 
}


void setup()
{
  display.setBrightness(0x0f);
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  pinMode(buzzerPin, OUTPUT);
  display.setSegments(data);
  pinMode(ledPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(CDSPin, INPUT);
  Serial.begin(9600);
  I2CBT.begin(38400);
  pinMode(trigPin, OUTPUT);        //Define inputs and outputs 
  pinMode(echoPin, INPUT);
  servoVer.attach(SERVO_PIN, 544, 2400); 
  servoVer.write(start_pos);
}

void loop()
{
  ringTone(buzzerPin);  

  Serial.println("GALA1");
  while(1){ 
  Serial.println("GALA2");
    check_setting_time();
    if (setting_hour!=0) break;
  }
  Serial.println("GALA3");
  distance();
  Serial.println("START");  
  digitalWrite(ledPin, LOW);
  digitalWrite(laserPin, LOW);

  for(;remain>=0;remain--)
  {
//      display.showNumberDec(remain);
        display.showNumberDecEx(remain, 0b11100000, true, 4, 0);
      delay(1000);
  }
  digitalWrite(ledPin, HIGH);
  digitalWrite(laserPin, HIGH);
  delay(1000);
    Serial.println("RING");  
  ringTone(buzzerPin);  
  find_lowest();
  while(true){
    CDSVal = analogRead(CDSPin);
    Serial.println("READ");  
    Serial.println(CDSVal);  
    if (CDSVal<100){ break;}
    delay(2000); //每2s讀取一次   
    
  }
}
