#include "Vessel.h"

Vessel::Vessel(int id, int cs_pin, int ssr_pin) : _tempSensor(cs_pin, 0, 4, 5), _pid(&_input, &_output, &_setpoint, _pidConfig.kp, _pidConfig.ki, _pidConfig.kd, DIRECT), _aTune(&_input, &_output)
{
  _id = id;
  _input = 0;
  _setpoint = 0;
  _windowSize = 2000;
  _tempSensor.begin(MAX31865_4WIRE);
  _tempReady = false;

  _output = 0;
  _pidConfig = FileSystemService::readVesselConfig(_id);
  _pid.SetOutputLimits(0, _windowSize);
  _pid.SetMode(AUTOMATIC);

  _at = false;
  _aTuneStartValue = _windowSize / 2;
  _aTuneNoise = 1;
  _aTuneStep = 50;
  _aTuneLookBack = 20;

  _ssr_pin = ssr_pin;
  pinMode(_ssr_pin, OUTPUT);

  _pid.SetTunings(_pidConfig.kp, _pidConfig.ki, _pidConfig.kd);
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
  else
  {
    _tempReady = _tempSensor.readRTDAsync(_rtdReg);
    if (_tempReady)
    {
      _input = _tempSensor.temperatureAsync(_rtdReg, 100.0, 430.0);
      Serial.print(_id);
      Serial.print("| Temp:");
      Serial.print(_input);
      Serial.print(" Setpoint:");
      Serial.print(_setpoint);
      Serial.print(" | Power:");
      Serial.print(_output / _windowSize * 100);
      Serial.print("\% | P:");
      Serial.print(_pid.GetKp());
      Serial.print(" I:");
      Serial.print(_pid.GetKi());
      Serial.print(" D:");
      Serial.print(_pid.GetKd());
      Serial.print(" Autotune:");
      Serial.println(_at);
    }
  }

  if (_at)
  {
    if (_aTune.Runtime() != 0)
    {
      _completeAutotune();
    }
  }
  else
  {
    _pid.Compute();

    if (_ssr_pin != 0)
    {
      if (millis() - _windowStartTime > _windowSize)
      {
        _windowStartTime += _windowSize;
      }
      if (_output > millis() - _windowStartTime)
      {
        digitalWrite(_ssr_pin, HIGH);
      }
      else
      {
        digitalWrite(_ssr_pin, LOW);
      }
    }
  }
}

void Vessel::toggleAutotune(bool atState)
{
  if (atState == _at)
    return;

  if (atState)
  {
    Serial.println("Starting autotune");
    _at = true;
    _output = _aTuneStartValue;
    _aTune.SetNoiseBand(_aTuneNoise);
    _aTune.SetOutputStep(_aTuneStep);
    _aTune.SetLookbackSec((int)_aTuneLookBack);
    _aTuneMode = _pid.GetMode();
  }
  else
  {
    Serial.println("Autotune cancelled");
    _at = false;
    _aTune.Cancel();
    _pid.SetMode(_aTuneMode);
  }
}

DynamicJsonDocument Vessel::getTelemetry()
{
  DynamicJsonDocument doc(256);
  doc["temperature"] = round(_input * 100) / 100;
  doc["setpoint"] = _setpoint;
  doc["at"] = _at;
  doc["power"] = round(_output / _windowSize * 100);
  doc["P"] = round(_pidConfig.kp * 100) / 100;
  doc["I"] = round(_pidConfig.ki * 100) / 100;
  doc["D"] = round(_pidConfig.kd * 100) / 100;
  return doc;
}

void Vessel::_completeAutotune()
{
  _at = false;
  _pidConfig.kp = _aTune.GetKp();
  _pidConfig.ki = _aTune.GetKi();
  _pidConfig.kd = _aTune.GetKd();
  _pid.SetTunings(_pidConfig.kp, _pidConfig.ki, _pidConfig.kd);
  _pid.SetMode(_aTuneMode);

  Serial.print("Autotune complete, storing values | P:");
  Serial.print(_pidConfig.kp);
  Serial.print(" I:");
  Serial.print(_pidConfig.ki);
  Serial.print(" D:");
  Serial.println(_pidConfig.kd);

  FileSystemService::writeVesselConfig(_id, _pidConfig);
}