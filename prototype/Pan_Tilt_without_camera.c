#include<Servo.h>
Servo servoVer;


int trigPin = 12;                  //Trig Pin
int echoPin = 11;                  //Echo Pin
long duration, cm, inches;
 
void setup() {
  Serial.begin (9600);             // Serial Port begin
  pinMode(trigPin, OUTPUT);        //Define inputs and outputs 
  pinMode(echoPin, INPUT);
  servoVer.attach(9, 500, 2500); 
  servoVer.write(10);
}
 
void loop()
{
  for(int x = 50; x <= 150; x+=5){
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
    
    delay(500);
  }
    for(int x = 150; x >= 50; x-=5){
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
    
    delay(500);
  }
}
