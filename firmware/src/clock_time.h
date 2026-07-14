#pragma once

#include "app_shared.h"

namespace ClockTime
{
    void begin();
    void update();
    void shutdownWifi();
    bool isTimeValid();
    bool hasNtpSync();
    int dayKey();
    bool consumeJustSynced();
    uint8_t hours();
    uint8_t minutes();
    uint8_t seconds();
}
