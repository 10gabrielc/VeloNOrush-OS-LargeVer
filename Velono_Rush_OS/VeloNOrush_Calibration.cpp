#include "VeloNOrush_Calibration.h"
#include <EEPROM.h>

// DEFAULT CONSTRUCTOR
CalibrationCore::CalibrationCore()
{
}

/*
    FUNCTION TO CALCULATE THE SAMPLE DELAY REQURED BETWEEN CALIBRATION
    LOOPS. TAKES A DESIRED TOTAL CALIBRATION TIME IN SECONDS FOR ALL
    SENSORS, AND THE NUMBER OF SAMPLES DESIRED PER SENSOR.
*/
void CalibrationCore::CalculateSampleDelay(int sampleTime, int numOfSamples)
{
  int tempMath = 0;                                   //variable for calculations

  tempMath = sampleTime * 1000;                       //convert seconds to milliseconds
  tempMath = tempMath / 96;                           //divide by number of sensors
  samplingDelay = tempMath / numOfSamples;            //divide total time needed by number of samples
}

/*
    FUNCTION TO STORE A "MAX" CALIBRATION VALUE INTO EEPROM.
    MAX VALUES REPRESENT THE HIGHEST READING POSSIBLE FOR A
    SENSOR, OCCURING WHEN THERE IS NO PRESSURE ON THE DEVICE.
    VALUES POSSIBLE ARE FROM 0 TO 255.
*/
void CalibrationCore::StoreMax(byte maxVal, byte row, byte col)
{
  int address = row * 8;                //storage for address, adjust based off row
  address = address + col;              //adjust based off column
  address = address + MAXS_BASE_ADDR;   //adjust based off base EEPROM address for max values

  byte shiftedMaxVal = 0;
  if(maxVal < MAXS_OFFSET)                  //verify that there will not be unsigned byte rollover
    shiftedMaxVal = 0;                      // when the max stored is adjusted
  else
    shiftedMaxVal = maxVal - MAXS_OFFSET;

  EEPROM.update(address, shiftedMaxVal);       //store max value in EEPROM
}

/*
    FUNCTION TO STORE A "MIN" CALIBRATION VALUE INTO EEPROM.
    MIN VALUES REPRESENT THE LOWEST READING POSSIBLE FOR A
    SENSOR, OCCURING WHEN A PERSON IS STANDING DIRECTLY ON
    A SENSOR. VALUES POSSIBLE ARE FROM 0 TO 255.
*/
void CalibrationCore::StoreMin(byte minVal, byte row, byte col)
{
  int address = row * 8;                //storage for address, adjust based off row
  address = address + col;              //adjust based off column
  address = address + MINS_BASE_ADDR;   //adjust based off base EEPROM address for min values

  byte shiftedMinVal = 0;
  if(minVal < MINS_OFFSET)
    shiftedMinVal = 0;
  else
    shiftedMinVal = minVal - MINS_OFFSET;

  EEPROM.update(address, shiftedMinVal);       //store min value in EEPROM
}

/*
    FUNCTION TO RETURN THE CALCULATED SAMPLE DELAY. VALUE
    RETURNED IS IN MILLISECONDS.
*/
byte CalibrationCore::GetCalibrationDelay()
{
  return samplingDelay;
}

/*
    FUNCTION TO RETURN A MAX VALUE FOR A SPECIFIC SENSOR.
    VALUE RETURNED IS FROM 0 TO 255. FUNCTION REQUIRES A
    ROW AND COLUMN TO READ THE VALUE FROM EEPROM.
*/
byte CalibrationCore::GetMax(byte row, byte col)
{
  int address = row * 8;                    //storage for address, adjust based off row
  address = address + col;                  //adjust based off column
  address = address + MAXS_BASE_ADDR;       //adjust based off base EEPROM address for max values

  byte maxValue = EEPROM.read(address);     //retrieve max calibration value from EEPROM
  
  return maxValue;
}

/*
    FUNCTION TO RETURN A MIN VALUE FOR A SPECIFIC SENSOR.
    VALUE RETURNED IS FROM 0 TO 255. FUNCTION REQUIRES A
    ROW AND COLUMN TO READ THE VALUE FROM EEPROM.
*/
byte CalibrationCore::GetMin(byte row, byte col)
{
  int address = row * 8;                    //storage for address, adjust based off row
  address = address + col;                  //adjust based off column
  address = address + MINS_BASE_ADDR;       //adjust based off base EEPROM address for min values

  byte minValue = EEPROM.read(address);     //retrieve min calibration value from EEPROM

  return minValue;
}

/*
    FUNCTION TO WRITE MANUAL CALIBRATION VALUES INTO THE EEPROM.
    CAN BE USED IN CASE SELF-CALIBRATION LEADS TO READ ERRORS.
    VALUES ARE BASED OFF HARD FLOORING AND ~120 POUDS OF FORCE.
*/
void CalibrationCore::DefaultCalibration()
{
  //Default minimum values recorded manually stored in program data
  const byte minDefaults[SENSOR_ROWS][SENSOR_COLS]
  {
    {4,7,8,6,5,6,5,5},
    {16,14,14,13,11,11,8,6},
    {19,27,20,31,20,11,6,5},
    {31,43,49,34,25,13,7,6},
    {18,27,35,36,30,15,6,5},
    {20,32,33,44,37,30,7,6},
    {19,23,29,25,24,8,5,6},
    {17,14,40,40,30,20,14,6},
    {14,14,17,19,19,16,6,6},
    {17,16,16,17,14,19,10,6},
    {17,15,17,16,15,11,7,7},
    {12,11,6,12,20,11,11,8}
  };
  //Default maximum values recorded manually stored in program data
  const byte maxDefaults[SENSOR_ROWS][SENSOR_COLS]
  {
    {108,115,130,110,150,130,160,125},
    {93,130,127,134,136,132,156,90},
    {140,147,149,105,126,129,145,125},
    {172,180,115,138,147,124,167,162},
    {125,150,133,158,159,156,175,164},
    {83,145,146,170,207,159,149,150},
    {109,151,130,123,138,94,161,115},
    {140,148,120,140,137,125,150,117},
    {135,144,86,113,108,121,145,163},
    {115,134,118,108,105,98,194,150},
    {161,168,119,127,125,143,162,170},
    {188,155,176,148,185,210,207,179}
  };

  //store all preset maxes and mins into EEPROM
  for(byte row = 0; row < SENSOR_ROWS; row++)
  {
    for(byte col = 0; col < SENSOR_COLS; col++)
    {
      //retrieve max and min values for current row and column
      byte tempMin = minDefaults[row][col];
      byte tempMax = maxDefaults[row][col];

      //store current min and max in EEPROM
      StoreMin(tempMin, row, col);
      StoreMax(tempMax, row, col);
    }
  }
}
