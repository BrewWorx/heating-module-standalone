#ifndef Vessel_h
#define Vessel_h

#include <Adafruit_MAX31865.h>
#include <PID_v1.h>
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
    // Constructor
    Vessel(unsigned int id, int cs_pin);

    // Setters
    void setTemperature(double temp) { _setpoint = temp; }
    void setHeating(bool heating) { _active = heating; }
    void setAutotune(bool at) { _at = at; }

    void compute();

    double getInput() { return _input; }

    void readConfigFromFlash();

private:
    unsigned short int _id;

    double _setpoint;
    double _input;
    double _output;

    double _kp;
    double _ki;
    double _kd;

    bool _at;
    bool _active;

    int _windowSize;
    unsigned long _windowStartTime;

    Adafruit_MAX31865 _tempSensor;
    bool _tempReady;
    uint16_t _rtdReg;

    PID _pid;

    PidConfig _pidConfig;


    void writeConfigToFlash();
};

#endif