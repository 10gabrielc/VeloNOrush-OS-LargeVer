#include "calibration.h"
#include <EEPROM.h>

CalibrationCore::CalibrationCore()
{
}

void CalibrationCore::CalculateSampleDelay(int sampleTime, int numOfSamples)
{
  //calculate the exact loop delay desired
  samplingDelay = (((sampleTime * 1000) / 96) / numOfSamples);
}

void CalibrationCore::StoreMax(byte maxVal, int row, int col)
{
  int address = (row*8) + col;
  EEPROM.update(address, maxVal);
}

void CalibrationCore::StoreMin(byte minVal, int row, int col)
{
  int address = (row*8) + col + 96;
  EEPROM.update(address, minVal);
}

int CalibrationCore::GetCalibrationDelay()
{
  return samplingDelay;
}

byte CalibrationCore::GetMax(int row, int col)
{
  int address = (row*8) + col;
  byte maxValue = EEPROM.read(address);
  return maxValue;
}

byte CalibrationCore::GetMin(int row, int col)
{
  int address = (row*8) + col + 96;
  byte minValue = EEPROM.read(address);
  return minValue;
}
/*
void CalibrationCore::LoadCalibrationVals()
{
  //read from the first 96 spots in the Arduino EEPROM for the
  //sensor max values
  int storageCounter = 0;
  
  for(int row = 0; row < SENSOR_ROWS; row++)
  {
    for(int col = 0; col < SENSOR_COLS; col++)
    {
      maxOffsets[row][col] = EEPROM.read(storageCounter);
      minOffsets[row][col] = EEPROM.read(storageCounter+96);
      storageCounter++;
    }
  }
}*/
