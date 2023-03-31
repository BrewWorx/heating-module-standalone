#include <LittleFS.h>

#include "PidConfig.h"

class FileSystemService
{
public:
    static PidConfig readVesselConfig(int vesselId);
    static bool writeVesselConfig(int vesselId, PidConfig pidConfig);
};