#include "calibration.h"
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

  EEPROM.update(address, maxVal);       //store max value in EEPROM
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

  EEPROM.update(address, minVal);       //store min value in EEPROM
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
    {18,20,21,27,30,27,37,32},
    {42,64,65,60,62,71,72,65},
    {66,245,70,75,70,94,112,99},
    {40,56,83,103,120,130,153,132},
    {45,64,76,60,97,94,95,102},
    {43,231,84,125,108,115,113,131},
    {48,83,88,109,142,140,156,130},
    {83,86,100,123,122,130,160,150},
    {53,82,74,63,76,78,96,87},
    {73,80,85,70,70,76,89,79},
    {57,61,75,110,109,106,110,113},
    {52,51,60,49,50,52,50,60}
  };
  //Default maximum values recorded manually stored in program data
  const byte maxDefaults[SENSOR_ROWS][SENSOR_COLS]
  {
    {110,170,160,145,185,187,197,196},
    {123,197,184,184,177,175,209,187},
    {177,245,180,131,144,184,200,180},
    {174,219,162,188,199,188,217,221},
    {149,207,190,208,197,195,199,189},
    {91,231,190,190,219,205,196,185},
    {156,202,161,200,210,190,242,161},
    {204,206,159,200,195,176,232,232},
    {203,199,135,170,156,167,211,211},
    {143,196,186,178,170,145,160,172},
    {183,218,149,193,200,202,230,234},
    {212,195,204,198,200,230,240,237}
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