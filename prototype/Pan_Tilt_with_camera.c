#include<Servo.h>
Servo servoVer; //Vertical Servo
Servo servoHor; //Horizontal Servo
int x = 90;
int y = 90;
int prevX = 90;
int prevY = 90;
int cam_x = 640/2;
int cam_y = 480/2;
void setup()
{
  Serial.begin(9600);
  servoVer.attach(5); //Attach Vertical Servo to Pin 5
  servoHor.attach(6); //Attach Horizontal Servo to Pin 6
  servoVer.write(90);
  servoHor.write(90);
}
void Pos()
{
  if(prevX != x || prevY != y)
  {
    int servoX = map(x, 600, 0, 70, 179);
    int servoY = map(y, 450, 0, 179, 95);
    servoX = min(servoX, 179);
    servoX = max(servoX, 70);
    servoY = min(servoY, 179);
    servoY = max(servoY, 95);
    
    servoHor.write(servoX);
    servoVer.write(servoY);
    prevX=x;
    prevY=y;
  }
}
void loop()
{
//      servoVer.write(90);
//  for (x=0; x<=179; x+=10){
//      servoHor.write(x);
//      delay(2000);
//  }
//  servoHor.write(90);
//    for (y=0; y<=179; y+=10){
//      servoVer.write(y);
//      delay(2000);
//    }
  if(Serial.available() > 0)
  {
    if(Serial.read() == 'X')
    {
      cam_x = Serial.parseInt();
      if(Serial.read() == 'Y')
      {
        cam_y = Serial.parseInt();
        if (cam_x-320>10) cam_x = 1;
        else if (cam_x-320<-10) cam_x = -1;
        else cam_x = 0;// (cam_x - 600/2)/200;
        
        if (cam_y-240>10) cam_y = -1;
        else if (cam_y-240<-10) cam_y = 1;
        else cam_y = 0;// (cam_y - 450/2)/100;
        x = x - cam_x;
        y = y - cam_y;
        if(prevX != x || prevY != y)
        {
          servoHor.write(x);
          servoVer.write(y);
          delay(10000);
          prevX=x;
          prevY=y;
        }
        
        
      }
    }
    while(Serial.available() > 0)
    {
      Serial.read();
    }
  }
}
