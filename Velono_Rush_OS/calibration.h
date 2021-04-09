#ifndef callibration_h
#define callibration_h

#define SENSOR_ROWS 12            //number of rows in sensor matrix
#define SENSOR_COLS 8             //number of rows in sensor matrix     
#define MINS_BASE_ADDR 0          //base EEPROM address for min storage
#define MAXS_BASE_ADDR 96         //base EEPROM address for max storage

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
    void DefaultCalibration();
    
  private:
    byte samplingDelay;     //storage for delay between calibration loops     
};
#endif
