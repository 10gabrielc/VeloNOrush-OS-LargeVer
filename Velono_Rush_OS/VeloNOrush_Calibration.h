#ifndef VeloNOrush_Calibration
#define VeloNOrush_Calibration

#define SENSOR_ROWS 12            //number of rows in sensor matrix
#define SENSOR_COLS 8             //number of rows in sensor matrix     
#define MINS_BASE_ADDR 0          //base EEPROM address for min storage
#define MAXS_BASE_ADDR 96         //base EEPROM address for max storage
#define MINS_OFFSET 5             //number to offset stored min calibration values
#define MAXS_OFFSET 20            //number to offset stored max calibration values

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
