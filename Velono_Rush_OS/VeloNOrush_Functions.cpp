#include "VeloNOrush_Functions.h"

/**
 * ~~~~~~~~~~~~~~PUBLIC FUNCTIONS~~~~~~~~~~~~~~~~~~
 */

VeloNOrushCore::VeloNOrushCore(byte a, byte b, byte c, byte d)
{
  pinA = a;
  pinB = b;
  pinC = c;
  pinD = d;

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);

  //enable all analog pins as inputs
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  CalculateSampleDelay(CALIBRATE_TIME, CALIBRATE_SAMPLES);
}

void VeloNOrushCore::ReadSensor(byte row, byte col)
{
  //variables needed
  byte voltageVal = 0;
  RowCol2Pins(row, col);
  SetMUX();
  voltageVal = analogRead(analogPin) / 4;
  delayMicroseconds(100);
  
  byte tempMax = GetMax(row, col);
  byte tempMin = GetMin(row, col);
  //Serial.println(voltageVal);
  //Serial.println(tempMin);
  
  if(voltageVal > tempMax)
    voltageVal = tempMax;
  if(voltageVal < tempMin)
    voltageVal = tempMin;
  
  voltageReadings[row][col] = voltageVal;
}

void VeloNOrushCore::ReadSensors()
{
  //for loop for iterating through each of the sensors in the array
  for(byte rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(byte colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      ReadSensor(rowCounter, colCounter);
    }
  }
}

byte VeloNOrushCore::GetSensorVal(byte row, byte col)
{
  return voltageReadings[row][col];
}

void VeloNOrushCore::MapSensorsToBrightness(byte maxVal)
{
  //clear out the array of all the brightnesses initially
  for(byte ledRow = 0; ledRow < LED_ROWS; ledRow++)
  {
    for(byte ledCol = 0; ledCol < LED_COLS; ledCol++)
    {
      ledBrightness[ledRow][ledCol] = 0;
    }
  }

  //iterate through each row of the stored voltages
  for(byte row = 0; row < SENSOR_ROWS; row++)
  {
    //create a storage for the current column/led being targeted
    int currentColLED = 0;
    
    //iterate through each column of the stored voltages
    for(byte col = 0; col < SENSOR_COLS; col++)
    {
      byte currentSensorVal = GetSensorVal(row, col);
      byte currentMax = GetMax(row, col);
      byte currentMin = GetMin(row, col);
      
      //calculate the current "brightness" that a specific sensor is generating
      byte curBr = map(
                        currentSensorVal, 
                        currentMax, 
                        currentMin, 
                        0, maxVal
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
        else if(col == SENSOR_COLS-1) //CHECK FOR THE RIGHT EDGE OF THE MATRIX
        {
          ledBrightness[row][currentColLED-1] += curBr4;
          ledBrightness[row][currentColLED] += curBr4;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr2;
          
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
      else if(row == (SENSOR_ROWS-1)) //CHECK IF AT THE BOTTOM EDGE OF THE SENSOR MATRIX
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
        else if(col == SENSOR_COLS-1)
        {
          ledBrightness[row-1][currentColLED-1] += curBr4;
          ledBrightness[row-1][currentColLED] += curBr4;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr2;
          
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
        else if(col == SENSOR_COLS-1)
        {
          ledBrightness[row-1][currentColLED-1] += curBr4;
          ledBrightness[row-1][currentColLED] += curBr4;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr2;
          
          ledBrightness[row][currentColLED-1] += curBr4;
          ledBrightness[row][currentColLED] += curBr4;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr2;
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

void VeloNOrushCore::SetBrightness(byte val, byte row, byte col)
{
  ledBrightness[row][col] = val;
}

byte VeloNOrushCore::GetBrightness(byte row, byte col)
{
  byte brightness = ledBrightness[row][col];
  return brightness;
}

bool VeloNOrushCore::CalibrateMins()
{
  Serial.println("Calibrating Mins");
  const byte errorVal = 0;
  const byte presetMins[SENSOR_ROWS][SENSOR_COLS] = 
  {
//col: 0  1  2  3  4  5  6  7                    ROW:      
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
  
  for(byte rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(byte colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      byte tempVal = presetMins[rowCounter][colCounter];
      StoreMin(tempVal, rowCounter, colCounter);
      //Serial.print(presetMins[rowCounter][colCounter]);
      //Serial.print("  ");
    }
    //Serial.println("");
  }
  Serial.println("End Mins Calibration");
  return true;
}

bool VeloNOrushCore::CalibrateMaxes()
{
  const int thresholdOffset = 0;
  byte sampleDelay = GetCalibrationDelay();
  byte maxVals[SENSOR_ROWS][SENSOR_COLS]
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
  //Serial.println(sampleDelay);
  Serial.println("Calibrating Maxes");
  //for loop for iterating through each of the sensors in the array
  for(byte rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(byte colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      /*unsigned long maxSum = 0;
      for(int samples = 0; samples < CALIBRATE_SAMPLES; samples++)
      {
        ReadSensor(rowCounter, colCounter);
        volatile byte tempRead = GetSensorVal(rowCounter, colCounter);
        maxSum += tempRead;
        delay(sampleDelay);
      }

      //find the average of the sensor value polled
      //byte newMaxVal = ((maxSum / CALIBRATE_SAMPLES) - thresholdOffset);
      byte newMaxVal = maxSum / CALIBRATE_SAMPLES;
      newMaxVal = maxSum - thresholdOffset;

      //call the calibration object to store the data in EEPROM
      //Serial.print(newMaxVal);
      //Serial.print("  ");*/
      volatile byte newMaxVal = maxVals[rowCounter][colCounter];
      StoreMax(newMaxVal, rowCounter, colCounter);
    }
    //Serial.println("");
  }
  Serial.println("End max calibration");
  return true;
}

/*
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~PRIVATE FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

void VeloNOrushCore::SetMUX()
{
  bool pinAval, pinBval, pinCval, pinDval = 0;    //bit storage of each input pin
  int readPin = muxPin;                        //storage of pin number to work with

  //convert the decimal pin number to binary
  pinAval = readPin % 2;
  readPin/=2;
  pinBval = readPin % 2;
  readPin/=2;
  pinCval = readPin % 2;
  readPin/=2;
  pinDval = readPin % 2;

  //set the pins of all MUXes
  digitalWrite(pinA, pinAval);
  digitalWrite(pinB, pinBval);
  digitalWrite(pinC, pinCval);
  digitalWrite(pinD, pinDval);
}

void VeloNOrushCore::RowCol2Pins(byte row, byte col)
{
  //use the equation below to convert the row and column to mux pin and analog pin
  //Each analog pin controls 16 mux pins
  analogPin = 14 + (row / 4) + (col / 4 * 3);
  muxPin = (row % 4) * 4 + (col % 4);

}
