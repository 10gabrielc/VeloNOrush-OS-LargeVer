#include "VeloNOrush_Modes.h"

JumpModeCore::JumpModeCore()
{
    currentFrame = 0;
    jumpDetected = false;
    jumpLocationRow = 0;
    jumpLocationCol = 0;
}

bool JumpModeCore::JumpCheck(byte sensorVal, byte sensorMin, int row, int col)
{
    byte jumpOffset = sensorMin + MINS_OFFSET;

    if(sensorVal <= jumpOffset)
    {
        //jump detected
        jumpDetected = true;
        if(row == SENSOR_ROWS-1)
            jumpLocationRow = LED_ROWS;
        else
            jumpLocationRow = row;
        jumpLocationCol = (int) col*3.25;
        currentFrame = 0;
    }

    return jumpDetected;
}

void JumpModeCore::NextFrame()
{
    if(currentFrame < (MAX_FRAMES - 1))
    {
        currentFrame = currentFrame + 1;
    }
    else
    {
        jumpDetected = false;
    }
}

bool JumpModeCore::GetAnimState()
{
    return jumpDetected;
}

byte JumpModeCore::GetPixel(int row, int col)
{
    //using the current frame, check for new LEDs to light
    int tempRow = 0;
    int tempCol = 0;
    byte lightingCode = PIXEL_IGNORE;

    //check upwards pixel
    tempCol = jumpLocationCol;
    tempRow = jumpLocationRow - (currentFrame / 3);
    if(tempRow >= 0)
    {
        if(row == tempRow && col == tempCol)
        {
            lightingCode = PIXEL_ON;
        }
    }

    //check downwards pixel
    tempRow = jumpLocationRow + (currentFrame/3);
    if(tempRow < LED_ROWS)
    {
        if(row == tempRow && col == tempCol)
        {
            lightingCode = PIXEL_ON;
        }
    }

    //check leftwards pixel
    tempCol = jumpLocationCol - currentFrame;
    tempRow = jumpLocationRow;
    if(tempCol >= 0)
    {
        if(row == tempRow && col == tempCol)
        {
            lightingCode = PIXEL_ON;
        }
    }

    //check rightwards pixel
    tempCol = jumpLocationCol + currentFrame;
    if(tempCol < LED_COLS)
    {
        if(row == tempRow && col == tempCol)
        {
            lightingCode = PIXEL_ON;
        }
    }

    //After 5 frames, start turning off lights
    if(currentFrame >= 6)
    {
        //check upwards pixel
        tempCol = jumpLocationCol;
        tempRow = jumpLocationRow - ((currentFrame/3) - 2);
        if(tempRow >= 0)
        {
            if(row == tempRow && col == tempCol)
            {
                lightingCode = PIXEL_OFF;
            }
        }

        //check downwards pixel
        tempRow = jumpLocationRow + ((currentFrame/3) - 2);
        if(tempRow < LED_ROWS)
        {
            if(row == tempRow && col == tempCol)
            {
                lightingCode = PIXEL_OFF;
            }
        }

        //check leftwards pixel
        tempCol = jumpLocationCol - (currentFrame - 6);
        tempRow = jumpLocationRow;
        if(tempCol >= 0)
        {
            if(row == tempRow && col == tempCol)
            {
                lightingCode = PIXEL_OFF;
            }
        }

        //check rightwards pixel
        tempCol = jumpLocationCol + currentFrame - 6;
        if(tempCol < LED_COLS)
        {
            if(row == tempRow && col == tempCol)
            {
                lightingCode = PIXEL_OFF;
            }
        }
    }

    //Return either a pixel on, off, or ignored
    return lightingCode;
}