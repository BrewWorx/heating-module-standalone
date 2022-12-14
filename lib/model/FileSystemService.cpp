#include "FileSystemService.h"

PidConfig FileSystemService::readVesselConfig(const char *vesselId)
{
    PidConfig _pidConfig;

    if (LittleFS.begin())
    {
        File f = LittleFS.open(vesselId, "r");

        if (!f)
        {
            Serial.println(F("Failed to open configuration file for Vessel for reading"));
        }
        else
        {
            unsigned int readSize;

            readSize = f.readBytes((char *)&_pidConfig, sizeof(_pidConfig));

            if (readSize != sizeof(_pidConfig))
            {
                Serial.println("Read size mismatch for Vessel configuration");
            }
        }
    }
    else
    {
        Serial.println(F("Could not mount the file system"));
    }

    return _pidConfig;
}

bool FileSystemService::writeVesselConfig(const char *vesselId, PidConfig pidConfig)
{
    bool res = false;

    if (LittleFS.begin())
    {
        File f = LittleFS.open(vesselId, "w");

        if (!f)
        {
            Serial.println(F("Failed to open configuration file for Vessel for writing"));
        }
        else
        {

            Serial.println(F("Saving configuration for Vessel"));

            unsigned int writeSize;

            writeSize = f.write((byte *)&pidConfig, sizeof(pidConfig));

            if (writeSize != sizeof(pidConfig))
            {
                Serial.println(F("Write size mismatch for Vessel configuration"));
            }
            else
            {
                Serial.println(F("Vessel config saved to FS successfully"));
                res = true;
            }
        }
    }
    else
    {
        Serial.println(F("Could not mount the file system"));
    }

    return res;
}