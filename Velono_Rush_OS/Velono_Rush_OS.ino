/*

MAIN PROGRAM FOR THE VeloNOrush LARGE VERSION PROJECT
SENIOR PROJECT UNDER DR. YIN
CAL POLY POMONA, 2020-2021

-The function of this project is pressure based detection with
 fast and low-budget components. This project makes use of
 material that decreases in resistance as pressure/weight is
 applied. Using multiplexers, numerous detection zones are
 created to collect user location and pressure/weight. The
 main output of the system is on a matrix of individually
 addressable LEDs (WS2812B chipset). The LEDs are programmed/driven 
 using the publically available FastLED library.

*/

//Including any extra files
#include "VeloNOrush_Functions.h"
#include "VeloNOrush_Modes.h"

//Select the pins that will count for the A,B,C, and D
const byte muxPinA = 7;
const byte muxPinB = 8;
const byte muxPinC = 9;
const byte muxPinD = 10;

//Pins for button interactions
const byte recalibratePin = 4;

//specifications of power supply used
const byte psuVoltage = 5;            //supply value in volts
const uint32_t psuAmperage = 2000;    //supply value in milliamps

//constants to describe sensor matrix
const int numOfSensors = 96;
const int numOfRows = 12;
const int numOfCols = 8;
const int numOfRowsLED = 11;
const int numOfColsLED = 26;

//variables used for different operation modes
const byte currentMode = 0;                 //0 is position/weight tracking, 1 is jump

//loop variables
const int loopDelay = 8;              //125hz refresh rate

/*ALL FASTLED DEFINITIONS*/
#include <FastLED.h>
#define NUM_LEDS 286
#define LED_PIN 6
#define MAX_BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

//Create FastLED container for all the lights
CRGB matrixLEDs[NUM_LEDS];

//variables for FastLED lighting color
byte globalHue = 170;
byte globalSat = 255;
byte globalVal = 255;

//constants throughout the code
const int adcDelay = 100;                       //Delay for ADC cooldown in microseconds

//Instantiate the main core for performing system functions
VeloNOrushCore CoreFunctions(muxPinA,muxPinB,muxPinC,muxPinD);
JumpModeCore JumpFunctions;

/*
 * FUNCTION TO TEST EACH LED CONNECTED TO THE SYSTEM
 * POWERS ON EACH LED IN SEQUENCE, MAPPING ITS BRIGHTNESS
 * TO THE MAXIMUM POSSIBLE BRIGHTNESS OF THE SYSTEM
 */
void TestLEDs()
{
  //iterate through all LEDs in the matrix
  
  for (int i = 0; i < NUM_LEDS; i++)
  {
    matrixLEDs[i] = CHSV(globalHue, globalSat, globalVal/2);      //set to global hue, staturation and value
    FastLED.show();                                             //update each loop
    delay(5);                                                  //10ms delay between each light
  }

  //wait 1 second and blank the LEDs
  delay(1000);
  FastLED.clear();
  FastLED.show();
}

void setup() 
{
  //Initialize all pins
  pinMode(recalibratePin, INPUT_PULLUP);  //must be pressed to recalibrate maxes/mins
  pinMode(13, OUTPUT);                    //used to notify if power goes over limit
  
  //HANDLE ALL TASKS RELATED TO THE FASTLED LIBRARY
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(matrixLEDs, NUM_LEDS).setCorrection(TypicalLEDStrip);    //initialize LEDs
  FastLED.setBrightness(MAX_BRIGHTNESS);                                //set maximum brightness for leds, independent of color value
  FastLED.setMaxRefreshRate(120);                                        //set LED update rate limit to 60 frames per second
  set_max_power_in_volts_and_milliamps(psuVoltage, psuAmperage);        //limit the brightness of the LEDs to keep within power spec
  set_max_power_indicator_LED(13);                                      //notify the user if leds draw too much power
  FastLED.clear();                                                      //clear all LED data
  FastLED.show();                                                       //update LED matrix to blank it

  //initialize serial monitor
  Serial.begin(9600);
  
  //generate a seed for a random number generator
  randomSeed(analogRead(A0));

  //test all the leds in the chain
  TestLEDs();

  int recalibrateTries = 0;
  bool btnDetected = false;
  bool minsFlag, maxsFlag = false;
  while((recalibrateTries < 3000) && (btnDetected == false))
  {
    recalibrateTries+=1;
    if(digitalRead(recalibratePin) == LOW)
    {
        fill_solid(matrixLEDs, NUM_LEDS, CHSV(0, 255, MAX_BRIGHTNESS/2));
        FastLED.show();
        maxsFlag = CoreFunctions.CalibrateMaxes();
        minsFlag = CoreFunctions.CalibrateMins();
        if(minsFlag && maxsFlag)
          fill_solid(matrixLEDs, NUM_LEDS, CHSV(120, 255, MAX_BRIGHTNESS/2));
        else
          fill_solid(matrixLEDs, NUM_LEDS, CHSV(30, 255, MAX_BRIGHTNESS/2));
        FastLED.show();
        delay(1000);
        FastLED.clear();
        FastLED.show();
        btnDetected = true;
    }
    delay(1);
  }
  delay(1000);
}

void loop() 
{
  CoreFunctions.ReadSensors();
  //delay(3000);
  if(currentMode == 1)
  {
    JumpHandling();
  }
  else
  {
    CoreFunctions.MapSensorsToBrightness(globalVal);
    //Lastly, we set the LED brightnesses
    SetBrightnesses();
  }
  globalHue+=5;
  delay(loopDelay);
}

void JumpHandling()
{
  //check all of the sensor spots for a detected jump
  bool jumpFlag = false;
  byte rowCounter = 0;
  byte colCounter = 0;
  byte tempSensorVal = 0;
  byte tempMinVal = 0;
  while(jumpFlag == false && (rowCounter < numOfRows))
  {
    //tempSensorVal = CoreFunctions.GetSensorVal(rowCounter, colCounter);
    //tempMinVal = CoreFunctions.GetMin(rowCounter, colCounter);
    //jumpFlag = JumpFunctions.JumpCheck(tempSensorVal, tempMinVal, rowCounter, colCounter);

    /*TEMPORARY FORCING OF DETECTION FOR TESTING*/
    byte randCol = (uint8_t) random(numOfCols);
    byte randRow = (uint8_t) random(numOfRows);
    jumpFlag = JumpFunctions.JumpCheck(10, 10, randRow, randCol);
    Serial.print("Jumpflag: ");
    Serial.println(jumpFlag);
    if(colCounter == numOfCols-1)
    {
      rowCounter++;
      colCounter = 0;
    }
    else
    {
      colCounter++;
    }
  }

  if(jumpFlag == true)
  {
    //perform the illumination of the LEDs based off current frame
    Serial.println("Doing the animation.");
    while(jumpFlag == true)
    {
      for(int row = 0; row < numOfRowsLED; row++)
      {
        for(int col = 0; col < numOfColsLED; col++)
        {
          byte ledCode = JumpFunctions.GetPixel(row, col);
          
          if(ledCode == PIXEL_ON)
            CoreFunctions.SetBrightness(globalVal, row, col);
          else if(ledCode == PIXEL_OFF)
            CoreFunctions.SetBrightness(0, row, col);
          else
          {
            //ledCode is PIXEL_IGNORE
          }
        }
      }
      SetBrightnesses();
      JumpFunctions.NextFrame();
      jumpFlag = JumpFunctions.GetAnimState();
    }
  }
  delay(100);
}

/*
 * FUNCTION THAT ASSIGNS THE CONVERTED BRIGHTNESS TO THE LED MATRIX
 */
void SetBrightnesses()
{
  int ledPosition = 0;
  bool isOddRow = false;
  
  //iterate through each LED and illuminate it based off brightness
  for(int row = 0; row < numOfRowsLED; row++)
  {
    if(row % 2 == 1)
      isOddRow = true;
    else
      isOddRow = false;

    
    for(int col = 0; col < numOfColsLED; col++)
    {
      byte brightness = CoreFunctions.GetBrightness(row, col);
      if(isOddRow == true)
      {
        ledPosition = ((((row*26)+25)-col));
        matrixLEDs[ledPosition] = CHSV(globalHue, globalSat, brightness);
      }
      else
      {
        ledPosition = ((row*26)+col);
        matrixLEDs[ledPosition] = CHSV(globalHue, globalSat, brightness);
      }
    }
  }
  FastLED.show();
}
