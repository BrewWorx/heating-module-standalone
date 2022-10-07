#include <LittleFS.h>

#include "PidConfig.h"

class FileSystemService
{
public:
    static PidConfig readVesselConfig(const char *vesselId);
    static bool writeVesselConfig(const char *vesselId, PidConfig pidConfig);
};