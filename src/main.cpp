#include "mbed.h"
#include "Servo.h"
#include "DigitDisplay.h"
#include "buzzer.h"

#define CLK PA_15                     // D9
#define DIO PB_2                      // D8
#define ledPin PA_4                   // D7
#define laserPin PA_3                 // D4
#define SERVO_PIN PA_7                // D11
#define buzzerPin PA_2                // D10
#define trigPin PA_6                  // D12
#define echoPin PA_7                  // D11

int CDSPin = PC_5; // light sensor in pin A0
int CDSVal = 0; // initial value for light sensor = 0

DigitDisplay display(CLK, DIO);
//Servo servoVer(SERVO_PIN);
Beep buzzer(buzzerPin);

// Setup: Pin Mode In/Out
DigitalOut myled(LED1);
DigitalOut out_led(PA_4);
DigitalOut out_laser(PA_3);
PwmOut servoVer(SERVO_PIN);
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


// Declare: Global Variables
long duration, cm, inches;
int my_dist[21];
int remain = 5;
const int start_pos = 120;        // Start position for swapping
const int end_pos = 80;           // End position for swapping 

// Function: Buzzer Beep ring
void ringTone() {
  myled = 0;
  for (int i=0; i<10; i++) { //repeat 10 times
    buzzer.beep(1000,0.5); 
    buzzer.beep(500,0.5); 
    }
  buzzer.nobeep();
  wait(2);
  myled = 0;
}

int main() {
    ringTone();
    for(remain=100;remain>=0;remain--)
    {
        display.write(remain);
    }
    out_led = 1;
    out_laser = 1;
    while(1) {
      for(int i=75.0; i>50.0; i-=3.0) {
          servoVer.pulsewidth(0.0005+0.0019*i/90.0);
          wait(1);
      }
//      for(int i=100; i>0; i--) {
//           servoVer = i/100.0;
//           wait(0.01);
//      }
    }
    
    printf("RTC example\n"); 
    set_time(1387188323); // Set RTC time to 16 December 2013 10:05:23 UTC
    printf("Date and time are set.\n");

    while(1) {

        time_t seconds = time(NULL);

        //printf("Time as seconds since January 1, 1970 = %d\n", seconds);
        
        printf("Time as a basic string = %s", ctime(&seconds));

        //char buffer[32];
        //strftime(buffer, 32, "%I:%M:%S %p\n", localtime(&seconds));
        //printf("Time as a custom formatted string = %s", buffer);

        myled = !myled;      
        wait(1);
    }
}
