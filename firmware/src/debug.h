#pragma once

#include <time.h>

#include "app_shared.h"

namespace Debug
{
    void begin();
    void log(const char *message);
    void logIpAddress(const IPAddress &ip);
    void logWifiPowerStep(const char *label);
    void logHostname(const char *hostname);
    void logLocalTime(const tm &timeInfo);
    void logRawRejected(uint8_t value);
    void logRawWritten(uint8_t value);
    void logDisplayBuffer(const uint8_t *digits, uint8_t count);
}
