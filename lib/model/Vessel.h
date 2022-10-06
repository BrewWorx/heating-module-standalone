#ifndef Vessel_h
#define Vessel_h

#include <Adafruit_MAX31865.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include <LittleFS.h>

struct PidConfig
{
    double kp;
    double ki;
    double kd;
};

class Vessel
{
public:
    // Constructors
    Vessel(unsigned int id, int cs_pin);
    Vessel(unsigned int id, int cs_pin, double* secondaryInput, bool* secondaryAt);

    // Setters
    void setTemperature(double temp) { _setpoint = temp; }
    void setHeating(bool heating) { _active = heating; }

    void compute();

    double getInput() { return _input; }

    void readConfigFromFlash();

    double output;                                                                          // PID output, 0-1
    bool at;                                                                                // Auto tune enabled
private:
    unsigned short int _id;

    double _setpoint;
    double _input;
    
    bool _active;

    double _secondaryOutput;
    bool _secondaryAt;
    

    int _windowSize;
    unsigned long _windowStartTime;

    Adafruit_MAX31865 _tempSensor;
    bool _tempReady;
    uint16_t _rtdReg;

    PID _pid;
    PidConfig _pidConfig;
    PID_ATune _aTune;

    void writeConfigToFlash();
};

#endif