#ifndef VeloNOrush_Functions
#define VeloNOrush_Functions

#define SENSOR_ROWS 12
#define SENSOR_COLS 8
#define LED_ROWS 11
#define LED_COLS 26
#define CALIBRATE_TIME 7
#define CALIBRATE_SAMPLES 20

#include "Arduino.h"
#include "calibration.h"

class VeloNOrushCore:public CalibrationCore
{
  public:
    VeloNOrushCore(byte a, byte b, byte c, byte d);
    void ReadSensor(byte row, byte col);
    void ReadSensors();
    byte GetSensorVal(byte row, byte col);
    void MapSensorsToBrightness(byte maxVal);
    void SetBrightness(byte val, byte row, byte col);
    byte GetBrightness(byte row, byte col);
    bool CalibrateMins();
    bool CalibrateMaxes();
  private:
    void SetMUX();
    void RowCol2Pins(byte row, byte col);
    byte voltageReadings[SENSOR_ROWS][SENSOR_COLS];
    byte ledBrightness[LED_ROWS][LED_COLS];
    byte analogPin;
    byte muxPin;
    byte pinA;
    byte pinB;
    byte pinC;
    byte pinD;
};
#endif
