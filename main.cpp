/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "LEDService.h"

#include "Servo.h"
#include "buzzer.h"
#include "HCSR04.h"
#include "TM1637.h" 

#define CLK PA_15                     // D9
#define DIO PB_2                      // D8
#define ledPin PA_4                   // D7
#define laserPin PA_3                 // D4
#define SERVO_PIN PD_14               // D2
#define buzzerPin PA_2                // D10
#define trigPin PB_4                  // D5
#define echoPin PB_1                  // D6
#define CDSPin A0                 // light sensor in pin A0

DigitalOut alivenessLED(LED1, 0);
DigitalOut actuatedLED(LED2, 0);
Serial pc(USBTX, USBRX);
Beep buzzer(buzzerPin);
HCSR04 sonar(trigPin, echoPin);

// Setup: Pin Mode In/Out
DigitalOut myled(LED1);
DigitalOut out_led(PA_4);
DigitalOut out_laser(PA_3);
PwmOut servoVer(SERVO_PIN);
AnalogIn light_sensor(CDSPin);

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
int CDSVal; // initial value for light sensor = 0

// TODO: replace the virtual timing with RTC clock
int hr = 15;
int minutes = 35;
int sec = 40;

const static char     DEVICE_NAME[] = "AwesomeAlarm";
static const uint16_t uuid16_list[] = {LEDService::LED_SERVICE_UUID};
int i = 0;
int ble_read_value = 0;
static EventQueue eventQueue(/* event count */ 10 * EVENTS_EVENT_SIZE);
bool end_ring = 0;
bool display_time = 1;
// Define Threads
Thread* thread_ring = new Thread();  // ring tone while waiting for laser gun shot
Thread* thread_ble = new Thread();   // read ble while main program is executed
Thread* thread_scan = new Thread();  // scan the distance while count the time
Thread* thread_time = new Thread();  // ount the time and display it when main program is running

LEDService *ledServicePtr;

// Function: Buzzer Beep ring
void ringTone() {
  int num = 10;
  for (int j=0; j<num; j++) { //repeat num times
      for (int i=0; i<10; i++) { //repeat 10 times
        buzzer.beep(1000,0.05); 
        wait(0.05);
        buzzer.beep(500,0.05); 
        wait(0.05);
        }
      buzzer.nobeep();
      wait(2);
      if (end_ring) {
          end_ring = 0;
          break;
      }
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
}

void disconnectionCallback(const Gap::DisconnectionCallbackParams_t *params)
{
    (void) params;
    BLE::Instance().gap().startAdvertising();
}

void blinkCallback(void)
{
    alivenessLED = !alivenessLED; /* Do blinky on LED1 to indicate system aliveness. */
//    pc.printf("CallBack!\n");
}

/**
 * This callback allows the LEDService to receive updates to the ledState Characteristic.
 *
 * @param[in] params
 *     Information about the characterisitc being updated.
 */
void onDataWrittenCallback(const GattWriteCallbackParams *params) {

    ble_read_value = *(params->data);
    if ((params->handle == ledServicePtr->getValueHandle())) {
        actuatedLED = *(params->data);
        pc.printf("\nSet Values: ");
        pc.printf("%d ", *(params->data));
        pc.printf("Length: %d", params->len);
        pc.printf("\n");
    } else{
        pc.printf("\nFailed to Set Values: ");
        pc.printf("%d ", *(params->data));
        pc.printf("Length: %d", params->len);
        pc.printf("\n");
    }
        
}

/**
 * This function is called when the ble initialization process has failled
 */
void onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

/**
 * Callback triggered when the ble initialization process has finished
 */
void bleInitComplete(BLE::InitializationCompleteCallbackContext *params)
{
    BLE&        ble   = params->ble;
    ble_error_t error = params->error;

    if (error != BLE_ERROR_NONE) {
        /* In case of error, forward the error handling to onBleInitError */
        onBleInitError(ble, error);
        return;
    }

    /* Ensure that it is the default instance of BLE */
    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        return;
    }

    ble.gap().onDisconnection(disconnectionCallback);
    ble.gattServer().onDataWritten(onDataWrittenCallback);

    bool initialValueForLEDCharacteristic = false;
    ledServicePtr = new LEDService(ble, initialValueForLEDCharacteristic);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, (uint8_t *)uuid16_list, sizeof(uuid16_list));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();
}

void scheduleBleEventsProcessing(BLE::OnEventsToProcessCallbackContext* context) {
    BLE &ble = BLE::Instance();
    eventQueue.call(Callback<void()>(&ble, &BLE::processEvents));
}

void run_ble(){
    
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(scheduleBleEventsProcessing);
    ble.init(bleInitComplete);

    eventQueue.dispatch_forever();
}

void count_time() {
    while(1){
        wait(1);
        sec = sec + 1;
        if (sec == 60){
            minutes = minutes + 1;
            sec = 0;
        }
        if (minutes == 60) {
            hr = hr + 1;
            minutes = 0;
        }
        if (hr == 24) {
            hr = 0;
        }
        if (display_time)
        write_digits(100*hr + minutes);
    }
    
}


int main()
{
    eventQueue.call_every(500, blinkCallback);
    pc.printf("\nHelloWorld!\n");
    thread_ble->start(run_ble);
    thread_time->start(count_time);
        
    pc.printf("BLE Setup Done!\n");

    CATALEX.cls(); 
    CATALEX.writeData(all_str);
    CATALEX.setBrightness(TM1637_BRT3);
    
    // Start infine loop!
    while(1){
    while (ble_read_value==0){
        pc.printf("Waiting for write...\n");
        wait(1);
    }
    thread_scan->start(scan);
    display_time = 0;
    for(remain=ble_read_value;remain>=0;remain--)
    {
        write_digits(remain);
        wait(1);
    }
    lowest();
    display_time = 1;
    thread_ring->start(ringTone);
    out_led = 1;
    out_laser = 1;
//    printf("RTC example\n"); 
//    set_time(1387188323); // Set RTC time to 16 December 2013 10:05:23 UTC
//    printf("Date and time are set.\n");

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
    end_ring = 1;
    out_led = 0;
    out_laser = 0;
    ble_read_value=0;
    thread_ring->join();
    thread_scan->join();
    delete thread_ring;
    delete thread_scan;
    thread_ring = new Thread();
    thread_scan = new Thread();
    }
}
