#ifndef callibration_h
#define callibration_h

#include "Arduino.h"
#include "VeloNOrush_Functions.h"

class CalibrateSensors
{
  public:
    CalibrateSensors(byte a, byte b, byte c, byte d);
    GetMaxes(int row, int col);
    
  private:
    byte pinA;
    byte pinB;
    byte pinC;
    byte pinD;
    int samplingDelay;
};
#endif
