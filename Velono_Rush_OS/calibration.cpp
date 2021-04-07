#include "calibration.h"
#include <EEPROM.h>

CalibrationCore::CalibrationCore()
{
}

void CalibrationCore::CalculateSampleDelay(int sampleTime, int numOfSamples)
{
  //calculate the exact loop delay desired
  int tempMath = 0;
  tempMath = sampleTime * 1000;                       //convert seconds to milliseconds
  tempMath = tempMath / 96;                           //divide by number of sensors
  samplingDelay = tempMath / numOfSamples;            //divide total time needed by number of samples
  //samplingDelay = (uint8_t) (((sampleTime * 1000) / 96) / numOfSamples);
}

void CalibrationCore::StoreMax(byte maxVal, byte row, byte col)
{
  //int address = (row*8) + col;
  int address = row * 8;
  address = address + col;
  address = address + MAXS_BASE_ADDR;

  EEPROM.update(address, maxVal);
}

void CalibrationCore::StoreMin(byte minVal, byte row, byte col)
{
  //int address = (row*8) + col + 96;
  int address = row * 8;
  address = address + col;
  address = address + MINS_BASE_ADDR;

  EEPROM.update(address, minVal);
}

byte CalibrationCore::GetCalibrationDelay()
{
  return samplingDelay;
}

byte CalibrationCore::GetMax(byte row, byte col)
{
  //int address = (row*8) + col;
  int address = row * 8;
  address = address + col;
  address = address + MAXS_BASE_ADDR;
  byte maxValue = EEPROM.read(address);
  return maxValue;
}

byte CalibrationCore::GetMin(byte row, byte col)
{
  //int address = (row*8) + col + 96;
  int address = row * 8;
  address = address + col;
  address = address + MINS_BASE_ADDR;
  byte minValue = EEPROM.read(address);
  return minValue;
}