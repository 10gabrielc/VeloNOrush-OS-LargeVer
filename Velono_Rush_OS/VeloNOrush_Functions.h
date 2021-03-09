#ifndef VeloNOrush_Functions
#define VeloNOrush_Functions

#define SENSOR_ROWS 11
#define SENSOR_COLS 8

#include "Arduino.h"

class VeloNOrushCore
{
  public:
    void VeloNOrushCore(byte a, byte b, byte c, byte d);
    void ReadSensors();
    void ReadSensor(byte row, byte col);
    byte GetSensorVal(byte row, byte col);
  private:
    void SetMUX(int pin);
    void RowCol2Pins(int row, int col);
    void GetNextRowCol();
    byte voltageReadings[SENSOR_ROWS][SENSOR_COLS];
    byte rowVal;
    byte colVal;
    byte analogPin;
    byte muxPin;
    byte storeCount;
    byte pinA;
    byte pinB;
    byte pinC;
    byte pinD;
    
}
