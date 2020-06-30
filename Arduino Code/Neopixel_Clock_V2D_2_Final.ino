/*
  Wanderingmoose Tinkering c2020
*****
  Neopixel Clock V2D_2 May 01, 2020
*****
Arduino Nano or Uno will work
  WS2812b LED Seven Segment Clock with DS18B20 temperature sensor.
  Orginal Code found here https://www.youtube.com/watch?v=LojGHyBFE8Q and is heavly moded for DS1307 RTC and DS18B20 temperature sensor.
  Set time on the DS1307 with seperate code. Best option for me at this time. (recommend doing this before running this code to make sure correct time will be displayed)
************

  LDR for light brighness control
  RTC DS1307 
  DS18B20 Temperature Sensor
  Three buttons
    -DST/Non-DST
    -24 hour to 12 hour
    -Color change
*/
#include <Wire.h>                 //
#include <FastLED.h>             //https://github.com/FastLED/FastLED
#include <OneWire.h>             // OneWire
#include <DallasTemperature.h>   // DS18B20
#include "RTClib.h"              // RTC

#define NUM_LEDS 86 // 3*7*4 +2  Number of LED controlled in the 4 digits with 2 led for colon
#define COLOR_ORDER GRB  // Define color order for your strip
#define LED_PIN 6 // Data pin for led comunication with WS2812b LEDS
#define DST_PIN 3  // Define DST adjust button pin --> Switch--> gnd
#define MTM_PIN 4   //Define Military or standard time  pin --> Switch--> gnd
#define DIS_Color_Pin 7  // Define Color change button pin --> Switch--> gnd
#define DS18B20_PIN 2   // Temperature communications pin for DS18S20 sensor
#define BRI_PIN A3  // Define Light sensor pin --> to voltage divider of 5VDC(LDR and 1Kohm) --> gnd 
RTC_DS1307 RTC;      // RTC Module
OneWire oneWire(DS18B20_PIN);          // OneWire Referenz setzen
DallasTemperature sensors(&oneWire);   // DS18B20 initialisieren

CRGB leds[NUM_LEDS]; // Define LEDs strip -----watch video mentions in discription for order of LEDS, I made my with three LEDS per segment instead of 4.
// 0,0,0 Blank Bar
// 1,1,1 Lit Bar
//    3
// 4    2
//    1
// 5    7
//    6
//  (1 2 3, 4 5 6, 7 8 9, 10 11 12, 13 14 15, 16 17 18, 19 20 21)
byte digits[13][21] = {
  {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Digit 0
  {0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}, // Digit 1
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0}, // Digit 2
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Digit 3
  {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1}, // Digit 4
  {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Digit 5
  {1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Digit 6
  {0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1}, // Digit 7
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // Digit 8
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // Digit 9 | 2D Array for numbers on 7 segment
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // Digit Degree Symbol
  {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0}, // Digit C
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  // Blank
};
bool Dot = true;  //Dot state
bool DST = false; //DST state
bool MTM = false; //Milatary or normal time
bool TempShow = true;
int last_digit = 0;

int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState5;         // current state of the button
int lastButtonState = 9;     // previous state of the button

long ledColor = CRGB::Green; // Color used (in hex) Had to use Green to get red displayed.
//Random colors i picked up
long ColorTable[16] = {
  CRGB::Amethyst,   //0
  CRGB::Aqua,       //1
  CRGB::Blue,       //2
  CRGB::Chartreuse, //3
  CRGB::DarkGreen,  //4
  CRGB::Pink,       //5
  CRGB::Purple,     //6
  CRGB::DeepPink,   //7
  CRGB::SaddleBrown,//8
  CRGB::Gold,       //9
  CRGB::GreenYellow,//10
  CRGB::LightCoral, //11
  CRGB::Tomato,     //12
  CRGB::OrangeRed,  //13
  CRGB::Red,        //14
  CRGB::DarkGoldenrod //15
};

//////////////////////////
void setup() {
  /////////////////////////
  Wire.begin();
  RTC.begin();
  sensors.begin();
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  LEDS.setBrightness(255); // Set initial brightness
  pinMode(DST_PIN, INPUT_PULLUP); // Define DST adjust button pin
  pinMode(MTM_PIN, INPUT_PULLUP); // Define Hours adjust button pin
  pinMode(DIS_Color_Pin, INPUT_PULLUP); //Define Color adjust button pin

}
//////////////////////////
// Get time in a single number, if hours will be a single digit then time will be displayed 155 instead of 0155
int GetTime() {
  /////////////////////////
  DateTime dtime = RTC.now(); // Get Current time
  int hour = dtime.hour();
  int minute = dtime.minute();
  int second = dtime.second();
  if (second % 2 == 0) {
    Dot = false;
  }
  else {
    Dot = true;
  };
  return (hour * 100 + minute);
};

//////////////////////////
// Convert time to array needet for display
void TimeToArray() {
  /////////////////////////
  int dtime = GetTime();  // Get time
  int cursor = 86; // last led number
  ledColor =  ColorTable[(buttonPushCounter)];
  if (DST) {  // if DST is true then add one hour
    dtime -= 100;
  };
  if (MTM) {
    if (dtime >= 1300) {
      dtime -= 1200;
    };
  };
  if (Dot) {
    leds[43] = ledColor;
    leds[42] = ledColor;
  }
  else  {
    leds[43] = 0x000000;
    leds[42] = 0x000000;
  };

  for (int i = 1; i <= 4; i++) {
    int digit = dtime % 10; // get last digit in time
    if (i == 1) {
      cursor = 65;
      for (int k = 0; k <= 20; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
      if (digit != last_digit)
      {
        ledColor =  ColorTable[(buttonPushCounter)];
      }
      last_digit = digit;

    }
    else if (i == 2) {
      cursor = 44;
      for (int k = 0; k <= 20; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
    }
    else if (i == 3) {
      cursor = 21;

      for (int k = 0; k <= 20; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
    }
    else if (i == 4) {
      cursor = 0;
      if (digit == 0) { // this section will make the first digit blank if zero.
        for (int k = 0; k <= 20; k++) {
          if (digits[12][k] == 1) {
            leds[cursor] = ledColor;
          }
          else if (digits[12][k] == 0) {
            leds[cursor] = 0x000000;
          };
          cursor ++;
        };
      }
      else
        for (int k = 0; k <= 20; k++) {
          if (digits[digit][k] == 1) {
            leds[cursor] = ledColor;
          }
          else if (digits[digit][k] == 0) {
            leds[cursor] = 0x000000;
          };
          cursor ++;
        };
    }//end of else if(i==4)
    dtime /= 10;
  };
};

//////////////////////////
// Convert temp to array needed for display
void TempToArray() {
  /////////////////////////
  DateTime dtime = RTC.now();
  ledColor = ColorTable[(buttonPushCounter)];
  if (dtime.second() != 20) {
    TempShow = false;
    return;
  }
  TempShow = true;
  sensors.requestTemperatures();
  int t = sensors.getTempCByIndex(0);
  int celsius = (t  * 100);
  int cursor = 86; // last led number

  leds[43] = 0x000000; //Turn off the colon on the display
  leds[42] = 0x000000;

  for (int i = 1; i <= 4; i++) {
    int digit = celsius % 10; // get last digit in temp
    if (i == 1) {

      cursor = 65; //fourth Digit

      for (int k = 0; k <= 20; k++) {
        if (digits[11][k] == 1) {
          leds[cursor] = ledColor;
        }
        else if (digits[11][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
    }
    else if (i == 2) {
      cursor = 44; //third digit

      for (int k = 0; k <= 20; k++) {
        if (digits[10][k] == 1) {
          leds[cursor] = ledColor;
        }
        else if (digits[10][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
    }
    else if (i == 3) {

      cursor = 21; //Second Digit

      for (int k = 0; k <= 20; k++) {
        if (digits[digit][k] == 1) {
          leds[cursor] = ledColor;
        }
        else if (digits[digit][k] == 0) {
          leds[cursor] = 0x000000;
        };
        cursor ++;
      };
    }
    else if (i == 4) {
      cursor = 0;
      if (digit == 0) { // this section will make the first digit blank if zero.
        for (int k = 0; k <= 20; k++) {
          if (digits[12][k] == 1) {
            leds[cursor] = ledColor;
          }
          else if (digits[12][k] == 0) {
            leds[cursor] = 0x000000;
          };
          cursor ++;
        };
      }
      else
        for (int k = 0; k <= 20; k++) {
          if (digits[digit][k] == 1) {
            leds[cursor] = ledColor;
          }
          else if (digits[digit][k] == 0) {
            leds[cursor] = 0x000000;
          };
          cursor ++;
        };

    }//end of else if(i==4)
    celsius /= 10;
  }
}

////////////////////////// Change time to DST from ST
void DSTcheck() {
  /////////////////////////
  int buttonDST = digitalRead(DST_PIN);
  if (buttonDST == LOW) {
    if (DST) {
      DST = false;
    }
    else if (!DST) {
      DST = true;
    };
    delay(250);
  };
}
////////////////////////// Change time to MTM or normal time
void MTMcheck() {
  /////////////////////////
  int buttonMTM = digitalRead(MTM_PIN);
  if (buttonMTM == LOW) {
    if (MTM) {
      MTM = false;
    }
    else if (!MTM) {
      MTM = true;
    };
    delay(250);
  };
}

///////////////////////////
/* coool effect function*/
void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  };
}
/////////////////////////
//////////////////////////
// Check Light sensor and set brightness accordingly
void BrightnessCheck()
/////////////////////////
{
  const byte sensorPin = BRI_PIN; // light sensor pin
  int sensorValue = analogRead(sensorPin); // Read sensor
  sensorValue = map(sensorValue, 0, 255, 10, 100); //maps sensor value so it will be scaled to 0-255
  LEDS.setBrightness(sensorValue);
}
///////////////////
// Check Button to changes color
void ColorChange()
{ // read the pushbutton up input pin:
  buttonState5 = digitalRead(DIS_Color_Pin);
  //buttonState5 = digitalRead(7);
  // compare the buttonState to its previous state
  if (buttonState5 != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState5 == LOW)
    {
      buttonPushCounter++;
    }
    delay(250);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState5;
  if (buttonPushCounter >= 16)
  {
    buttonPushCounter = 0;
  };
}

/////////////////////////
//////////////////
void loop()  // Main loop
/////////////////////////
{
  BrightnessCheck(); // Check brightness
  DSTcheck(); // Check DST
  MTMcheck(); //Check if Military or standard time is used
  ColorChange(); //Check for Color Change
  TimeToArray(); // Get leds array with required configuration
  TempToArray(); //Get leds array with required configuration
  FastLED.show(); // Display leds array
  if (TempShow == true) delay (8000);
}
