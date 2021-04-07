#ifndef VeloNOrush_Modes
#define VeloNOrush_Modes

#define SENSOR_ROWS 12
#define SENSOR_COLS 8
#define LED_ROWS 11
#define LED_COLS 26
#define MINS_OFFSET 5
#define PIXEL_ON 1
#define PIXEL_OFF 2
#define PIXEL_IGNORE 0
#define MAX_FRAMES 42

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
