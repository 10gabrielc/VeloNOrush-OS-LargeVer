#include "VeloNOrush_Functions.h"

//Select the pins that will count for the A,B,C, and D
const byte muxPinA = 7;    
const byte muxPinB = 8;
const byte muxPinC = 9;
const byte muxPinD = 10;

//Pins for button interactions
const byte recalibratePin = 4;

//specifications of power supply used
const byte psuVoltage = 5;
const uint32_t psuAmperage = 2000;

//constants to describe sensor matrix
const int numOfSensors = 96;
const int numOfRows = 12;
const int numOfCols = 8;
const int numOfRowsLED = 11;
const int numOfColsLED = 26;

//storage for LED brightnesses
byte ledBrightness[numOfRowsLED][numOfColsLED];

//loop variables
const int loopDelay = 8;

/*ALL FASTLED STUFF*/
#include <FastLED.h>
#define NUM_LEDS 286
#define LED_PIN 6
#define MAX_BRIGHTNESS 150
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

//change these to bytes
struct location{
  byte row;
  byte col;
};

VeloNOrushCore CoreFunctions(muxPinA,muxPinB,muxPinC,muxPinD);

//global memory usage:
// 3 arrays of 96 bytes: 288
// 286 LEDs, each LED with 24 bits: 858
// total: 1146

/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

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
    matrixLEDs[i] = CHSV(globalHue, globalSat, globalVal);      //set to global hue, staturation and value
    FastLED.show();                                             //update each loop
    delay(10);                                                  //10ms delay between each light
  }

  //wait 1 second and blank the LEDs
  delay(1000);
  FastLED.clear();
  FastLED.show();
}

void setup() 
{
  //Initialize all pins
  pinMode(recalibratePin, INPUT_PULLUP);  //must be pressed to recalibrate maxes
  pinMode(13, OUTPUT);                    //used to notify if power goes over limit

  //HANDLE ALL TASKS RELATED TO THE FASTLED LIBRARY
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(matrixLEDs, NUM_LEDS).setCorrection(TypicalLEDStrip);    //initialize LEDs
  FastLED.setBrightness(MAX_BRIGHTNESS);                                //set maximum brightness for leds, independent of color value
  FastLED.setMaxRefreshRate(60);                                        //set LED update rate limit to 60 frames per second
  set_max_power_in_volts_and_milliamps(psuVoltage, psuAmperage);        //limit the brightness of the LEDs to keep within power spec
  set_max_power_indicator_LED(13);                                      //notify the user if leds draw too much power
  FastLED.clear();                                                      //clear all LED data
  FastLED.show();                                                       //update LED matrix to blank it

  //initialize serial monitor
  Serial.begin(9600);
  
  //test all the leds in the chain
  TestLEDs();

  int recalibrateTries = 0;
  bool btnDetected = false;
  while((recalibrateTries < 3000) && (btnDetected == false))
  {
    recalibrateTries+=1;
    if(digitalRead(recalibratePin) == LOW)
    {
        fill_solid(matrixLEDs, NUM_LEDS, CHSV(0, 255, MAX_BRIGHTNESS/8));
        FastLED.show();
        CoreFunctions.CalibrateMaxes();
        CoreFunctions.CalibrateMins();
        fill_solid(matrixLEDs, NUM_LEDS, CHSV(120, 255, MAX_BRIGHTNESS/8));
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
  
  ConvertVoltageToBrightness();

  //Lastly, we set the LED brightnesses
  SetBrightnesses();

  delay(loopDelay);
}

/*
 * FUNCTION THAT TAKES THE STORED VOLTAGES AND CONVERTS THEM INTO
 * A BRIGHTNESS BETWEEN 0 AND THE MAX BRIGHTNESS
 */
void ConvertVoltageToBrightness()
{
  //clear out the array of all the brightnesses initially
  for(int ledRow = 0; ledRow < numOfRowsLED; ledRow++)
  {
    for(int ledCol = 0; ledCol < numOfColsLED; ledCol++)
    {
      ledBrightness[ledRow][ledCol] = 0;
    }
  }
  
  //iterate through each row of the stored voltages
  for(int row = 0; row < numOfRows; row++)
  {
    //create a storage for the current column/led being targeted
    int currentColLED = 0;
    
    //iterate through each column of the stored voltages
    for(int col = 0; col < numOfCols; col++)
    {

      byte currentSensorVal = CoreFunctions.GetSensorVal(row, col);
      byte currentMax = CoreFunctions.GetMax(row, col);
      byte currentMin = CoreFunctions.GetMin(row, col);
      
      //calculate the current "brightness" that a specific sensor is generating
      byte curBr = map(
                        currentSensorVal, 
                        currentMax, 
                        currentMin, 
                        0, globalVal
                       );

      //store pre-converted forms of the brightness to be assigned to LEDs
      byte curBr2 = curBr/2;
      byte curBr4 = curBr/4;
      
      //START WITH TOP EDGE OF THE SENSOR MATRIX
      if(row == 0)
      {
        //we can only assign values to LEDs below this row, as there are no
        //leds above these sensors

        //START WITH CHECKING FOR THE LEFT EDGE OF THE MATRIX
        if(col == 0)
        {
          ledBrightness[row][currentColLED] += curBr2;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr4;
          ledBrightness[row][currentColLED+3] += curBr4;
          
          currentColLED+=3;
        }
        else if(col == numOfCols-1) //CHECK FOR THE RIGHT EDGE OF THE MATRIX
        {
          ledBrightness[row][currentColLED] += curBr4;
          ledBrightness[row][currentColLED+1] += curBr4;
          ledBrightness[row][currentColLED+2] += curBr2;
          ledBrightness[row][currentColLED+3] += curBr2;
          
          currentColLED+=3;
        }
        else  //ALL OTHER COLUMNS INBETWEEN
        {
          if(col == 2 || col == 5)
          {
            ledBrightness[row][currentColLED-1] += curBr4;    
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr2;
            ledBrightness[row][currentColLED+3] += curBr4;
            ledBrightness[row][currentColLED+4] += curBr4;
            
            currentColLED+=4;
          }
          else
          {
            ledBrightness[row][currentColLED-1] += curBr4;
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr4;
            ledBrightness[row][currentColLED+3] += curBr4;
            
            currentColLED+=3;
          }
        }
        
      }
      else if(row == (numOfRows-1)) //CHECK IF AT THE BOTTOM EDGE OF THE SENSOR MATRIX
      {
        //need to also check if we are in the first or last column
        if(col == 0)
        {
          ledBrightness[row-1][currentColLED] += curBr2;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr4;
          ledBrightness[row-1][currentColLED+3] += curBr4;
          
          currentColLED+=3;
        }
        else if(col == numOfCols-1)
        {
          ledBrightness[row-1][currentColLED] += curBr4;
          ledBrightness[row-1][currentColLED+1] += curBr4;
          ledBrightness[row-1][currentColLED+2] += curBr2;
          ledBrightness[row-1][currentColLED+3] += curBr2;
          
          currentColLED+=3;
        }
        else
        {
          if(col == 2 || col == 5)
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;    
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr2;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            ledBrightness[row-1][currentColLED+4] += curBr4;
            
            currentColLED+=4;
          }
          else
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr4;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            
            currentColLED+=3;
          }
        }
      }
      else  //HANDLE ALL THE ROWS INBETWEEN
      {
        //need to also check if we are in the first or last column
        if(col == 0)
        {
          ledBrightness[row-1][currentColLED] += curBr2;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr4;
          ledBrightness[row-1][currentColLED+3] += curBr4;
          
          ledBrightness[row][currentColLED] += curBr2;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr4;
          ledBrightness[row][currentColLED+3] += curBr4;
          
          currentColLED+=3;
        }
        else if(col == numOfCols-1)
        {
          ledBrightness[row-1][currentColLED] += curBr4;
          ledBrightness[row-1][currentColLED+1] += curBr4;
          ledBrightness[row-1][currentColLED+2] += curBr2;
          ledBrightness[row-1][currentColLED+3] += curBr2;
          
          ledBrightness[row][currentColLED] += curBr4;
          ledBrightness[row][currentColLED+1] += curBr4;
          ledBrightness[row][currentColLED+2] += curBr2;
          ledBrightness[row][currentColLED+3] += curBr2;
          currentColLED+=3;
        }
        else
        {
          if(col == 2 || col == 5)
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;    
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr2;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            ledBrightness[row-1][currentColLED+4] += curBr4;
            
            ledBrightness[row][currentColLED-1] += curBr4;    
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr2;
            ledBrightness[row][currentColLED+3] += curBr4;
            ledBrightness[row][currentColLED+4] += curBr4;
            
            currentColLED+=4;
          }
          else
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr4;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            
            ledBrightness[row][currentColLED-1] += curBr4;
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr4;
            ledBrightness[row][currentColLED+3] += curBr4;
            
            currentColLED+=3;
          }
        }
      }
    }
  }
}

/*
 * FUNCTION THAT ASSIGNS THE CONVERTED BRIGHTNESS TO THE LED MATRIX
 */
void SetBrightnesses()
{
  int ledPosition = 0;
  bool isOddRow = false;
  
  //iterate through each LED and illuminate it based off brightness
  for(int row = 0; row < numOfRows; row++)
  {
    if(row % 2 == 1)
      isOddRow = true;
    else
      isOddRow = false;

    
    for(int col = 0; col < numOfCols; col++)
    {
      if(isOddRow == true)
      {
        //need to reverse the lighting order, and offset by 2
        //each row has 26 LEDs, of which 3 are lit per sensor

        int ledPosition = ((((row*26)+25)-(col*3)));
        
        for(int i = 0; i < 3; i++)
        {
          matrixLEDs[ledPosition-i] = CHSV(globalHue, globalSat, ledBrightness[row][col]);
        }
      }
      else
      {
        int ledPosition = ((row*26)+(col*3));

        for(int i = 0; i < 3; i++)
        {
          matrixLEDs[ledPosition+i] = CHSV(globalHue, globalSat, ledBrightness[row][col]);
        }
      }
    }
  }
  FastLED.show();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~TRASH BELOW HERE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

/*void CalibrateSensors()
{
    //storage for pin vals
    bool pinAval, pinBval, pinCval, pinDval = 0;
    int rowCounter, colCounter = 0;
    
    //storage for averaging math
    int sumOfV0, sumOfV1, sumOfV2, sumOfV3, sumOfV4, sumOfV5 = 0;

    //constants for loop timing
    const int thresholdOffset = 20;
    const int totalCalibrateTime = 3;
    const int samplesPerCalibrate = 10;

    //calculate the exact loop delay desired
    int sampleDelay = (((totalCalibrateTime * 1000) / 16) / samplesPerCalibrate);
    for(int j = 14; j < 20; j++)
    {
      for(int i = 0; i < 16; i++)
      {
          //set the selection pins of the MUX
          int currentPin = i;
          pinAval = currentPin % 2;
          currentPin/=2;
          pinBval = currentPin % 2;
          currentPin/=2;
          pinCval = currentPin % 2;
          currentPin/=2;
          pinDval = currentPin % 2;
  
          //set pins on the MUX
          digitalWrite(pinA, pinAval);
          digitalWrite(pinB, pinBval);
          digitalWrite(pinC, pinCval);
          digitalWrite(pinD, pinDval);
  
          //flush any previous values out of the sum variables
          sumOfV0 = 0;
          sumOfV1 = 0;
          sumOfV2 = 0;
          sumOfV3 = 0;
          sumOfV4 = 0;
          sumOfV5 = 0;

          
          //sample the selected sensors numerous times
          for(int j = 0; j < samplesPerCalibrate; j++)
          {
            sumOfV0+=analogRead(voltPin1);
            delayMicroseconds(100);
            sumOfV1+=analogRead(voltPin2);
            delayMicroseconds(100);
            sumOfV2+=analogRead(voltPin3);
            delayMicroseconds(100);
            sumOfV3+=analogRead(voltPin4);
            delayMicroseconds(100);
            sumOfV4+=analogRead(voltPin5);
            delayMicroseconds(100);
            sumOfV5+=analogRead(voltPin6);
            delayMicroseconds(100);
            delay(sampleDelay);
          }
  
          //store the result of averaging sensor readings
          sensorMaxOffsets[i]    = ((sumOfV0 / 10) + thresholdOffset);
          sensorMaxOffsets[i+16] = ((sumOfV1 / 10) + thresholdOffset);
          sensorMaxOffsets[i+32] = ((sumOfV2 / 10) + thresholdOffset);
          sensorMaxOffsets[i+48] = ((sumOfV3 / 10) + thresholdOffset);
          sensorMaxOffsets[i+64] = ((sumOfV4 / 10) + thresholdOffset);
          sensorMaxOffsets[i+80] = ((sumOfV5 / 10) + thresholdOffset);
          
          Serial.println(sensorMaxOffsets[i]);
          Serial.println(sensorMaxOffsets[i+16]);
          Serial.println(sensorMaxOffsets[i+32]);
          Serial.println(sensorMaxOffsets[i+48]);
          Serial.println(sensorMaxOffsets[i+64]);
          Serial.println(sensorMaxOffsets[i+80]);
          delay(1000);
      }
    }
    //blink the LED matrix to signify the sensors have been calibrated
    fill_solid(matrixLEDs, NUM_LEDS, CHSV(96, 255, MAX_BRIGHTNESS));
    FastLED.show();
    delay(250);
    FastLED.clear();
    FastLED.show();
    delay(250);
    fill_solid(matrixLEDs, NUM_LEDS, CHSV(96, 255, MAX_BRIGHTNESS));
    FastLED.show();
    delay(250);
    FastLED.clear();
    FastLED.show();

    delay(500);
}*/

/*void SetBrightness()
{
  int ledCount = 0;
  int ledPosition = 0;
  bool isOdd = false;
  const byte ledHue = 130;
  const byte ledSat = 255;

  for(int row = 0; row < 16; row++)
  {
    if(row % 2 == 1)
      isOdd = true;
    else
      isOdd = false;
    for(int col = 0; col < 16; col++)
    {
      if(isOdd == true)
      {
        ledPosition = (row*16) + col;
      }
      else
      {
        ledPosition = ((row*16) + 15) - col;
      }

      matrixLEDs[ledPosition] = CHSV(ledHue, ledSat, ledBrightness[ledCount]);
      ledCount++;
    }
  }

  FastLED.show();
}*/

/*void TestMatrix()
{
  const byte ledHue = 130;
  const byte ledSat = 255;
  int ledCount = 0;

  matrixLEDs[0] = CHSV(0, ledSat, 150);
  FastLED.show();
  delay(1000);
  FastLED.clear();
  FastLED.show();
  
  for(int row = 0; row < 16; row++)
  {
    if(row % 2 == 1)
      isOdd = true;
    else
      isOdd = false;
    for(int col = 0; col < 16; col++)
    {
      if(isOdd == true)
      {
        ledPosition = (row*16) + col;
      }
      else
      {
        ledPosition = ((row*16) + 15) - col;
      }

      matrixLEDs[ledPosition] = CHSV(ledHue, ledSat, ledBrightness[ledCount]);
      ledCount++;
    }
  }
  
  matrixLEDs[i] = CHSV(ledHue, ledSat, MAX_BRIGHTNESS);
  FastLED.show();
  delay(100);


  delay(1000):
  FastLED.clear();
  FastLED.show();
}


 * FUNCTION FOR SETTING THE PINS OF THE MUX TO READ A
 * SPECIFIC INPUT PIN.
 * -ACCEPTS AN INTEGER AS THE PIN TO READ
 * -SETS THE PINS OF ALL THE MUXES AT ONCE

void SetMuxSelection(int pinToRead)
{
  //variables needed
  bool pinAval, pinBval, pinCval, pinDval = 0;    //bit storage of each input pin
  int readPin = pinToRead;                        //storage of pin number to work with

  //convert the decimal pin number to binary
  pinAval = pinToRead % 2;
  pinToRead/=2;
  pinBval = pinToRead % 2;
  pinToRead/=2;
  pinCval = pinToRead % 2;
  pinToRead/=2;
  pinDval = pinToRead % 2;

  //set the pins of all MUXes
  digitalWrite(muxPinA, pinAval);
  digitalWrite(muxPinB, pinBval);
  digitalWrite(muxPinC, pinCval);
  digitalWrite(muxPinD, pinDval);
}

*/
