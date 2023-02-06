/*
   Example using a piezo to trigger an audio sample to play,
   and a knob to set the playback pitch,
   with Mozzi sonification library.

   Demonstrates one-shot samples and analog input for control.

   This example goes with a tutorial on the Mozzi site:
   http://sensorium.github.io/Mozzi/learn/introductory-tutorial/

   The circuit:
     Audio output on digital pin 9 on a Uno or similar, or
    DAC/A14 on Teensy 3.1, or
     check the README or http://sensorium.github.io/Mozzi/

     Potentiometer connected to analog pin 0.
       Center pin of the potentiometer goes to the analog pin.
       Side pins of the potentiometer go to +5V and ground

     Piezo on analog pin 3:
       + connection of the piezo attached to the analog pin
       - connection of the piezo attached to ground
       1-megohm resistor between the analog pin and ground

    Mozzi documentation/API
    https://sensorium.github.io/Mozzi/doc/html/index.html

    Mozzi help/discussion/announcements:
    https://groups.google.com/forum/#!forum/mozzi-users

   Tim Barrass 2013.
   CC by-nc-sa
*/

#include <MozziGuts.h>
#include <Sample.h> // Sample template
#include <samples/burroughs1_18649_int8.h> // a converted audio sample included in the Mozzi download
#include "MPU6050.h"

#define SWING_TIMEOUT 500   // timeout between swings
#define SWING_L_THR 150     // swing angle speed threshold
#define SWING_THR 300       // fast swing angle speed threshold
#define STRIKE_THR 150      // hit acceleration threshold
#define STRIKE_S_THR 320    // hard hit acceleration threshold
#define FLASH_DELAY 80      // flash time while hit

int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR, COMPL, mpuTimer;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
float k = 0.2;

const int threshold = 80;  // threshold value to decide when the detected signal is a knock or not

// use: Sample <table_size, update_rate> SampleName (wavetable)
Sample <BURROUGHS1_18649_NUM_CELLS, AUDIO_RATE> aSample(BURROUGHS1_18649_DATA);
float recorded_pitch = (float) BURROUGHS1_18649_SAMPLERATE / (float) BURROUGHS1_18649_NUM_CELLS;

boolean triggered = false;

MPU6050 accelgyro;

void setup(){
  Serial.begin(9600); // for Teensy 3.1, beware printout can cause glitches
//  Serial.begin(115200); // set up the Serial output so we can look at the piezo values // set up the Serial output so we can look at the piezo values
  // IMU initialization
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  if (accelgyro.testConnection()) Serial.println(F("MPU6050 OK"));
  else Serial.println(F("MPU6050 fail"));
  
  startMozzi(); // :))
}


void updateControl(){
  // read the knob
  int movement_value = freq_f; // value is 0-1023

  // map it to values between 0.1 and about double the recorded pitch
  float pitch = (recorded_pitch * (float) movement_value / 512.f) + 0.1f;

  // set the sample playback frequency
  aSample.setFreq(pitch);

// only trigger once each time the piezo goes over the threshold
  aSample.start();

  Serial.println(); // next line
}


void getFreq() {
  if (millis() - mpuTimer > 500) {                            
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);       

    // find absolute and divide by 100
    gyroX = abs(gx / 100);
    gyroY = abs(gy / 100);
    gyroZ = abs(gz / 100);
    accelX = abs(ax / 100);
    accelY = abs(ay / 100);
    accelZ = abs(az / 100);

    // vector sum
    ACC = sq((long)accelX) + sq((long)accelY) + sq((long)accelZ);
    ACC = sqrt(ACC);
    GYR = sq((long)gyroX) + sq((long)gyroY) + sq((long)gyroZ);
    GYR = sqrt((long)GYR);
    COMPL = ACC + GYR;
//      // debugging the IMU
//      Serial.print("$");
//      Serial.print(gyroX);
//      Serial.print(" ");
//      Serial.print(gyroY);
//      Serial.print(" ");
//      Serial.print(gyroZ);
//      Serial.println(";");
    freq = (long)COMPL * COMPL / 1500;                        // parabolic tone change
    freq = constrain(freq, 18, 300);                          
    freq_f = freq * k + freq_f * (1 - k);                     // smooth filter
//      if (DEBUG) Serial.print(freq_f);
    mpuTimer = micros();                                     
  }
}


int updateAudio(){
  return aSample.next();
}


void loop(){
  getFreq();
  audioHook();
}
