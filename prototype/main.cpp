// Hello World to sweep a servo through its full range
 
#include "mbed.h"
#include "Servo.h"
 
Servo myservo(PA_7);
 
int main() {    
    for(float p=-1.0; p<1.0; p += 0.2) {
        myservo = p;
        wait(1);
    }
}
 
