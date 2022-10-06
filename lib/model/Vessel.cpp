#include "Vessel.h"

Vessel::Vessel(unsigned int id, int cs_pin) : _tempSensor(cs_pin, 0, 4, 5), _pid(&_input, &output, &_setpoint, _pidConfig.kp, _pidConfig.ki, _pidConfig.kd, DIRECT), _aTune(&_input, &output)
{
  _id = id;
  _input = 0;
  _setpoint = 0;
  _active = false;
  _windowSize = 2000;
  _tempSensor.begin(MAX31865_4WIRE);
  _tempReady = false;

  output = 0;
  at = false;

  readConfigFromFlash();
}

Vessel::Vessel(unsigned int id, int cs_pin, double *secondaryInput, bool *secondaryAt) : _tempSensor(cs_pin, 0, 4, 5), _pid(&_input, &output, &_setpoint, _pidConfig.kp, _pidConfig.ki, _pidConfig.kd, DIRECT), _aTune(&_input, &output)
{
  _id = id;
  _input = 0;
  _setpoint = 0;
  _active = false;
  _windowSize = 2000;
  _tempSensor.begin(MAX31865_4WIRE);
  _tempReady = false;

  output = 0;
  at = false;

  _secondaryAt = secondaryAt;

  readConfigFromFlash();
}

/**
 * Read temperature and compute PID values.
 */
void Vessel::compute()
{
  // Log fault states
  uint8_t fault = _tempSensor.readFault();
  if (fault)
  {
    Serial.print(F("Fault 0x"));
    Serial.println(fault, HEX);
  }

  if (_secondaryAt)
  {
    at = false;
    _input = _secondaryOutput * 100; // Output of secondary vessel PID * 100 (celcius, maximum water temperature)
  }
  else
  {
    _secondaryAt = false;
    _tempReady = _tempSensor.readRTDAsync(_rtdReg);
    if (_tempReady)
    {
      _input = _tempSensor.temperatureAsync(_rtdReg, 100.0, 430.0);
    }
  }

  if (at) {
    if (_aTune.Runtime() != 0)
      {
        at = false;
        _pidConfig.kp = _aTune.GetKp();
        _pidConfig.ki = _aTune.GetKi();
        _pidConfig.kd = _aTune.GetKd();
        _pid.SetTunings(_pidConfig.kp, _pidConfig.ki, _pidConfig.kd);

        //AutoTuneHelper(false, hltPID);
      }
  } else {
    _pid.Compute();
  }
}

/**
 * Read vessel configuration from Flash memory
 */
void Vessel::readConfigFromFlash()
{
  if (LittleFS.begin())
  {
    String idStr = String(_id);

    File f = LittleFS.open(idStr, "r");

    if (!f)
    {
      Serial.println(F("Failed to open configuration file for Vessel ") + idStr + F(" for reading"));
    }
    else
    {
      unsigned int readSize;

      readSize = f.readBytes((char *)&_pidConfig, sizeof(_pidConfig));

      if (readSize != sizeof(_pidConfig))
      {
        Serial.println(F("Read size mismatch for Vessel ") + idStr + F(" configuration"));
      }
    }
  }
  else
  {
    Serial.println(F("Could not mount the file system"));
  }
}

/**
 * Write vessel configuration to Flash memory
 */
void Vessel::writeConfigToFlash()
{
  if (LittleFS.begin())
  {
    String idStr = String(_id);

    File f = LittleFS.open(idStr, "w");

    if (!f)
    {
      Serial.println(F("Failed to open configuration file for Vessel ") + idStr + F(" for writing"));
    }
    else
    {

      Serial.println(F("Saving configuration for Vessel ") + idStr);

      unsigned int writeSize;

      writeSize = f.write((byte *)&_pidConfig, sizeof(_pidConfig));

      if (writeSize != sizeof(_pidConfig))
      {
        Serial.println(F("Write size mismatch for Vessel ") + idStr + F(" configuration"));
      }
      else
      {
        Serial.println(F("Config for Vessel ") + idStr + F(" saved to FS successfully"));
      }
    }
  }
  else
  {
    Serial.println(F("Could not mount the file system"));
  }
}