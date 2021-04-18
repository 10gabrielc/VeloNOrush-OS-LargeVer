#include <EEPROM.h>

//pins used for buttons
const byte btnAdvancePin = 2;

//Select the pins that will count for the A,B,C, and D
const byte muxPinA = 7;
const byte muxPinB = 8;
const byte muxPinC = 9;
const byte muxPinD = 10;

//constants to describe sensor matrix
const byte numOfSensors = 96;
const byte numOfRows = 12;
const byte numOfCols = 8;

//loop variables
const int loopDelay = 100;

//constants throughout the code
const byte adcDelay = 100;

byte muxPin = 0;
byte analogPin = 0;

//storage of maxes
byte minVals[numOfRows][numOfCols];

//Function to blink the onboard LED of the UNO over 1 second
void BlinkLEDs(int blinkCount, int blinkTime)
{
  int delayAmount = ((blinkTime*1000) / (blinkCount * 2));

  for (int i = 0; i < blinkCount; i++)
  {
    digitalWrite(13, HIGH);
    delay(delayAmount);
    digitalWrite(13, LOW);
    delay(delayAmount);
  }
}

void SetMUX()
{
  bool pinAval, pinBval, pinCval, pinDval = 0;    //bit storage of each input pin
  byte readPin = muxPin;                          //storage of pin number to work with

  //convert the decimal pin number to binary
  pinAval = readPin % 2;
  readPin /= 2;
  pinBval = readPin % 2;
  readPin /= 2;
  pinCval = readPin % 2;
  readPin /= 2;
  pinDval = readPin % 2;

  //set the pins of all MUXes
  digitalWrite(muxPinA, pinAval);
  digitalWrite(muxPinB, pinBval);
  digitalWrite(muxPinC, pinCval);
  digitalWrite(muxPinD, pinDval);
}

void RowCol2Pins(byte row, byte col)
{
  //use the equation below to convert the row and column to mux pin and analog pin
  //Each analog pin controls 16 mux pins
  analogPin = 14 + (row / 4) + (col / 4 * 3);
  muxPin = (row % 4) * 4 + (col % 4);
}

void setup() 
{
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  pinMode(muxPinA, OUTPUT);
  pinMode(muxPinB, OUTPUT);
  pinMode(muxPinC, OUTPUT);
  pinMode(muxPinD, OUTPUT);
  pinMode(btnAdvancePin, INPUT_PULLUP);

  Serial.begin(9600);

  for(int row = 0; row < numOfRows; row++)
  {
    for(int col = 0; col < numOfCols; col++)
    {
      minVals[row][col] = 255;
    }
  }
}

void loop() 
{
  
  // put your main code here, to run repeatedly:
  for(byte row = 0; row < numOfRows; row++)
  {
    for(byte col = 0; col < numOfCols; col++)
    {
      volatile uint8_t voltStorage = 0;
      while(digitalRead(btnAdvancePin) == HIGH)
      {
        RowCol2Pins(row, col);
        SetMUX();
        voltStorage = (byte) (analogRead(analogPin) / 4);
        Serial.print(row);
        Serial.print(col);
        Serial.print(" | ");
        Serial.println(voltStorage);

        byte lastValue = minVals[row][col];
        if(voltStorage < lastValue)
          minVals[row][col] = voltStorage;
        delay(loopDelay);
      }
      Serial.print(minVals[row][col]);
      Serial.println(" stored.");
      BlinkLEDs(10, 1);
    }
  }
  while(true)
  {
    Serial.println("---------------------------------------");
    for (int row = 0; row < numOfRows; row++)
    {
      for (int col = 0; col < numOfCols; col++)
      {
        Serial.print(minVals[row][col]);
        Serial.print("  ");
      }
      Serial.println(" ");
    }
    Serial.println("-----------------------------------------");
    BlinkLEDs(2, 1);
  }
}
