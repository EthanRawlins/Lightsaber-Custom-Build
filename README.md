# Lightsaber Custom Build based on Alex Gyver's GyverSaber repo
* [Project description](#chapter-0)
* [Folders](#chapter-1)
* [Schemes](#chapter-2)
* [Components](#chapter-3)
* [Assembly and set up](#chapter-7)

<a id="chapter-0"></a>
## Project description
ARDUINO BASED COLOR-CHANGING LIGHTSABER WITH SOUND

#### CAPABILITIES:
* Smooth turning on/off with lightsaber-like sound effect
* Sounds:
  + MODE 1: generated hum. Frequency depends on angle velocity of blade
  + MODE 2: hum sound from SD card
    - Slow swing - long hum sound (randomly from 4 sounds)
    - Fast swing - short hum sound (randomly from 5 sounds)
* Bright white flash when hitting
* Play one of 16 hit sounds, when hit
  + Weak hit - short sound
  + Hard hit - long sound
* After power on blade shows current battery level from 0 to 100 percent
* Battery safe mode:
  + If battery is drained, and attempting to TURN ON: Light Saber will not turn on, button LED will PULSE a couple of times
  + If battery is drained, and Light Saber is ALREADY ON: Light Saber will be turned off automatically
#### CONTROL BUTTON:
* HOLD - turn on / turn off Light Saber
* DOUBLE CLICK - change color (blue - red - pink - purple - green - cyan - orange - yellow)
* QUADRUPLE CLICK - change sound mode (hum generation - hum playing)
* Selected color and sound mode stored in EEPROM (non-volatile memory)

<a id="chapter-1"></a>
## Folders
- **libraries** - libraries
- **GyverSaber** - original arduino sketch
- **Customized-Lightsaber-Controller** - modified arduino sketch
- **Testing-generating-lightsaber-sounds-mozzi** - Testing audio generation arduino sketch
- **SDsounds** - Sound files
- **schemes** - wiring diagrams for original and modified configuration

<a id="chapter-2"></a>
## Schemes
![SCHEME](https://github.com/EthanRawlins/Lightsaber-Custom-Build/blob/main/schemes/LightsaberCustomBuildWiringDiagram.jpg)

<a id="chapter-3"></a>
## Components
* Arduino NANO http://ali.pub/20o35g  http://ali.pub/20o36t
* Addressable LED strip. WS2812B, 5V. Take **white PCB, IP30, 60 LEDs per meter**
  https://www.amazon.com/gp/product/B09PBJ92FV/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
* Button with LED. **Take 5V version** http://ali.pub/23ct29
* MPU6050 http://ali.pub/23mryw  http://ali.pub/23mst1
* Cheap MicroSD http://ali.pub/23msne  http://ali.pub/23msqp
* MicroSD module mini http://ali.pub/23ms27  http://ali.pub/23ms5b
* Or this http://ali.pub/23ms11
* Battery 3.7v LiPo https://www.amazon.com/gp/product/B08215B4KK/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
* Amplifier http://ali.pub/23mp6d  http://ali.pub/23mp75  http://ali.pub/23mp7q
* Speaker http://ali.pub/23mq8h  http://ali.pub/23mq9g  http://ali.pub/23mqc6
* Resistors KIT http://ali.pub/23mqei  http://ali.pub/23mqfo
* Power button http://ali.pub/23mtiw
* Charging port USB C https://www.amazon.com/gp/product/B07PKND8KG/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
* Prototype board http://ali.pub/23mrwy  

<a id="chapter-7"></a>
## Assembly and set up
* [Quick start with Arduino](https://learn.sparkfun.com/tutorials/installing-arduino-ide)
* Open GyverSaber.ino and tune:
  - Number of microcircuits WS2812B on LED strip
  - Turn on or turn off blade pulsation
  - Hardly recommend measure real resistance of voltage divider resistors
    + System can work without battery monitoring, just deactivate BATTERY_SAFE. **BUT IT IS NOT RECOMMENDED**
* Flash arduino
* MicroSD info:
  - Size < 4G
  - Format to FAT
  - Copy audiofiles **in the root**
  - If you want add your own sounds, convert them to .WAV:
    + 8 bit
	+ 16-32 kHz
	+ Mono
	+ Use online converters or Total Audio Converter
* Assemble scheme
* Enjoy!

## GyverSaber settings
    NUM_LEDS 124         // number of LEDs WS2812B on LED strip
    BTN_TIMEOUT 800     // button hold delay, ms
    BRIGHTNESS 255      // max LED brightness (0 - 255)

    SWING_TIMEOUT 500   // timeout between swings
    SWING_L_THR 150     // swing angle speed threshold
    SWING_THR 300       // fast swing angle speed threshold
    STRIKE_THR 150      // hit acceleration threshold
    STRIKE_S_THR 320    // hard hit acceleration threshold
    FLASH_DELAY 80      // flash time while hit

    R1 100000           // voltage divider real resistance
    R2 51000            // voltage divider real resistance
    BATTERY_SAFE 1      // battery monitoring (1 - allow, 0 - disallow)

    DEBUG 0             // debug information in Serial (1 - allow, 0 - disallow)

Check out Alex Gyver's original repo here: https://github.com/AlexGyver/EnglishProjects/tree/master/GyverSaber