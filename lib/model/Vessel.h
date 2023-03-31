#ifndef Vessel_h
#define Vessel_h

#include <Adafruit_MAX31865.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#include "FileSystemService.h"
#include "PidConfig.h"

class Vessel
{
public:
    // Constructors
    Vessel(int id, int cs_pin, int ssr_pin = 0);
    // Getters
    double getInput() { return _input; }
    DynamicJsonDocument getTelemetry();

    // Setters
    void setTemperature(double temp) { _setpoint = temp; }

    void compute();
    void toggleAutotune(bool atState);
private:
    int _id;
    int _ssr_pin;

    double _setpoint;
    double _input;

    double _output; // PID output, 0 - windowSize
    int _windowSize;
    unsigned long _windowStartTime;
    bool _at;       // Auto tune enabled

    byte _aTuneMode;
    double _aTuneStep;
    double _aTuneNoise;
    double _aTuneStartValue;
    unsigned int _aTuneLookBack;

    Adafruit_MAX31865 _tempSensor;
    bool _tempReady;
    uint16_t _rtdReg;

    PID _pid;
    PidConfig _pidConfig;
    PID_ATune _aTune;

    void _completeAutotune();
};

#endif