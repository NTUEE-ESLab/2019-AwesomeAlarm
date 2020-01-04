#include "mbed.h"
#include "Servo.h"
//#include "DigitDisplay.h"
#include "buzzer.h"
#include "HCSR04.h"
#include "TM1637.h" 

#define CLK PA_15                     // D9
#define DIO PB_2                      // D8
#define ledPin PA_4                   // D7
#define laserPin PA_3                 // D4
#define SERVO_PIN PD_14               // D2
#define buzzerPin PA_2                // D10
#define trigPin PA_6                  // D12
#define echoPin PA_7                  // D11
#define CDSPin A0                 // light sensor in pin A0
int CDSVal; // initial value for light sensor = 0


//DigitDisplay display(CLK, DIO);
//Servo servoVer(SERVO_PIN);
Beep buzzer(buzzerPin);
Serial pc(USBTX,USBRX); 
HCSR04 sonar(trigPin, echoPin);

// Setup: Pin Mode In/Out
DigitalOut myled(LED1);
DigitalOut out_led(PA_4);
DigitalOut out_laser(PA_3);
PwmOut servoVer(SERVO_PIN);
AnalogIn light_sensor(CDSPin);
//display.setBrightness(0x0f);
//display.setColon(true);
//uint8_t data[] = { 0x0, 0x0, 0x0, 0x0 };
//pinMode(buzzerPin, OUTPUT);
//pinMode(ledPin, OUTPUT);
//pinMode(laserPin, OUTPUT);
//pinMode(SERVO_PIN, OUTPUT);
//pinMode(CDSPin, INPUT);
//Serial.begin(9600);
//I2CBT.begin(38400);
//pinMode(trigPin, OUTPUT);        //Define inputs and outputs 
//pinMode(echoPin, INPUT);
//servoVer.attach(SERVO_PIN, 544, 2400); 
//servoVer.write(start_pos);

//DisplayData_t size is 6 bytes (6 grids @ 8 segments)
char digits[] = {63,6,91,79,102,109,125,7,127,111};
char numdigits[4];

TM1637::DisplayData_t all_str  = {0xFF, 0xFF, 0xFF, 0xFF};  
TM1637::DisplayData_t data_bits = {0xFF, 0xFF, 0xFF, 0xFF};  

// KeyData_t size is 1 bytes  
TM1637::KeyData_t keydata; 

// TM1637 declaration, Select the desired type in TM1637_Config.h
TM1637 CATALEX(DIO, CLK);      //F401    


// Declare: Global Variables
long duration, cm, inches;
float my_dist[21];
int16_t remain = 5;
const float start_pos = 120.0;
const float end_pos = 60.0;

// Function: Buzzer Beep ring
void ringTone(int num) {
  for (int j=0; j<num; j++) { //repeat num times
      for (int i=0; i<10; i++) { //repeat 10 times
        buzzer.beep(1000,0.05); 
        wait(0.05);
        buzzer.beep(500,0.05); 
        wait(0.05);
        }
      buzzer.nobeep();
      wait(2);
  }
}

// Function convert digits to data bits and write to TM1637
void write_digits(int num){
        int i=0;
        while(i<4)
        {
            int digit = num % 10;
            num = num / 10;
            numdigits[i]=digit;
            i++;
        }
        TM1637::DisplayData_t dat = {digits[numdigits[3]],digits[numdigits[2]],digits[numdigits[1]],digits[numdigits[0]]};
        CATALEX.writeData(dat);
}


// Function to write Servo motor by degrees
void write_servo(int degree){
    servoVer.pulsewidth(0.0009+(0.0033-0.0009)*degree/240.0);
}

// Function: measure distance
void scan(){
  float dis = 0;
  int idx = 0;
  for(int i=start_pos; i>=end_pos; i-=5.0) {
      write_servo(i);
      wait(1);
      dis = sonar.getCm();
      if (dis < 20.0){
        wait(1);  
        dis = sonar.getCm();
      }
      printf("Angle %d degree  ", i);
      printf("Distance %.2f cm \n", dis); 
      my_dist[idx] = dis;
      idx += 1;
  }
}

// Function: measure distance
void lowest(){
  float dis = 0.0;
  int idx = 0;
  float new_dist = 0;
  float maximum = 0;
  int maximum_t = 0;
  int max_angle = 0;
  for(int i=start_pos; i>=end_pos; i-=5.0) {
      write_servo(i);
      wait(1);
      dis = sonar.getCm();
      if (dis < 20.0){
        printf("Distance %.2f cm , is too short\n", dis);
        wait(1);  
        dis = sonar.getCm();
      }
      printf("Angle %d degree  ", i);
      printf("Distance %.2f cm  ", dis);
      printf("Origin %.2f cm  ", my_dist[idx]); 
      new_dist = my_dist[idx] - dis;
      printf("Diff %.2f cm  \n", new_dist); 
      if (new_dist > maximum && new_dist < 20.0){
        maximum = new_dist;
        maximum_t = idx;
        max_angle = i;
      }
      idx += 1;
  }
  printf("MIN Angle %d degree  ", 120- 5*maximum_t);
  printf("MIN Distance %.2f cm  ", maximum);
  printf("MIN Origin %.2f cm\n", my_dist[maximum_t]); 
  idx = 0;
  write_servo(max_angle);
//  for(int i=start_pos; i>=end_pos; i-=5.0) {
//      write_servo(i);
//      wait(1);
//      if (idx==maximum_t) break;
//      idx += 1;
//  }
}


int main() {
    
    pc.printf("Hello World!\n");

    CATALEX.cls(); 
    CATALEX.writeData(all_str);
    CATALEX.setBrightness(TM1637_BRT3);
    scan();

    for(remain=5;remain>=0;remain--)
    {
        write_digits(remain);
        wait(1);
    }
    ringTone(2);
    out_led = 1;
    out_laser = 1;
    lowest();
    printf("RTC example\n"); 
//    set_time(1387188323); // Set RTC time to 16 December 2013 10:05:23 UTC
    printf("Date and time are set.\n");

    while(1) {

        time_t seconds = time(NULL);

        //printf("Time as seconds since January 1, 1970 = %d\n", seconds);
        
        printf("Time as a basic string = %s", ctime(&seconds));
        
        CDSVal = light_sensor.read_u16()/100;
        pc.printf("READ %d", CDSVal);  
        if (CDSVal<100) {break;}

        //char buffer[32];
        //strftime(buffer, 32, "%I:%M:%S %p\n", localtime(&seconds));
        //printf("Time as a custom formatted string = %s", buffer);

        myled = !myled;      
        wait(1);
    }
    
    out_led = 0;
    out_laser = 0;
    while(1);
}
