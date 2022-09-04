#include "Vessel.h"

Vessel::Vessel(unsigned int id, int cs_pin) : _tempSensor(cs_pin, 0, 4, 5), _pid(&_input, &_output, &_setpoint, _kp, _ki, _kd, DIRECT)
{
  _input = 0;
  _setpoint = 0;
  _output = 0;
  _active = false;
  _at = false;
  _windowSize = 2000;
  _tempSensor.begin(MAX31865_4WIRE);

  readConfigFromFlash();
}

/**
 * Read temperature and compute PID values.
 */
void Vessel::compute()
{
  _tempSensor.readRTD();

  // Log fault states
  uint8_t fault = _tempSensor.readFault();
  if (fault)
  {
    Serial.print(F("Fault 0x"));
    Serial.println(fault, HEX);
  }

  _input = _tempSensor.temperature(100.0, 430.0);
  _pid.Compute();
}

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

      writeSize = f.write((byte*) &_pidConfig, sizeof(_pidConfig));

      if (writeSize != sizeof(_pidConfig))
      {
        Serial.println(F("Write size mismatch for Vessel ") + idStr + F(" configuration"));
      } else {
        Serial.println(F("Config for Vessel ") + idStr + F(" saved to FS successfully"));
      }
    }
  }
  else
  {
    Serial.println(F("Could not mount the file system"));
  }
}