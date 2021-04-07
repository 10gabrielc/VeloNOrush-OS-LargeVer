#ifndef callibration_h
#define callibration_h

#define SENSOR_ROWS 12
#define SENSOR_COLS 8
#define MIN_OFFSET 5
#define MINS_BASE_ADDR 0
#define MAXS_BASE_ADDR 96

#include "Arduino.h"

class CalibrationCore
{
  public:
    CalibrationCore();
    void CalculateSampleDelay(int sampleTime, int numOfSamples);
    void StoreMax(byte maxVal, byte row, byte col);
    void StoreMin(byte minVal, byte row, byte col);
    byte GetCalibrationDelay();
    byte GetMax(byte row, byte col);
    byte GetMin(byte row, byte col);
    
  private:
    byte samplingDelay;
};
#endif
