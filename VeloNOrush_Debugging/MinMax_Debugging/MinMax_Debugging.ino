#include <EEPROM.h>

//pins used for buttons
const byte btnAdvancePin = 2;
const byte btnStorePin = 3;
const byte btnReadEEPROMPin = 4;
const byte btnReadPin = 5;
const byte btnUpdateMaxPin = 11;
const byte btnUpdateMinPin = 12;

//Select the pins that will count for the A,B,C, and D
const byte muxPinA = 7;
const byte muxPinB = 8;
const byte muxPinC = 9;
const byte muxPinD = 10;

//constants to describe sensor matrix
const byte numOfSensors = 96;
const byte numOfRows = 12;
const byte numOfCols = 8;

//loop variables
const int loopDelay = 3000;

//constants throughout the code
const byte adcDelay = 100;

//variables to hold sensor selection data
byte analogPin = 14;
byte muxPin = 0;

byte minVals[numOfRows][numOfCols]
{
  {18,20,21,27,30,27,37,32},
  {42,64,65,60,62,71,72,65},
  {66,245,70,75,70,94,112,99},
  {40,56,83,103,120,130,153,132},
  {45,64,76,60,97,94,95,102},
  {43,231,84,125,108,115,113,131},
  {48,83,88,109,142,140,156,130},
  {83,86,100,123,122,130,160,150},
  {53,82,74,63,76,78,96,87},
  {73,80,85,70,70,76,89,79},
  {57,61,75,110,109,106,110,113},
  {52,51,60,49,50,52,50,60}
};
byte maxVals[numOfRows][numOfCols]
{
  {110,170,160,145,185,187,197,196},
  {123,197,184,184,177,175,209,187},
  {177,245,180,131,144,184,200,180},
  {174,219,162,188,199,188,217,221},
  {149,207,190,208,197,195,199,189},
  {91,231,190,190,219,205,196,185},
  {156,202,161,200,210,190,242,161},
  {204,206,159,200,195,176,232,232},
  {203,199,135,170,156,167,211,211},
  {143,196,186,178,170,145,160,172},
  {183,218,149,193,200,202,230,234},
  {212,195,204,198,200,230,240,237}
};

/*ALL FASTLED DEFINITIONS*/
#include <FastLED.h>
#define NUM_LEDS 78
#define LED_PIN 6
#define MAX_BRIGHTNESS 200
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

//Create FastLED container for all the lights
CRGB matrixLEDs[NUM_LEDS];

//variables for FastLED lighting color
byte globalHue = 145;
byte globalSat = 255;
byte globalVal = 255;

//specifications of power supply used
const byte psuVoltage = 5;            //supply value in volts
const uint32_t psuAmperage = 2000;    //supply value in milliamps

/*ALL USER FUNCTIONS*/

//Function to blink the onboard LED of the UNO over 1 second
void BlinkLEDs(int blinkCount)
{
  int delayAmount = (1000 / (blinkCount * 2));

  for (int i = 0; i < blinkCount; i++)
  {
    digitalWrite(13, HIGH);
    delay(delayAmount);
    digitalWrite(13, LOW);
    delay(delayAmount);
  }
}

void TestLEDs()
{
  //iterate through all LEDs in the matrix

  for (int i = 0; i < NUM_LEDS; i++)
  {
    matrixLEDs[i] = CHSV(globalHue, globalSat, globalVal / 2);    //set to global hue, staturation and value
    FastLED.show();                                             //update each loop
    delay(5);                                                  //10ms delay between each light
  }

  //wait 1 second and blank the LEDs
  delay(1000);
  FastLED.clear();
  FastLED.show();
}

void SetMUX()
{
  bool pinAval, pinBval, pinCval, pinDval = 0;    //bit storage of each input pin
  byte readPin = muxPin;                          //storage of pin number to work with

  //convert the decimal pin number to binary
  pinAval = readPin % 2;
  readPin /= 2;
  pinBval = readPin % 2;
  readPin /= 2;
  pinCval = readPin % 2;
  readPin /= 2;
  pinDval = readPin % 2;

  //set the pins of all MUXes
  digitalWrite(muxPinA, pinAval);
  digitalWrite(muxPinB, pinBval);
  digitalWrite(muxPinC, pinCval);
  digitalWrite(muxPinD, pinDval);
}

void RowCol2Pins(byte row, byte col)
{
  //use the equation below to convert the row and column to mux pin and analog pin
  //Each analog pin controls 16 mux pins
  analogPin = 14 + (row / 4) + (col / 4 * 3);
  muxPin = (row % 4) * 4 + (col % 4);
}

void GetMins()
{
  byte minVal = 0;
  byte storedMin = 255;
  //fill array with max values
  for (int row = 0; row < numOfRows; row++)
  {
    for (int col = 0; col < numOfCols; col++)
    {
      minVals[row][col] = 255;
    }
  }

  FastLED.clear();
  FastLED.show();
  for (int row = 0; row < numOfRows; row++)
  {
    for (int col = 0; col < numOfCols; col++)
    {
      RowCol2Pins(row, col);
      SetMUX();
      while (digitalRead(btnAdvancePin) == HIGH)
      {
        minVal = (byte) (analogRead(analogPin) / 4);
        Serial.println(minVal);
        storedMin = minVals[row][col];
        if (minVal < storedMin)
        {
          minVals[row][col] = minVal;
        }
        delay(loopDelay);
      }
      Serial.print("Value stored: ");
      Serial.println(minVals[row][col]);
      BlinkLEDs(3);
      if (row == 11)
      {
        ShowSelectedSensor(10, (col + 18));
      }
      else
      {
        ShowSelectedSensor(row, col);
      }
      delay(1000);
    }
  }

  //print out all the values
  for (int row = 0; row < numOfRows; row++)
  {
    for (int col = 0; col < numOfCols; col++)
    {
      Serial.print(minVals[row][col]);
      Serial.print("  ");
    }
    Serial.println(" ");
  }
  BlinkLEDs(3);
}

void GetMaxes()
{
  const int thresholdOffset = 0;
  byte sampleDelay = 100;
  byte maxVal = 0;

  Serial.println("Max Calibration starting soon, \nstep on the sensors then step off!");
  BlinkLEDs(10);
  Serial.println("Calibrating Maxes");
  //for loop for iterating through each of the sensors in the array
  for (int rowCounter = 0; rowCounter < numOfRows; rowCounter++)
  {
    for (int colCounter = 0; colCounter < numOfCols; colCounter++)
    {
      volatile unsigned long maxSum = 0;
      for (int samples = 0; samples < 5; samples++)
      {
        RowCol2Pins(rowCounter, colCounter);
        SetMUX();
        maxVal = (byte) (analogRead(analogPin) / 4);
        maxSum += maxVal;
        delay(sampleDelay);
      }

      //find the average of the sensor value polled
      byte newMaxVal = ((maxSum / 5) - thresholdOffset);

      //call the calibration object to store the data in EEPROM
      Serial.print(newMaxVal);
      Serial.print("  ");
      maxVals[rowCounter][colCounter] = newMaxVal;
      //BlinkLEDs(1);
    }
    Serial.println("");
  }
  Serial.println("End max calibration");
}

void StoreMins()
{
  int address = 0;
  for (int row = 0; row < numOfRows; row++)
  {
    for (int col = 0; col < numOfCols; col++)
    {
      //int address = (row * 8) + col + (numOfSensors * 3);
      byte valToStore = minVals[row][col];
      EEPROM.update(address, valToStore);
      address++;
    }
  }
}

void StoreMaxes()
{
  int address = numOfSensors;
  for (int row = 0; row < numOfRows; row++)
  {
    for (int col = 0; col < numOfCols; col++)
    {
      byte valToStore = maxVals[row][col];
      EEPROM.update(address, valToStore);
      address++;
    }
  }
}

void ReadMinsMaxes(bool fromEEPROM)
{
  if (fromEEPROM == true)
  {
    //Print the maximums from EEPROM first
    Serial.println("Printing the Max Values from EEPROM...");
    for (int row = 0; row < numOfRows; row++)
    {
      for (int col = 0; col < numOfCols; col++)
      {
        int address = (row * 8) + col;
        byte storedVal = EEPROM.read(address);
        Serial.print(storedVal);
        Serial.print("  ");
      }
      Serial.println("");
    }

    //Print the minimums from EEPROM
    Serial.println("Printing the Min Values from EEPROM...");
    for (int row = 0; row < numOfRows; row++)
    {
      for (int col = 0; col < numOfCols; col++)
      {
        int address = (row * 8) + col + numOfSensors;
        byte storedVal = EEPROM.read(address);
        Serial.print(storedVal);
        Serial.print("  ");
      }
      Serial.println("");
    }
  }
  else
  {
    Serial.println("Printing the Min Values from dynamic memory...");
    for (int row = 0; row < numOfRows; row++)
    {
      for (int col = 0; col < numOfCols; col++)
      {
        byte storedVal = minVals[row][col];
        Serial.print(storedVal);
        Serial.print("  ");
      }
      Serial.println("");
    }
    Serial.println("Printing the Max Values from dynamic memory...");
    for (int row = 0; row < numOfRows; row++)
    {
      for (int col = 0; col < numOfCols; col++)
      {
        byte storedVal = maxVals[row][col];
        Serial.print(storedVal);
        Serial.print("  ");
      }
      Serial.println("");
    }
  }
}

void ShowSelectedSensor(byte row, byte col)
{
  //show which row is being calibrated by lighting up that row red
  //show how many sensors of that row have been calibrated by changing light to green
  int ledPosition = 0;
  bool isOddRow = false;

  //determine if on an odd row
  if (row % 2 == 1)
    isOddRow = true;
  else
    isOddRow = false;

  if (isOddRow == true)
  {
    ledPosition = ((((row * 26) + 25) - col));
    matrixLEDs[ledPosition] = CHSV(96, globalSat, globalVal);
  }
  else
  {
    ledPosition = ((row * 26) + col);
    matrixLEDs[ledPosition] = CHSV(96, globalSat, globalVal);
  }

  //if that was the last sensor of a row, set the next row to red
  if (col == (numOfCols - 1))
  {
    if (row != (numOfRows - 1))
    {
      for (int i = 0; i < 8; i++)
      {
        if (isOddRow == false)
        {
          ledPosition = (((((row + 1) * 26) + 25) - (col - i)));
          matrixLEDs[ledPosition] = CHSV(0, globalSat, globalVal);
        }
        else
        {
          ledPosition = (((row + 1) * 26) + (col - i));
          matrixLEDs[ledPosition] = CHSV(0, globalSat, globalVal);
        }
      }
    }
  }
  FastLED.show();
}

void UpdateMax()
{
  byte rowSelect = 0;
  byte colSelect = 0;
  byte newMaxVal = 0;
  byte prevMaxVal = 0;
  
  Serial.println("Enter the row to access: ");
  while (Serial.available() == 0) {
  }
  rowSelect = (byte) Serial.parseInt();
  Serial.println("Enter the column to access: ");
  while (Serial.available() == 0) {
  }
  colSelect = (byte) Serial.parseInt();
  
  RowCol2Pins(rowSelect, colSelect);
  SetMUX();
  newMaxVal = (byte) (analogRead(analogPin) / 4);
  prevMaxVal = maxVals[rowSelect][colSelect];
  maxVals[rowSelect][colSelect] = newMaxVal;

  Serial.print("The previous max value: ");
  Serial.println(prevMaxVal);
  Serial.print("The new max value: ");
  Serial.println(newMaxVal);

  delay(4000);
}

void UpdateMin()
{
  byte rowSelect = 0;
  byte colSelect = 0;
  byte newMinVal = 0;
  byte prevMinVal = 0;
  
  Serial.println("Enter the row to access: ");
  while (Serial.available() == 0) {
  }
  rowSelect = (byte) Serial.parseInt();
  Serial.println("Enter the column to access: ");
  while (Serial.available() == 0) {
  }
  colSelect = (byte) Serial.parseInt();
  
  RowCol2Pins(rowSelect, colSelect);
  SetMUX();
  newMinVal = (byte) (analogRead(analogPin) / 4);
  prevMinVal = maxVals[rowSelect][colSelect];
  minVals[rowSelect][colSelect] = newMinVal;

  Serial.print("The previous min value: ");
  Serial.println(prevMinVal);
  Serial.print("The new min value: ");
  Serial.println(newMinVal);

  delay(4000);
}

void setup()
{
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);
  pinMode(muxPinD, OUTPUT);
  pinMode(btnAdvancePin, INPUT_PULLUP);
  pinMode(btnStorePin, INPUT_PULLUP);
  pinMode(btnReadEEPROMPin, INPUT_PULLUP);
  pinMode(btnReadPin, INPUT_PULLUP);
  pinMode(btnUpdateMaxPin, INPUT_PULLUP);
  pinMode(btnUpdateMinPin, INPUT_PULLUP);

  Serial.begin(9600);

  //HANDLE ALL TASKS RELATED TO THE FASTLED LIBRARY
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(matrixLEDs, NUM_LEDS).setCorrection(TypicalLEDStrip);    //initialize LEDs
  FastLED.setBrightness(MAX_BRIGHTNESS);                                //set maximum brightness for leds, independent of color value
  FastLED.setMaxRefreshRate(120);                                        //set LED update rate limit to 60 frames per second
  set_max_power_in_volts_and_milliamps(psuVoltage, psuAmperage);        //limit the brightness of the LEDs to keep within power spec
  set_max_power_indicator_LED(13);                                      //notify the user if leds draw too much power
  FastLED.clear();                                                      //clear all LED data
  FastLED.show();                                                       //update LED matrix to blank it

  TestLEDs();

  //GetMaxes();
  //GetMins();
  BlinkLEDs(3);
}

void loop()
{
  //convert the current value and map it from a range of 0 to 100
  volatile byte tempMin = 0;
  volatile byte tempMax = 0;
  volatile byte currentVolt = 0;
  volatile byte currentMax = 0;
  volatile byte currentMin = 0;
  volatile byte currentVal = 0;
  static bool storeFlag = 0;
  static bool readEEPROMFlag = 0;
  static bool readFlag = 0;
  static bool updateMaxFlag = 0;
  static bool updateMinFlag = 0;

  Serial.println("---------------------------------------");
  for (int row = 0; row < numOfRows; row++)
  {
    for (int col = 0; col < numOfCols; col++)
    {
      RowCol2Pins(row, col);
      SetMUX();
      currentVolt = (byte) (analogRead(analogPin) / 4);
      /*currentMax = maxVals[row][col];
      currentMin = minVals[row][col];
      if (currentVolt > currentMax)
        currentVolt = currentMax;
      if (currentVolt < currentMin)
        currentVolt = currentMin;
      currentVal = map(currentVolt, currentMax, currentMin, 0, 100);*/
      Serial.print(currentVolt);
      Serial.print("  ");
    }
    Serial.println(" ");
  }
  Serial.println("-----------------------------------------");
  
  readEEPROMFlag = digitalRead(btnReadEEPROMPin);
  storeFlag = digitalRead(btnStorePin);
  readFlag = digitalRead(btnReadPin);
  updateMaxFlag = digitalRead(btnUpdateMaxPin);
  updateMinFlag = digitalRead(btnUpdateMinPin);
  
  if (readEEPROMFlag == LOW)
  {
    ReadMinsMaxes(true);
    delay(5000);
  }
  if (readFlag == LOW)
  {
    ReadMinsMaxes(false);
    delay(5000);
  }
  if (storeFlag == LOW)
  {
    StoreMins();
    StoreMaxes();
    BlinkLEDs(7);
  }
  if (updateMaxFlag == LOW)
  {
    UpdateMax();
  }
  if (updateMinFlag == LOW)
  {
    UpdateMin();
  }
  BlinkLEDs(3);
  delay(loopDelay);
}
