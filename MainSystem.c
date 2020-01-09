#include <Arduino.h>
#include <TM1637Display.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Servo.h>


int setting_hour = 0;
int setting_min = 0;
int setting_count = 0;
int insize; 
byte cmmd[20]; 
int receivedmessage[10];

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
int trigPin = 5;                  //Trig Pin
int echoPin = 6;                  //Echo Pin
float duration, cm, inches;
int my_dist[21];
void ringTone(int pin) {
  //return;
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
const int start_pos = 130;
const int end_pos = 90;
const int x_step = 3;
bool show_time = 0;

int time_now = 0;
void display_time(){
  if (show_time){
    time_now = setting_hour * 100 + setting_min;
    display.showNumberDecEx(time_now, 0b11100000, true, 4, 0);
  }
}



void distance()
{
    int t = 0;
    for(int x = start_pos; x >= end_pos; x-=x_step){
    servoVer.write(x);
    while (1){
      delay(200);
      
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);     // 給 Trig 高電位，持續 10微秒
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
  
      pinMode(echoPin, INPUT);             // 讀取 echo 的電位
      duration = pulseIn(echoPin, HIGH);   // 收到高電位時的時間
  
  
      Serial.print("Distance: ");
      Serial.println(duration*0.034/2);
  
  
     
      cm = (duration/2) / 29.1;         // 將時間換算成距離 cm 或 inch  
      inches = (duration/2) / 74; 
      if (cm>10) break;
    }
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
    for(int x = start_pos; x >= end_pos; x-=x_step){
      servoVer.write(x);
      Serial.print(x); 
      delay(1000);
      
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);     // 給 Trig 高電位，持續 10微秒
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
    
//      pinMode(echoPin, INPUT); /            // 讀取 echo 的電位
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


void setup()
{
  display.setBrightness(0x0f);
  uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
  pinMode(buzzerPin, OUTPUT);
  display.setSegments(data);
  Wire.begin(9);
  pinMode(ledPin, OUTPUT);
  pinMode(laserPin, OUTPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(CDSPin, INPUT);
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);        //Define inputs and outputs 
  pinMode(echoPin, INPUT);
  servoVer.attach(SERVO_PIN, 544, 2400); 
  servoVer.write(start_pos);
  
  Wire.onReceive(receiveEvent);
}

void receiveEvent(int howmany) {
  Serial.println(howmany);
  for(int i=0;i<howmany; i++){
    receivedmessage[i] = Wire.read(); // read one character from the I2C
    Serial.println("received");
    Serial.println(receivedmessage[i]);
  }
  setting_hour = receivedmessage[0];
  setting_min =  receivedmessage[1];
  setting_count = receivedmessage[2];
}
void loop()
{
  
  while(setting_count <= 1){
//    Serial.println(setting_count);/
    delay(500);
  }
  Serial.println("jaha");
  distance();
  Serial.println("START");  
  digitalWrite(ledPin, LOW);
  digitalWrite(laserPin, LOW);
  show_time = 0;
  for(remain=setting_count;remain>=0;remain--)
  {
//      display.showNumberDec(remain);
        display.showNumberDecEx(remain, 0b11100000, true, 4, 0);
      delay(1000);
  }
  show_time = 1;
  find_lowest();
  Serial.println("RING");  
  digitalWrite(ledPin, HIGH);
  digitalWrite(laserPin, HIGH);
  delay(100); 
  
  ringTone(buzzerPin); 
  ringTone(buzzerPin); 
  ringTone(buzzerPin); 
  while(true){
    CDSVal = analogRead(CDSPin);
    Serial.println("READ");  
    Serial.println(CDSVal);  
    if (CDSVal<100){ break;}
    delay(1000); //每2s讀取一次   
    display_time();
    
  }
 setting_count = 0;
display.showNumberDecEx(0, 0b11100000, true, 4, 0);
  digitalWrite(ledPin, LOW);
  digitalWrite(laserPin, LOW);
 
}
