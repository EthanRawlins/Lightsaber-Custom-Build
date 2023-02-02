/*
     ETHAN RAWLINS CUSTOMIZED LIGHTSABER CONTROLLER FORKED FROM "GYVERSABER PROJECT BY ALEX GYVER"
   HARDWARE:
     * Addressable LED strip (WS2812B) to get any blade color and smooth turn on effect
     * Microsd card module to play some sounds
     * IMU MPU6050 (accel + gyro) to generate hum. Frequency depends on angle velocity of blade
       OR measure angle speed and play some hum sounds from sd
     * 3W speaker
     * 3.3V speaker amp module
     * 3.7W LiPo battery
     * Arduino Pro Micro, 3.3V, 8Mhz
     * Toggle switch for powering all components on/off
     * Momentary switch button for turning saber (lights and sounds) on/off
     * Lightsaber Handle to fit all electronics inside
     * 1" OD Polycarbonate tube for blade
   CAPABILITIES:
     Smooth turning on/off with lightsaber-like sound effect (lights on blade crawl up/down when turned on/off)
     Sounds:
       MODE 1: generated hum. Frequency depends on angle velocity of blade
       MODE 2: hum sound from sd card
         Slow swing - long hum sound (randomly from 4 sounds)
         Fast swing - short hum sound (randomly from 5 sounds)
         Play one of 16 hit sounds, when hit
          Weak hit - short sound
          Hard hit - long sound
     Bright white flash when hitting
     After power on blade shows current battery level from 0 to 100 percent (percent of blade lit up indicates charge level)
     Battery safe mode:
       If battery is drained, and attempting to TURN ON: Light Saber will not turn on, button LED will PULSE a couple of times
       If battery is drained, and Light Saber is ALREADY ON: Light Saber will be turned off automatically
   CONTROL BUTTON:
     HOLD - turn on / turn off Light Saber
     DOUBLE CLICK - cycle color (blue, red, pink, purple, green, cyan, orange, yellow)
     TRIPLE CLICK - nothing for now
     QUADRUPLE CLICK - change sound mode (hum generation - SD card audio files)
     Selected color and sound mode stored in EEPROM (non-volatile memory)
*/

// ---------------------------- SETTINGS -------------------------------
#define NUM_LEDS 124        // number of LEDs WS2812B on LED strip
#define BTN_TIMEOUT 800     // button hold delay, ms
#define BRIGHTNESS 255      // max LED brightness (0 - 255)

#define SWING_TIMEOUT 500   // timeout between swings
#define SWING_L_THR 150     // slow swing angle speed threshold
#define SWING_THR 300       // fast swing angle speed threshold
#define STRIKE_THR 120      // hit acceleration threshold (150 default)
#define STRIKE_S_THR 180    // hard hit acceleration threshold (320 default)
#define FLASH_DELAY 80      // flash time while hit
#define TONE_HIT_FREQ 200   // tone freq while hit

#define R1 100000           // voltage divider real resistance
#define R2 51000            // voltage divider real resistance
#define BATTERY_SAFE 1      // battery monitoring (1 - allow, 0 - disallow)

#define DEBUG 1             // debug information in Serial (1 - allow, 0 - disallow)
// ---------------------------- SETTINGS -------------------------------

#define LED_PIN 6
#define BTN 3
#define VOLT_PIN A6
#define BTN_LED 8
#define SD_ChipSelectPin 4

// -------------------------- LIBS ---------------------------
#include <avr/pgmspace.h>   // PROGMEM library
#include <SD.h>
#include <TMRpcm.h>         // audio from sd library
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <toneAC.h>         // hum generation library
#include "FastLED.h"        // addressable LED library
#include <EEPROM.h>

CRGB leds[NUM_LEDS];
TMRpcm sd_audio;
MPU6050 accelgyro;
// -------------------------- LIBS ---------------------------


// ------------------------------ VARIABLES ---------------------------------
int16_t ax, ay, az;
int16_t gx, gy, gz;
unsigned long ACC, GYR, COMPL;
int gyroX, gyroY, gyroZ, accelX, accelY, accelZ, freq, freq_f = 20;
float k = 0.2;
unsigned long humTimer = -9000, mpuTimer, nowTimer;
int stopTimer;
boolean bzzz_flag, ls_chg_state, ls_state;
boolean btnState, btn_flag, hold_flag;
byte btn_counter;
unsigned long btn_timer, PULSE_timer, swing_timer, swing_timeout, battery_timer, bzzTimer;
byte nowNumber;
byte LEDcolor;  // 0 - blue, 1 - red, 2 - pink, 3 - purple, 4 - green, 5 - cyan, 6 - orange, 7 - yellow
byte nowColor, red, green, blue, redOffset, greenOffset, blueOffset;
boolean eeprom_flag, swing_flag, swing_allow, strike_flag, HUMmode;
float voltage;
int PULSEOffset;
// ------------------------------ VARIABLES ---------------------------------

// --------------------------------- SOUNDS ----------------------------------
const char strike1[] PROGMEM = "SK1.wav";
const char strike2[] PROGMEM = "SK2.wav";
const char strike3[] PROGMEM = "SK3.wav";
const char strike4[] PROGMEM = "SK4.wav";
const char strike5[] PROGMEM = "SK5.wav";
const char strike6[] PROGMEM = "SK6.wav";
const char strike7[] PROGMEM = "SK7.wav";
const char strike8[] PROGMEM = "SK8.wav";

const char* const strikes[] PROGMEM  = {
  strike1, strike2, strike3, strike4, strike5, strike6, strike7, strike8
};

int strike_time[8] = {779, 563, 687, 702, 673, 661, 666, 635};

const char strike_s1[] PROGMEM = "SKS1.wav";
const char strike_s2[] PROGMEM = "SKS2.wav";
const char strike_s3[] PROGMEM = "SKS3.wav";
const char strike_s4[] PROGMEM = "SKS4.wav";
const char strike_s5[] PROGMEM = "SKS5.wav";
const char strike_s6[] PROGMEM = "SKS6.wav";
const char strike_s7[] PROGMEM = "SKS7.wav";
const char strike_s8[] PROGMEM = "SKS8.wav";

const char* const strikes_short[] PROGMEM = {
  strike_s1, strike_s2, strike_s3, strike_s4,
  strike_s5, strike_s6, strike_s7, strike_s8
};
int strike_s_time[8] = {270, 167, 186, 250, 252, 255, 250, 238};

const char swing1[] PROGMEM = "SWS1.wav";
const char swing2[] PROGMEM = "SWS2.wav";
const char swing3[] PROGMEM = "SWS3.wav";
const char swing4[] PROGMEM = "SWS4.wav";
const char swing5[] PROGMEM = "SWS5.wav";

const char* const swings[] PROGMEM  = {
  swing1, swing2, swing3, swing4, swing5
};
int swing_time[8] = {389, 372, 360, 366, 337};

const char swingL1[] PROGMEM = "SWL1.wav";
const char swingL2[] PROGMEM = "SWL2.wav";
const char swingL3[] PROGMEM = "SWL3.wav";
const char swingL4[] PROGMEM = "SWL4.wav";

const char* const swings_L[] PROGMEM  = {
  swingL1, swingL2, swingL3, swingL4
};
int swing_time_L[8] = {636, 441, 772, 702};

char BUFFER[10];
// --------------------------------- SOUNDS ---------------------------------

void setup() {
  // ----------------------------------------------------------------------------------------------------------------- Changed this from WS2811 to WS2812B ----------------------------------------------------------------------
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(100);  // 100% of LED strip brightness
  setAll(0, 0, 0);             // and turn it off

  Wire.begin();
  Serial.begin(9600);

  // ---- SETTING PINS ----
  pinMode(BTN, INPUT_PULLUP);
  pinMode(BTN_LED, OUTPUT);
  digitalWrite(BTN_LED, 1);
  // ---- SETTING PINS ----

  randomSeed(analogRead(2));    // starting point for random generator

  // IMU initialization
  accelgyro.initialize();
  accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
  accelgyro.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  if (DEBUG) {
    if (accelgyro.testConnection()) Serial.println(F("MPU6050 OK"));
    else Serial.println(F("MPU6050 fail"));
  }

  // sd initialization
  sd_audio.speakerPin = 9;
  if (DEBUG) {
    if (!SD.begin(SD_ChipSelectPin)) Serial.println(F("sd Fail"));
    else Serial.println(F("sd OK"));
  } else {
    SD.begin(SD_ChipSelectPin);
  }
  sd_audio.setVolume(4);

  if ((EEPROM.read(0) >= 0) && (EEPROM.read(0) <= 5)) {  // check first start
    nowColor = EEPROM.read(0);   // remember color
    HUMmode = EEPROM.read(1);    // remember mode
  } else {                       // first start
    EEPROM.write(0, 0);          // set default
    EEPROM.write(1, 0);          // set default
    nowColor = 0;                // set default
  }

  setColor(nowColor);
  byte capacity = voltage_measure();       // get battery level
  capacity = map(capacity, 100, 0, (NUM_LEDS - 1), 1);  // convert into blade length
  if (DEBUG) {
    Serial.print(F("Battery: "));
    Serial.println(capacity);
  }

  for (char i = 0; i <= capacity; i++) {   // show battery level
    setPixel(i, red, green, blue);
    FastLED.show();
    delay(5);
  }
  delay(500);                         // 1 second to show battery level
  setAll(0, 0, 0);
  FastLED.setBrightness(BRIGHTNESS);   // set bright
}

// --- MAIN LOOP---
void loop() {
  getFreq();
  on_off_sound();
  btnTick();
  strikeTick();
  swingTick();
  batteryTick();
}
// --- MAIN LOOP---

void btnTick() {
  btnState = !digitalRead(BTN);
  if (btnState && !btn_flag) {
    if (DEBUG) Serial.println(F("BTN PRESS"));
    btn_flag = 1;
    btn_counter++;
    btn_timer = millis();
  }
  if (!btnState && btn_flag) {
    btn_flag = 0;
    hold_flag = 0;
  }
  // if the button is held down
  if (btn_flag && btnState && (millis() - btn_timer > BTN_TIMEOUT) && !hold_flag) {
    ls_chg_state = 1;                     // flag to change saber state (on/off)
    hold_flag = 1;
    btn_counter = 0;
    if (DEBUG) Serial.println("BTN HELD DOWN");
  }
//  if (ls_state) toneAC(freq_f);
  // if the button was clicked multiple times before the timeout
  if ((millis() - btn_timer > BTN_TIMEOUT) && (btn_counter != 0)) {
    Serial.println("HERE");
    if (ls_state) {
      Serial.println(btn_counter);
      if (btn_counter == 2) {               // double press

        if (DEBUG) Serial.println(F("Double Button Press"));
        nowColor++;                         // cycle color
        if (nowColor >= 8) nowColor = 0;
        setColor(nowColor);
        setAll(red, green, blue);
        eeprom_flag = 1;
      }
      if (btn_counter == 3) {               // triple press
        if (DEBUG) Serial.println(F("Triple Button Press"));
      }
      if (btn_counter == 4) {               // quadruple press

        if (DEBUG) Serial.println(F("Quadruple Button Press"));
        HUMmode = !HUMmode;
        if (HUMmode) {
          noToneAC();
          sd_audio.play("HUM.wav");
        } else {
          sd_audio.disable();
          toneAC(freq_f);
        }
        eeprom_flag = 1;
      }
      btn_counter = 0;
    }
  }
}

void on_off_sound() {
  if (ls_chg_state) {                // if change flag
    if (!ls_state) {                 // if GyverSaber is turned off
      if (voltage_measure() > 10 || !BATTERY_SAFE) {
        if (DEBUG) Serial.println(F("SABER ON"));
        if (HUMmode) {
          sd_audio.play("ON.wav");
        }
//        else tone_on_sound();
        if (DEBUG) Serial.println(F("Playing On Sound"));
        delay(200);
        light_up();
        if (DEBUG) Serial.println(F("light up"));
        delay(200);
        bzzz_flag = 1;
        if (DEBUG) Serial.println(F("bzzz flag"));
        ls_state = true;               // remember that Light Saber is on
        if (HUMmode) {
          noToneAC();
          sd_audio.play("HUM.wav");
          if (DEBUG) Serial.println("playing hum.wav");
        } else {
          sd_audio.disable();
          toneAC(freq_f);
          if (DEBUG) Serial.println("playing toneAC");
        }
      } else {
        if (DEBUG) Serial.println(F("LOW VOLTAGE!"));
        for (int i = 0; i < 5; i++) {
          digitalWrite(BTN_LED, 0);
          delay(400);
          digitalWrite(BTN_LED, 1);
          delay(400);
        }
      }
    } else {                          // if Light Saber is turned on
      bzzz_flag = 0;
      if (HUMmode) {
        sd_audio.play("OFF.wav");
      }
//      else tone_off_sound();
      delay(300);
      light_down();
      delay(300);
      noToneAC();
      sd_audio.disable();
      if (DEBUG) Serial.println(F("SABER OFF"));
      ls_state = false;
      if (eeprom_flag) {
        eeprom_flag = 0;
        EEPROM.write(0, nowColor);   // write color in EEPROM
        EEPROM.write(1, HUMmode);    // write mode in EEPROM
      }
    }
    ls_chg_state = 0;
  }

  if (((millis() - humTimer) > 9000) && bzzz_flag && HUMmode) {
    sd_audio.play("HUM.wav");
    humTimer = millis();
    swing_flag = 1;
    strike_flag = 0;
  }
  long delta = millis() - bzzTimer;
  if ((delta > 3) && bzzz_flag && !HUMmode) {
    if (strike_flag) {
      sd_audio.disable();
      strike_flag = 0;
    }
    toneAC(freq_f);
    bzzTimer = millis();
  }
}

void strikeTick() {
  if ((ACC > STRIKE_THR) && (ACC < STRIKE_S_THR)) {
//    if (!HUMmode) noToneAC();               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    nowNumber = random(8);               //****************************************************************************************** Uncomment this for sd Card Testing************************************
    // read the title of the track from PROGMEM               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes_short[nowNumber])));               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    sd_audio.play(BUFFER);               //****************************************************************************************** Uncomment this for sd Card Testing************************************
    hit_flash();
//    if (!HUMmode)               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//      bzzTimer = millis() + strike_s_time[nowNumber] - FLASH_DELAY;               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    else               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//      humTimer = millis() - 9000 + strike_s_time[nowNumber] - FLASH_DELAY;               //****************************************************************************************** Uncomment this for sd Card Testing************************************
    strike_flag = 1;
  }
  if (ACC >= STRIKE_S_THR) {
//    if (!HUMmode) noToneAC();               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    nowNumber = random(8);               //****************************************************************************************** Uncomment this for sd Card Testing************************************
    // read the title of the track from PROGMEM               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    strcpy_P(BUFFER, (char*)pgm_read_word(&(strikes[nowNumber])));               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//    sd_audio.play(BUFFER);               //****************************************************************************************** Uncomment this for sd Card Testing************************************
    hit_flash();
//    if (!HUMmode)               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//      bzzTimer = millis() + strike_time[nowNumber] - FLASH_DELAY;
//    else               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//      humTimer = millis() - 9000 + strike_time[nowNumber] - FLASH_DELAY;               //****************************************************************************************** Uncomment this for sd Card Testing************************************
    strike_flag = 1;
  }
}

void swingTick() {
  if (GYR > 80 && (millis() - swing_timeout > 100) && HUMmode) {
    swing_timeout = millis();
    if (((millis() - swing_timer) > SWING_TIMEOUT) && swing_flag && !strike_flag) {
      if (GYR >= SWING_THR) {
        nowNumber = random(5);
        // read the title of the track from PROGMEM
        strcpy_P(BUFFER, (char*)pgm_read_word(&(swings[nowNumber])));
        sd_audio.play(BUFFER);
        humTimer = millis() - 9000 + swing_time[nowNumber];
        swing_flag = 0;
        swing_timer = millis();
        swing_allow = 0;
      }
      if ((GYR > SWING_L_THR) && (GYR < SWING_THR)) {
        nowNumber = random(5);
        // read the title of the track from PROGMEM               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//        strcpy_P(BUFFER, (char*)pgm_read_word(&(swings_L[nowNumber])));               //****************************************************************************************** Uncomment this for sd Card Testing************************************
//        sd_audio.play(BUFFER);                             //****************************************************************************************** Uncomment this for sd Card Testing************************************
        humTimer = millis() - 9000 + swing_time_L[nowNumber];
        swing_flag = 0;
        swing_timer = millis();
        swing_allow = 0;
      }
    }
  }
}

void getFreq() {
  if (ls_state) {                                               // if GyverSaber is on
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
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

void light_up() {
  for (char i = 0; i <= (NUM_LEDS - 1); i++) {        
    setPixel(i, red, green, blue);
//    setPixel((NUM_LEDS - 1 - i), red, green, blue);
    setPixel(i, red, green, blue);
    FastLED.show();
    toneAC(int(i) / 2);
    delay(2);
  }
}

void light_down() {
  for (char i = (NUM_LEDS - 1); i >= 0; i--) {      
    setPixel(i, 0, 0, 0);
//    setPixel((NUM_LEDS - 1 - i), 0, 0, 0);
    setPixel(i, 0, 0, 0);
    FastLED.show();
    toneAC(int(i) / 2);
    delay(2);
  }
}

void hit_flash() {
  setAll(255, 255, 255);       
  if (!HUMmode) {
    toneAC(TONE_HIT_FREQ);   
    delay(FLASH_DELAY);
  }
//  delay(FLASH_DELAY);                
  setAll(red, blue, green);        
}

void setColor(byte color) {
  switch (color) {
    // 0 - blue, 1 - red, 2 - pink, 3 - purple, 4 - green, 5 - cyan, 6 - orange, 7 - yellow
    case 0:
      red = 0;
      green = 0;
      blue = 255;
      break;
    case 1:
      red = 255;
      green = 0;
      blue = 0;
      break;
    case 2:
      red = 255;
      green = 0;
      blue = 70;
      break;
    case 3:
      red = 200;
      green = 0;
      blue = 200;
      break;
    case 4:
      red = 0;
      green = 255;
      blue = 0;
      break;
    case 5:
      red = 0;
      green = 255;
      blue = 255;
      break;
    case 6:
      red = 255;
      green = 30;
      blue = 0;
      break;
    case 7:
      red = 255;
      green = 200;
      blue = 0;
      break;
  }
}

void batteryTick() {
  if (millis() - battery_timer > 30000 && ls_state && BATTERY_SAFE) {
    if (voltage_measure() < 15) {
      ls_chg_state = 1;
    }
    battery_timer = millis();
  }
}

byte voltage_measure() {
  voltage = 0;
  for (int i = 0; i < 10; i++) {    
    voltage += (float)analogRead(VOLT_PIN) * 3.7 / 1023 * (R1 + R2) / R2;
  }
  voltage = voltage / 10;           
  int volts = (voltage * 100) + 19;
  if (DEBUG) Serial.print(volts);
  if (volts > 387)
    return map(volts, 420, 387, 100, 77);
  else if ((volts <= 387) && (volts > 375) )
    return map(volts, 387, 375, 77, 54);
  else if ((volts <= 375) && (volts > 368) )
    return map(volts, 375, 368, 54, 31);
  else if ((volts <= 368) && (volts > 340) )
    return map(volts, 368, 340, 31, 8);
  else if (volts <= 340)
    return map(volts, 340, 260, 8, 0);
}
