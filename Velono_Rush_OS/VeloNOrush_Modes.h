#ifndef VeloNOrush_Modes
#define VeloNOrush_Modes

#define SENSOR_ROWS 12
#define SENSOR_COLS 8
#define LED_ROWS 11
#define LED_COLS 26

class JumpModeCore
{
    public:
        JumpModeCore();
        void JumpCheck();
    private:
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