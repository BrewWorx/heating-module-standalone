#ifndef Vessel_h
#define Vessel_h

#include <Adafruit_MAX31865.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include <LittleFS.h>

#include "FileSystemService.h"
#include "PidConfig.h"

class Vessel
{
public:
    // Constructors
    Vessel(const char *id, int cs_pin);
    Vessel(const char *id, int cs_pin, double* secondaryInput, bool* secondaryAt);

    // Setters
    void setTemperature(double temp) { _setpoint = temp; }
    void setHeating(bool heating) { _active = heating; }

    void compute();

    double getInput() { return _input; }
    
    double output;                                                                          // PID output, 0-1
    bool at;                                                                                // Auto tune enabled
private:
    const char *_id;

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
};

#endif