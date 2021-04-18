#ifndef VeloNOrush_Modes
#define VeloNOrush_Modes

#define SENSOR_ROWS 12          //number of rows in sensor matrix
#define SENSOR_COLS 8           //number of cols in sensor matrix
#define LED_ROWS 11             //number of rows in LED matrix
#define LED_COLS 26             //number of cols in LED matrix
#define PIXEL_ON 1              //code for a pixel to be turned on
#define PIXEL_OFF 2             //code for a pixel to be turned off
#define PIXEL_IGNORE 0          //code for a pixel to be ignored
#define MAX_FRAMES 42           //max number of frames for a jump animation

#include "Arduino.h"

class JumpModeCore
{
    public:
        JumpModeCore();
        bool JumpCheck(byte sensorVal, byte sensorMin, int row, int col);
        void NextFrame();
        bool GetAnimState();
        byte GetPixel(int row, int col);
    private:
        byte currentFrame;
        bool jumpDetected;
        int jumpLocationRow;
        int jumpLocationCol;
};
class DanceModeCore
{
    public:
        DanceModeCore();
    private:
};
class LightShowCore
{
    public:
        LightShowCore();
    private:

};
#endif
