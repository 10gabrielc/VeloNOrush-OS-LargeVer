#ifndef callibration_h
#define callibration_h

#define SENSOR_ROWS 12
#define SENSOR_COLS 8

#include "Arduino.h"

class CalibrationCore
{
  public:
    CalibrationCore();
    void CalculateSampleDelay(int sampleTime, int numOfSamples);
    void StoreMax(byte maxVal, int row, int col);
    void StoreMin(byte minVal, int row, int col);
    int GetCalibrationDelay();
    byte GetMax(int row, int col);
    byte GetMin(int row, int col);
    
  private:
    int samplingDelay;
    //byte maxOffsets[SENSOR_ROWS][SENSOR_COLS];
    //byte minOffsets[SENSOR_ROWS][SENSOR_COLS];
};
#endif

/*
  void LoadCalibrationVals();
 */
