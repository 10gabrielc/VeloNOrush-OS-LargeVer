#include "VeloNOrush_Functions.h"

VeloNOrushCore::VeloNOrushCore(byte a, byte b, byte c, byte d)
{
  pinA = a;
  pinB = b;
  pinC = c;
  pinD = d;

  pinMode(pinA, OUTPUT);
  pinMode(pinB, OUTPUT);
  pinMode(pinC, OUTPUT);
  pinMode(pinD, OUTPUT);
  
}

void VeloNOrushCore::ReadSensors()
{
  //for loop for iterating through each of the sensors in the array
  for(int rowCounter = 0; rowCounter < SENSOR_ROWS; rowCounter++)
  {
    for(int colCounter = 0; colCounter < SENSOR_COLS; colCounter++)
    {
      ReadSensor(rowCounter, colCounter);
    }
  }
}

void VeloNOrushCore::ReadSensor(int row, int col)
{
  //variables needed
  int voltageVal = 0;
  const byte sensorMinOffsets[numOfRows][numOfCols] = {{10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10},
                                                      {10,10,10,10,10,10,10,10}};

  RowCol2Pins(row, col);
  SetMUX(muxPin);
  voltageVal = analogRead(analogPin) / 4;
  delayMicroseconds(100);
  if(voltageVal > sensorMaxOffsets[row][col])
      voltageVal = sensorMaxOffsets[row][col];
  if(voltageVal < sensorMinOffsets[row][col])
    voltageVal = sensorMinOffsets[row][col];
  
  voltageReadings[row][col] = voltageVal;

}




void VeloNOrushCore::SetMUX(int pin)
{
  bool pinAval, pinBval, pinCval, pinDval = 0;    //bit storage of each input pin
  int readPin = pinToRead;                        //storage of pin number to work with

  //convert the decimal pin number to binary
  pinAval = pinToRead % 2;
  pinToRead/=2;
  pinBval = pinToRead % 2;
  pinToRead/=2;
  pinCval = pinToRead % 2;
  pinToRead/=2;
  pinDval = pinToRead % 2;

  //set the pins of all MUXes
  digitalWrite(muxPinA, pinAval);
  digitalWrite(muxPinB, pinBval);
  digitalWrite(muxPinC, pinCval);
  digitalWrite(muxPinD, pinDval);
}

void VeloNOrushCore::RowCol2Pins(int row, int col)
{
  //use the equation below to convert the row and column to mux pin and analog pin
  //Each analog pin controls 16 mux pins
  analogPin = 14 + (row / 4) + (col / 4 * 3);
  muxPin = (row % 4) * 4 + (col % 4);

}

void VeloNOrushCore::GetNextRowCol()
{
  //Handle determining which location to store the sensor's data. Because there are 6 muxes,
  //with each MUX reading data in squares of 4x4, the data needs to be reorganized and
  //cannot simply be stored in sequential order. Otherwise reading the data would be confusin
  //for later algorithms
  if(storeCount < (96/2))
  {
    //reset the column counter after 4 columns, and increment row counter
    if(col >= 3)
    {
      col = 0;
      row++;
    }
    else
      col++;
      
  }
  //at the middle point, transition to polling the right side of the sensor array
  else if(storeCount == (96/2))
  {
    col = 4;
    row = 0;
  }
  else  //(storageCounter > 48)
  {
    if(col >= 7)
    {
      col = 4;
      row++;
    }
    else
      col++;
  }
}
