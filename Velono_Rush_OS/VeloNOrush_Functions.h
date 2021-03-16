#ifndef VeloNOrush_Functions
#define VeloNOrush_Functions

#define SENSOR_ROWS 12
#define SENSOR_COLS 8
#define CALIBRATE_TIME 7
#define CALIBRATE_SAMPLES 20

#include "Arduino.h"
#include "calibration.h"

class VeloNOrushCore:public CalibrationCore
{
  public:
    VeloNOrushCore(byte a, byte b, byte c, byte d);
    void ReadSensor(int row, int col);
    void ReadSensors();
    byte GetSensorVal(int row, int col);
    bool CalibrateMins();
    bool CalibrateMaxes();
  private:
    void SetMUX();
    void RowCol2Pins(int row, int col);
    //void GetNextRowCol();
    byte voltageReadings[SENSOR_ROWS][SENSOR_COLS];
    byte analogPin;
    byte muxPin;
    byte pinA;
    byte pinB;
    byte pinC;
    byte pinD;
};
#endif
