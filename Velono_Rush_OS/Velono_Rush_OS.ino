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

//Include required files for system functionality
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
const byte currentMode = 0;           //0 is position/weight tracking, 1 is jump

//loop variables
const int loopDelay = 8;              //125hz refresh rate

/*ALL FASTLED DEFINITIONS*/
#include <FastLED.h>
#define NUM_LEDS 286
#define LED_PIN 6
#define MAX_BRIGHTNESS 255
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

//Create FastLED container for the LED matrix
CRGB matrixLEDs[NUM_LEDS];

//variables for FastLED lighting color
byte globalHue = 170;
byte globalSat = 255;
byte globalVal = 255;

//Instantiate the main core for performing system functions
VeloNOrushCore CoreFunctions(muxPinA,muxPinB,muxPinC,muxPinD);

//instantiate instance of "Jump Mode" core
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
    matrixLEDs[i] = CHSV(globalHue, globalSat, globalVal/2);  //set to global hue, staturation and value
    FastLED.show();                                           //update each loop
    delay(5);                                                 //5ms delay between each light
  }

  //wait 1 second and blank the LED matrix
  delay(1000);
  FastLED.clear();
  FastLED.show();
}

/*
    FUNCTION TO CHECK IF THE USER WISHES TO RECALIBRATE THE SYSTEM UPON STARTUP.
    THIS CAN BE USED TO CHANGE THE MINIMUM AND MAXIMUM BOUNDS OF EACH SENSOR,
    IF LED LIGHTING SEEMS INACCURATE.
*/
void CheckForCalibration()
{
  int recalibrateTries = 0;           //storage for number of tries to recalibrate
  bool btnDetected = false;           //flag for if the recalibrate button is pressed
  bool minsFlag, maxsFlag = false;    //flags for if the min and max calibrations occured successfully
  const int maxTries = 3000;

  //continuously check for a recalibrate button press, or move on after a number of tries
  while((recalibrateTries < maxTries) && (btnDetected == false))
  {
    //check if the recalibrate button has been pressed. Button is in pull-up mode
    if(digitalRead(recalibratePin) == LOW)
    {
        //indicate calibration is happening by setting LEDs to red
        fill_solid(matrixLEDs, NUM_LEDS, CHSV(0, 255, MAX_BRIGHTNESS/2));
        FastLED.show();

        //calibrate maxes and minimums, and store their completion flags
        maxsFlag = CoreFunctions.CalibrateMaxes();
        minsFlag = CoreFunctions.CalibrateMins();

        //check if calibrations were successful, then wait 1 second to show results
        if(minsFlag && maxsFlag)
          fill_solid(matrixLEDs, NUM_LEDS, CRGB::Green);    //Success: set LEDs to green
        else
          fill_solid(matrixLEDs, NUM_LEDS, CRGB::Yellow);   //Fail: set LEDs to yellow
        FastLED.show();
        delay(1000);

        //clear LED matrix
        FastLED.clear();
        FastLED.show();

        //set detection flag to break the loop
        btnDetected = true;
    }
    recalibrateTries+=1;    //increment loop counter
    delay(1);               //wait 1 millisecond
  }
}

/*
    FUNCTION THAT CONTAINS MAIN PROCESS FOR DETECTING JUMPS WHILE IN THE
    JUMP MODE.
*/
void JumpHandling()
{
  bool jumpFlag = false;      //flag to indicate if a jump is detected
  byte rowCounter = 0;        //storage for current row
  byte colCounter = 0;        //storage for current column
  byte tempSensorVal = 0;     //storage for sensor value at current row/col
  byte tempMinVal = 0;        //storage for minimum value at current row/col

  //loop the process while no jump has been detected and the
  //final sensor has not been reached
  while(jumpFlag == false && (rowCounter < numOfRows))
  {
    //tempSensorVal = CoreFunctions.GetSensorVal(rowCounter, colCounter);
    //tempMinVal = CoreFunctions.GetMin(rowCounter, colCounter);
    //jumpFlag = JumpFunctions.JumpCheck(tempSensorVal, tempMinVal, rowCounter, colCounter);

    /*TEMPORARY FORCING OF DETECTION FOR TESTING*/
    byte randCol = (uint8_t) random(numOfCols);
    byte randRow = (uint8_t) random(numOfRows);

    //based off current sensor values, check if a jump has occured at a specific row and column
    jumpFlag = JumpFunctions.JumpCheck(10, 10, randRow, randCol);
    //Serial.print("Jumpflag: ");
    //Serial.println(jumpFlag);

    //increment row and column counters based on their status
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

  //HANDLE WHEN A JUMP IS DETECTED!
  if(jumpFlag == true)
  {
    //continue the animation until it is finished
    while(jumpFlag == true)
    {
      //iterate through each of the LEDs
      for(int row = 0; row < numOfRowsLED; row++)
      {
        for(int col = 0; col < numOfColsLED; col++)
        {
          //check if the current LED should be changed
          byte ledCode = JumpFunctions.GetPixel(row, col);
          
          //determine what to do with the current LED
          if(ledCode == PIXEL_ON)
            CoreFunctions.SetBrightness(globalVal, row, col);   //turn on the LED
          else if(ledCode == PIXEL_OFF)
            CoreFunctions.SetBrightness(0, row, col);           //turn off the LED
          else    //ledCode is PIXEL_IGNORE
          {}                                                    //preserve the state of the LED
        }
      }

      //update the LED matrix based off the previous calculations
      SetBrightnesses();

      //increment the animation frame counter in the jump object
      JumpFunctions.NextFrame();

      //check if the animation has finished
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
    //determine if the current row is odd or even: required due to LED's
    // sequential wiring, snaking back and forth through the matrix
    if(row % 2 == 1)
      isOddRow = true;
    else
      isOddRow = false;

    for(int col = 0; col < numOfColsLED; col++)
    {
      //retrieve brightness of a specific LED
      byte brightness = CoreFunctions.GetBrightness(row, col);

      //calculate which LED to select based off current row and column
      if(isOddRow == true)
      {
        //ledPosition = ((((row*26)+25)-col));
        ledPosition = row * 26;
        ledPosition = ledPosition + 25;
        ledPosition = ledPosition - col;

        //Update the LED container with the new value for brightness
        matrixLEDs[ledPosition] = CHSV(globalHue, globalSat, brightness);
      }
      else
      {
        //ledPosition = ((row*26)+col);
        ledPosition = row * 26;
        ledPosition = ledPosition + col;

        //update the LED container with the new value for brightness
        matrixLEDs[ledPosition] = CHSV(globalHue, globalSat, brightness);
      }
    }
  }

  //update the LEDs with the new data
  FastLED.show();
}

/*
    ARDUINO REQUIRED SETUP FUNCTION: RUNS ONCE.
    -SETS PINS NEEDED
    -INITIALIZES FASTLED SYSTEM
    -INITIALIZES SERIAL MONITOR
    -TESTS LEDS
    -RECALIBRATES SYSTEM
*/
void setup() 
{
  //Initialize required pins
  pinMode(recalibratePin, INPUT_PULLUP);  //must be pressed to recalibrate maxes/mins during setup
  pinMode(13, OUTPUT);                    //used to notify if power goes over limit
  
  //HANDLE ALL TASKS RELATED TO THE FASTLED LIBRARY
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(matrixLEDs, NUM_LEDS).setCorrection(TypicalLEDStrip);    //initialize LEDs
  FastLED.setBrightness(MAX_BRIGHTNESS);                                //set maximum brightness for leds, independent of color value
  FastLED.setMaxRefreshRate(120);                                       //set LED update rate limit to 120 frames per second
  set_max_power_in_volts_and_milliamps(psuVoltage, psuAmperage);        //limit the brightness of the LEDs to keep within power spec
  set_max_power_indicator_LED(13);                                      //notify the user if leds draw too much power
  FastLED.clear();                                                      //clear all LED data
  FastLED.show();                                                       //update LED matrix to blank it

  //initialize serial monitor
  Serial.begin(9600);
  
  //generate a seed for the random number generator
  randomSeed(analogRead(A0));

  //test all the leds in the chain
  TestLEDs();

  CheckForCalibration();
  
  delay(1000);
}

/*
    ARDUINO MAIN FUNCTION: LOOPS INFINITELY.
    -WHERE THE SYSTEM PROGRESSES THROUGH ITS REQUIRED TASKS
*/
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