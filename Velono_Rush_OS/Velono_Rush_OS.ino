//Select the pins that will count for the A,B,C, and D
const byte muxPinA = 7;    
const byte muxPinB = 8;
const byte muxPinC = 9;
const byte muxPinD = 10;

//Pins for the different MUX chip outputs
const byte voltPin1 = A0;
const byte voltPin2 = A1;
const byte voltPin3 = A2;
const byte voltPin4 = A3;
const byte voltPin5 = A4;
const byte voltPin6 = A5;

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

//storage for all voltage readings
byte voltageReadings[numOfRows][numOfCols];

//storage for sensor offsets
byte sensorMaxOffsets[numOfRows][numOfCols];

//storage for LED brightnesses
//byte ledBrightness[numOfRows][numOfCols];
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

/*
 * FUNCTION FOR SETTING THE PINS OF THE MUX TO READ A
 * SPECIFIC INPUT PIN.
 * -ACCEPTS AN INTEGER AS THE PIN TO READ
 * -SETS THE PINS OF ALL THE MUXES AT ONCE
 */
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

struct location GetNextRowCol(int storageCounter, int rowVal, int colVal)
{
  struct location loc;
  //Handle determining which location to store the sensor's data. Because there are 6 muxes,
  //with each MUX reading data in squares of 4x4, the data needs to be reorganized and
  //cannot simply be stored in sequential order. Otherwise reading the data would be confusin
  //for later algorithms
  if(storageCounter < (numOfSensors/2))
  {
    //reset the column counter after 4 columns, and increment row counter
    if(colVal >= 3)
    {
      colVal = 0;
      rowVal++;
    }
    else
      colVal++;
      
  }
  //at the middle point, transition to polling the right side of the sensor array
  else if(storageCounter == (numOfSensors/2))
  {
    colVal = 4;
    rowVal = 0;
  }
  else  //(storageCounter > 48)
  {
    if(colVal >= 7)
    {
      colVal = 4;
      rowVal++;
    }
    else
      colVal++;
  }

  loc.row = rowVal;
  loc.col = colVal;

  return loc;
}

/*
 * FUNCTION TO CALIBRATE THE VELOSTAT-BASED SENSORS
 * SAMPLES EACH SENSOR OVER A PERIOD OF TIME TO CREATE AN AVERAGE
 * OF VOLTAGE WHEN THERE IS NO PRESS OCCURING
 * SAMPLE RATE AND QUALITY CAN BE CHANGED IN-FUNCTION
 */
void CalibrateSensorMaxes()
{
  /*
   * TRUE CALIBRATION OF SENSORS: GET THE MAXIMUM VOLTAGE THE SENSOR READS
   * -GET/READ THE MINIMUM VOLTAGE THE SENSORS READ
   * -MAP THIS VALUE FROM WHATEVER RANGE IT IS, TO 255-0
   * THIS LETS IT FIT INTO JUST A BYTE OF DATA INSTEAD OF AN INT!
   */
  
  //variables needed for the function
  int rowCounter, colCounter = 0;
  int storageCounter = 0;
  int voltageSums = 0;
  int sensorCounter = 0;
  int sensorCounterLast = 0;
  struct location loc;
  loc.row = 0;
  loc.col = 0;
  
  //constants for loop timing
  const int thresholdOffset = 40;         //A shift to the threshold to filter out high "blips"
  const int totalCalibrateTime = 5;       //Time in seconds to wait for entire calibration process
  const int samplesPerCalibrate = 10;     //Number of reads to perform per sensor

  //calculate the exact loop delay desired
  int sampleDelay = (((totalCalibrateTime * 1000) / 96) / samplesPerCalibrate);

  //interating through each sensor
  for(int voltPin = 14; voltPin < 20; voltPin++)
  {
    for(int muxIn = 0; muxIn < 16; muxIn++)
    {
      //set mux pin to desired input
      SetMuxSelection(muxIn);

      //clear any previous values
      voltageSums = 0;

      //Perform the sampling of a sensor, adding results to a sum
      for(int samples = 0; samples < samplesPerCalibrate; samples++)
      {
        int tempRead = analogRead(voltPin); //read voltage
        voltageSums+=tempRead;              //store voltage
        delayMicroseconds(100);             //ADC cooldown delay
        delay(sampleDelay);                 //delay to match calibration timing
      }

      //take the average of the readings, and store it in the desired location
      sensorMaxOffsets[loc.row][loc.col] = (((voltageSums / samplesPerCalibrate) - thresholdOffset) / 4);

      storageCounter++;

      loc = GetNextRowCol(storageCounter, loc.row, loc.col);

      int ledsToLight = map(storageCounter, 0, numOfSensors-1, 0, NUM_LEDS-1);
      FastLED.clear();
      for(int led = 0; led < ledsToLight+1; led++)
      {
        matrixLEDs[led] = CHSV(globalHue/8, globalSat, globalVal);
      }
      FastLED.show();
    }
  }

  //blink the LED matrix to signify the sensors have been calibrated
  fill_solid(matrixLEDs, NUM_LEDS, CHSV(96, globalSat, globalVal));
  FastLED.show();
  delay(250);
  FastLED.clear();
  FastLED.show();
  delay(250);
  fill_solid(matrixLEDs, NUM_LEDS, CHSV(96, globalSat, globalVal));
  FastLED.show();
  delay(250);
  FastLED.clear();
  FastLED.show();

  delay(500);
}

void setup() 
{
  //Initialize all pins
  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);
  pinMode(muxPinD, OUTPUT);
  pinMode(voltPin1, INPUT);
  pinMode(voltPin2, INPUT);
  pinMode(voltPin3, INPUT);
  pinMode(voltPin4, INPUT);
  pinMode(voltPin5, INPUT);
  pinMode(voltPin6, INPUT);
  pinMode(13, OUTPUT);        //used to notify if power goes over limit

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
  
  //calibrate the sensors once before loop
  CalibrateSensorMaxes();
  
  delay(1000);
}

void loop() 
{
  //First, we read the status of all the sensors
  ReadSensors();

  //Second, we convert sensor data to brightnesses
  ConvertVoltageToBrightness();

  //Lastly, we set the LED brightnesses
  SetBrightnesses();

  delay(loopDelay);
}

int GetSensorVoltage(int readPin, int row, int col)
{
  //variables needed
  int voltageVal = 0;

  const byte sensorMinOffsets[numOfRows][numOfCols] = {{10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10}};

  
  //read voltage from the specific analog port
  voltageVal = (analogRead(readPin) / 4);
  
  //Give the ADC time to reset
  delayMicroseconds(adcDelay);
  
  //Adjust the read voltage if it is outside of the "voltage boundaries"
  if(voltageVal > sensorMaxOffsets[row][col])
    voltageVal = sensorMaxOffsets[row][col];
  else if(voltageVal < sensorMinOffsets[row][col])
    voltageVal = sensorMinOffsets[row][col];

  //return the calibrated sensor reading
  return voltageVal;
}

/*
 * FUNCTION FOR READING THE VOLTAGES FROM EACH OF THE SENSORS
 * UTILIZES 16 TO 1 MULTIPLEXERS TO CONVERT 96 SENSORS TO 6
 * ANALOG READ PINS
 */
void ReadSensors()
{
  //variables needed for the function
  int storageCounter = 0;                           //counter for number of voltages stored so far
  struct location loc;                       //struct variable for storing both row and column
  loc.row = 0;
  loc.col = 0;

  //for loop for iterating through each MUX or each analog pin
  //starts from A0 (14) to A5 (19)
  for(int analogPin = 14; analogPin < 20; analogPin++)
  {
    //for loop for interation through each pin of a MUX
    for(int muxPin = 0; muxPin < 16; muxPin++)
    {
      //set muxes to current pin choice
      SetMuxSelection(muxPin);
      
      //get and store the sensor data
      voltageReadings[loc.row][loc.col] = GetSensorVoltage(analogPin, loc.row, loc.col);

      //increment storage counter to determine row/col
      storageCounter++;

      //recalculate the current position on the board
      loc = GetNextRowCol(storageCounter, loc.row, loc.col);
    }
  }
}

/*
 * FUNCTION THAT TAKES THE STORED VOLTAGES AND CONVERTS THEM INTO
 * A BRIGHTNESS BETWEEN 0 AND THE MAX BRIGHTNESS
 */
void ConvertVoltageToBrightness()
{
  const byte sensorMinOffsets[numOfRows][numOfCols] = {{10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10},
                                                    {10,10,10,10,10,10,10,10}};

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

      //calculate the "brightness" that a specific sensor is generating
      byte curBr = map(
                                        voltageReadings[row][col], 
                                        sensorMaxOffsets[row][col], 
                                        sensorMinOffsets[row][col], 
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
  for(int row = 0; row < numOfRowsLED; row++)
  {
    if(row % 2 == 1)
      isOddRow = true;
    else
      isOddRow = false;

    
    for(int col = 0; col < numOfColsLED; col++)
    {
      if(isOddRow == true)
      {
        //need to reverse the lighting order, and offset by 2
        //each row has 26 LEDs, of which 3 are lit per sensor

        int ledPosition = ((((row*26)+25)-col));
        matrixLEDs[ledPosition] = CHSV(globalHue, globalSat, ledBrightness[row][col]);
      }
      else
      {
        int ledPosition = ((row*26)+col);
        matrixLEDs[ledPosition] = CHSV(globalHue, globalSat, ledBrightness[row][col]);
      }
    }
  }
  FastLED.show();
}

void CalibrateSensorMins()
{
  
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
}*/
