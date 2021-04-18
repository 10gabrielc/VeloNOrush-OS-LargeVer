#include "VeloNOrush_Functions.h"

/*
    DEFAULT CONSTRUCTOR.
    INITIALIZES PINS ON THE ARDUINO TO CONTROL THE
    MULTIPLEXOR AND READ FROM THE ANALOG INPUTS
*/
VeloNOrushCore::VeloNOrushCore(byte a, byte b, byte c, byte d)
{
  //store passed pins for mux inputs in private memory
  pinA = a; 
  pinB = b;
  pinC = c;
  pinD = d;

  //initialize MUX pins as inputs
  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);

  //enable ALL analog pins as inputs: the large version requires
  //all analog inputs!
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);

  //calculate the sample calibration delay based off default values
  CalculateSampleDelay(CALIBRATE_TIME, CALIBRATE_SAMPLES);
}

/*
    FUNCTION TO READ DATA FROM A SPECIFIC SENSOR IN THE SENSOR
    MATRIX. TARGETS A SPECIFIC SENSOR USING ROW AND COLUMN.
    STORES THE RESULT IN PRIVATE MEMORY (voltageReadings)
*/
void VeloNOrushCore::ReadSensor(byte row, byte col)
{
  //configure pins to select a specific sensor to read
  RowCol2Pins(row, col);
  SetMUX();

  //get the voltage at the sensor's node. Divide the
  // result by 4 to truncate the data and fit the result within
  // a single byte!
  byte voltageVal = analogRead(analogPin) / 4;
  
  //retrieve the min and max values for this sensor from EEPROM
  byte tempMax = GetMax(row, col);    
  byte tempMin = GetMin(row, col);
  
  //adjust current reading to keep value within min and max bounds
  if(voltageVal > tempMax)
    voltageVal = tempMax;
  if(voltageVal < tempMin)
    voltageVal = tempMin;
  
  //store current sensor reading into private memory
  voltageReadings[row][col] = voltageVal;

  //small delay to allow Arduino ADC to reset
  delayMicroseconds(100);
}

/*
    FUNCTION TO READ ALL SENSORS IN THE SENSOR MATRIX SEQUENTIALLY.
    UNLESS A SPECIFIC SENSOR'S VALUE IS NEEDED, THIS IS THE
    SUGGESTED METHOD FOR POLLING ALL SENSORS.
*/
void VeloNOrushCore::ReadSensors()
{
  //for loop for iterating through each of the sensors in the array
  for(byte rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(byte colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      //read and store the data of a specific sensor
      // based off row and column
      ReadSensor(rowCounter, colCounter); 
    }
  }
}

/*
    FUNCTION TO RETURN A SENSOR VALUE FROM MEMORY BASED OFF A ROW AND
    COLUMN. VALUE RETURED IS A BYTE.
*/
byte VeloNOrushCore::GetSensorVal(byte row, byte col)
{
  return voltageReadings[row][col];
}

/*
    FUNCTION TO CONVERT SENSOR READINGS INTO LED BRIGHTNESSES.
    THE SENSOR ARRAY FOR THE LARGE VERSION IS 12x8, WHILE THE
    LED ARRAY IS 11x26: A SPECIAL MAPPING ALGORITHM IS THEN
    REQUIRED TO DISPLAY POSITION ACCURATELY!
*/
void VeloNOrushCore::MapSensorsToBrightness(byte maxVal)
{
  //clear out the array of all the brightnesses initially
  for(byte ledRow = 0; ledRow < LED_ROWS; ledRow++)
  {
    for(byte ledCol = 0; ledCol < LED_COLS; ledCol++)
    {
      ledBrightness[ledRow][ledCol] = 0;
    }
  }

  //iterate through each row of the stored voltages
  for(byte row = 0; row < SENSOR_ROWS; row++)
  {
    //create a storage for the current column/led being targeted
    int currentColLED = 0;
    
    //iterate through each column of the stored voltages
    for(byte col = 0; col < SENSOR_COLS; col++)
    {
      //retrieve a sensor's last reading, min, and max value
      byte currentSensorVal = GetSensorVal(row, col);
      byte currentMax = GetMax(row, col);
      byte currentMin = GetMin(row, col);
      
      //calculate the current "brightness" that a specific sensor is generating
      byte curBr = map(
                        currentSensorVal, 
                        currentMax, 
                        currentMin, 
                        0, maxVal
                       );

      //store pre-converted forms of the brightness to be assigned to LEDs
      byte curBr2 = curBr/2;
      byte curBr4 = curBr/4;
      
      //START WITH TOP EDGE OF THE SENSOR MATRIX
      if(row == 0)
      {
        //we can only assign values to LEDs below this row, as there are no
        //leds above these sensors

        //START WITH CHECKING FOR THE LEFT EDGE OF THE MATRIX
        if(col == 0)
        {
          ledBrightness[row][currentColLED] += curBr2;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr4;
          ledBrightness[row][currentColLED+3] += curBr4;
          
          currentColLED+=3;
        }
        else if(col == SENSOR_COLS-1) //CHECK FOR THE RIGHT EDGE OF THE MATRIX
        {
          ledBrightness[row][currentColLED-1] += curBr4;
          ledBrightness[row][currentColLED] += curBr4;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr2;
          
          currentColLED+=3;
        }
        else  //ALL OTHER COLUMNS INBETWEEN
        {
          if(col == 2 || col == 5)
          {
            ledBrightness[row][currentColLED-1] += curBr4;    
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr2;
            ledBrightness[row][currentColLED+3] += curBr4;
            ledBrightness[row][currentColLED+4] += curBr4;
            
            currentColLED+=4;
          }
          else
          {
            ledBrightness[row][currentColLED-1] += curBr4;
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr4;
            ledBrightness[row][currentColLED+3] += curBr4;
            
            currentColLED+=3;
          }
        }
        
      }
      else if(row == (SENSOR_ROWS-1)) //CHECK IF AT THE BOTTOM EDGE OF THE SENSOR MATRIX
      {
        //need to also check if we are in the first or last column
        if(col == 0)
        {
          ledBrightness[row-1][currentColLED] += curBr2;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr4;
          ledBrightness[row-1][currentColLED+3] += curBr4;
          
          currentColLED+=3;
        }
        else if(col == SENSOR_COLS-1)
        {
          ledBrightness[row-1][currentColLED-1] += curBr4;
          ledBrightness[row-1][currentColLED] += curBr4;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr2;
          
          currentColLED+=3;
        }
        else
        {
          if(col == 2 || col == 5)
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;    
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr2;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            ledBrightness[row-1][currentColLED+4] += curBr4;
            
            currentColLED+=4;
          }
          else
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr4;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            
            currentColLED+=3;
          }
        }
      }
      else  //HANDLE ALL THE ROWS INBETWEEN
      {
        //need to also check if we are in the first or last column
        if(col == 0)
        {
          ledBrightness[row-1][currentColLED] += curBr2;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr4;
          ledBrightness[row-1][currentColLED+3] += curBr4;
          
          ledBrightness[row][currentColLED] += curBr2;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr4;
          ledBrightness[row][currentColLED+3] += curBr4;
          
          currentColLED+=3;
        }
        else if(col == SENSOR_COLS-1)
        {
          ledBrightness[row-1][currentColLED-1] += curBr4;
          ledBrightness[row-1][currentColLED] += curBr4;
          ledBrightness[row-1][currentColLED+1] += curBr2;
          ledBrightness[row-1][currentColLED+2] += curBr2;
          
          ledBrightness[row][currentColLED-1] += curBr4;
          ledBrightness[row][currentColLED] += curBr4;
          ledBrightness[row][currentColLED+1] += curBr2;
          ledBrightness[row][currentColLED+2] += curBr2;
          currentColLED+=3;
        }
        else
        {
          if(col == 2 || col == 5)
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;    
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr2;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            ledBrightness[row-1][currentColLED+4] += curBr4;
            
            ledBrightness[row][currentColLED-1] += curBr4;    
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr2;
            ledBrightness[row][currentColLED+3] += curBr4;
            ledBrightness[row][currentColLED+4] += curBr4;
            
            currentColLED+=4;
          }
          else
          {
            ledBrightness[row-1][currentColLED-1] += curBr4;
            ledBrightness[row-1][currentColLED] += curBr4;
            ledBrightness[row-1][currentColLED+1] += curBr2;
            ledBrightness[row-1][currentColLED+2] += curBr4;
            ledBrightness[row-1][currentColLED+3] += curBr4;
            
            ledBrightness[row][currentColLED-1] += curBr4;
            ledBrightness[row][currentColLED] += curBr4;
            ledBrightness[row][currentColLED+1] += curBr2;
            ledBrightness[row][currentColLED+2] += curBr4;
            ledBrightness[row][currentColLED+3] += curBr4;
            
            currentColLED+=3;
          }
        }
      }
    }
  }
}

/*
    FUNCTION TO STORE A BRIGHTNESS VALUE FOR A SPECIFIC LED BASED
    OFF A ROW AND COLUMN. ACCEPTS A BYTE AS BRIGHTNESS VALUE.
*/
void VeloNOrushCore::SetBrightness(byte val, byte row, byte col)
{
  ledBrightness[row][col] = val;
}

/*
    FUNCTION TO RETRIEVE A BRIGHTNESS FROM MEMORY BASED OFF PASSED
    ROW AND COLUMN. RETURNS A VALUE FROM 0-255
*/
byte VeloNOrushCore::GetBrightness(byte row, byte col)
{
  byte brightness = ledBrightness[row][col];
  return brightness;
}

/*
    FUNCTION TO CALIBRATE THE MINIMUMS OF EACH OF THE SENSORS. DUE TO
    THE REQUIRED STEPS OF CALIBRATING THESE MINIMUMS, THEY ARE HARD-CODED
    AND MUST BE MANUALLY CHANGED TO ADJUST THE MAX POSSIBLE PRESSURE. 
    A MINIMUM VALUE SIGNIFIES THE MINIMUM VOLTAGE READING POSSIBLE, WHICH
    IN TURN MEANS THE MOST PRESSURE BEING APPLIED TO A SENSOR.
*/
bool VeloNOrushCore::CalibrateMins()
{
  //because the minimums require each sensor to be stepped on in turn
  // (and takes a long time due to 96 sensors), these pre-calculated
  // values are what is used to calibrate the minimum values.
  const byte minDefaults[SENSOR_ROWS][SENSOR_COLS]
  {
    {4,7,8,6,5,6,5,5},
    {16,14,14,13,11,11,8,6},
    {19,27,20,31,20,11,6,5},
    {31,43,49,34,25,13,7,6},
    {18,27,27,31,25,10,6,5},
    {20,27,28,39,32,25,7,6},
    {19,18,24,20,19,8,5,6},
    {17,14,40,40,30,20,14,6},
    {14,14,12,14,14,11,6,6},
    {17,16,16,17,14,19,10,6},
    {17,15,17,16,15,11,7,7},
    {12,11,6,12,20,11,11,8}
  };
  
  //iterate through each sensor and store the value in EEPROM
  for(byte rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(byte colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      byte tempVal = minDefaults[rowCounter][colCounter];
      StoreMin(tempVal, rowCounter, colCounter);
    }
  }

  //return a 1 to signify calibration success
  return true;
}

/*
    FUNCTION TO CALIBRATE THE MAXIMUMS OF EACH OF THE SENSORS. THIS 
    MUST BE PERFORMED WHILE THERE IS NO PRESSURE BEING APPLIED TO THE
    DEVICE. THE SYSTEM TAKES AN AVERAGE OF THE READINGS TO IMPROVE
    ACCURACY. A MAXIMUM VALUE REPRESENTS THE MAXIMUM A VOLTAGE READING
    OF A SENSOR CAN BE, WHICH IN TURN RESEMBLES WHEN THERE IS MINIMAL
    PRESSURE.
*/
bool VeloNOrushCore::CalibrateMaxes()
{
  const int thresholdOffset = 0;                //offset to help keep LEDs off when there is no pressure
  byte sampleDelay = GetCalibrationDelay();     //retrieve the calibration delay time between loops

  //for loop for iterating through each of the sensors in the array
  for(byte rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(byte colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      //storage for sums needed when averaging
      unsigned long maxSum = 0;                 

      //sample a sensor a specific number of times
      for(int samples = 0; samples < CALIBRATE_SAMPLES; samples++)
      {
        ReadSensor(rowCounter, colCounter);                             //retrieve specific sensor reading
        volatile byte tempRead = GetSensorVal(rowCounter, colCounter);  //store reading
        maxSum += tempRead;                                             //add to continuous sum
        delay(sampleDelay);                                             //wait calculated amount of time
      }

      //find the average of the sensor value polled
      byte newMaxVal = maxSum / CALIBRATE_SAMPLES;      //find the average through division
      newMaxVal = maxSum - thresholdOffset;             //adjust based off desired offset

      //call the calibration object to store the data in EEPROM
      StoreMax(newMaxVal, rowCounter, colCounter);
    }
  }
  return true;
}

/*
    FUNCTION TO SET THE SELECTION PINS (A,B,C,D) ON ALL MULTIPLEXERS
    USES MUX PIN SELECTION BETWEEN 0 AND 15 TO TOGGLE SELECTION PINS
*/
void VeloNOrushCore::SetMUX()
{
  bool pinAval, pinBval, pinCval, pinDval = 0;    //bit storage of each input pin
  int readPin = muxPin;                           //storage of pin number to work with

  //convert the decimal pin number to binary
  pinAval = readPin % 2;
  readPin/=2;
  pinBval = readPin % 2;
  readPin/=2;
  pinCval = readPin % 2;
  readPin/=2;
  pinDval = readPin % 2;

  //set the pins of all MUXes
  digitalWrite(pinA, pinAval);
  digitalWrite(pinB, pinBval);
  digitalWrite(pinC, pinCval);
  digitalWrite(pinD, pinDval);
}

/*    
    FUNCTION TO CALCULATE WHICH MUX CHIP TO READ FROM, AND WHICH PIN
    ON THE MUX CHIP TO ENABLE. REQUIRES A ROW AND COLUMN.
*/
void VeloNOrushCore::RowCol2Pins(byte row, byte col)
{
  //use the equation below to convert the row and column to mux pin and analog pin.
  //Each analog pin is linked to 16 mux pins essentially.
  analogPin = 14 + (row / 4) + (col / 4 * 3);
  muxPin = (row % 4) * 4 + (col % 4);
}
