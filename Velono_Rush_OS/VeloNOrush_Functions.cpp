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

void VeloNOrushCore::ReadSensor(int row, int col)
{
  //variables needed
  int voltageVal = 0;
  RowCol2Pins(row, col);
  SetMUX();
  voltageVal = analogRead(analogPin) / 4;
  delayMicroseconds(100);

  byte tempMax = GetMax(row, col);
  byte tempMin = GetMin(row, col);
  
  if(voltageVal > tempMax)
      voltageVal = tempMax;
  if(voltageVal < tempMin)
    voltageVal = tempMin;
  
  voltageReadings[row][col] = voltageVal;
}

void VeloNOrushCore::ReadSensors()
{
  //for loop for iterating through each of the sensors in the array
  for(int rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(int colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      ReadSensor(rowCounter, colCounter);
    }
  }
}

byte VeloNOrushCore::GetSensorVal(int row, int col)
{
  return voltageReadings[row][col];
}

bool VeloNOrushCore::CalibrateMins()
{
  const byte tempMinVal = 10;
  
  for(int rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(int colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      StoreMin(tempMinVal, rowCounter, colCounter);
    }
  }
  return true;
}

bool VeloNOrushCore::CalibrateMaxes()
{
  const int thresholdOffset = 10;
  int sampleDelay = GetCalibrationDelay();

  //for loop for iterating through each of the sensors in the array
  for(int rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(int colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      unsigned long maxSum = 0;
      for(int samples = 0; samples < CALIBRATE_SAMPLES; samples++)
      {
        ReadSensor(rowCounter, colCounter);
        int tempRead = GetSensorVal(rowCounter, colCounter);
        maxSum += tempRead;
        delay(sampleDelay);
      }

      //find the average of the sensor value polled
      byte newMaxVal = ((maxSum / CALIBRATE_SAMPLES) - thresholdOffset);

      //call the calibration object to store the data in EEPROM
      StoreMax(newMaxVal, rowCounter, colCounter);
    }
  }

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

void VeloNOrushCore::RowCol2Pins(int row, int col)
{
  //use the equation below to convert the row and column to mux pin and analog pin
  //Each analog pin controls 16 mux pins
  analogPin = 14 + (row / 4) + (col / 4 * 3);
  muxPin = (row % 4) * 4 + (col % 4);

}

/*void VeloNOrushCore::GetNextRowCol()
{
  //Handle determining which location to store the sensor's data. Because there are 6 muxes,
  //with each MUX reading data in squares of 4x4, the data needs to be reorganized and
  //cannot simply be stored in sequential order. Otherwise reading the data would be confusin
  //for later algorithms
  if(storeCount < (96/2))
  {
    //reset the column counter after 4 columns, and increment row counter
    if(col >= 3)
    {
      col = 0;
      row++;
    }
    else
      col++;
      
  }
  //at the middle point, transition to polling the right side of the sensor array
  else if(storeCount == (96/2))
  {
    col = 4;
    row = 0;
  }
  else  //(storageCounter > 48)
  {
    if(col >= 7)
    {
      col = 4;
      row++;
    }
    else
      col++;
  }
}*/
