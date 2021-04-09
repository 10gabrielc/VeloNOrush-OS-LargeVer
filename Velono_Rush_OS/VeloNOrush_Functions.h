#ifndef VeloNOrush_Functions
#define VeloNOrush_Functions

#define SENSOR_ROWS 12            //number of rows in sensor matrix
#define SENSOR_COLS 8             //number of columns in sensor matrix
#define LED_ROWS 11               //number of rows in LED matrix
#define LED_COLS 26               //number of columns in LED matrix
#define CALIBRATE_TIME 7          //default time in seconds for calibration
#define CALIBRATE_SAMPLES 20      //default number of samples per sensor

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

    byte voltageReadings[SENSOR_ROWS][SENSOR_COLS];   //sensor reading storage for a single loop
    byte ledBrightness[LED_ROWS][LED_COLS];           //brightness of each LED for a single loop
    byte analogPin;                                   //storage for which analog pin to check
    byte muxPin;                                      //storage for which MUX input to read
    byte pinA;                                        //storage for MUX pin A state
    byte pinB;                                        //storage for MUX pin B state
    byte pinC;                                        //storage for MUX pin C state
    byte pinD;                                        //storage for MUX pin D state
};
#endif
