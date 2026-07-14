#pragma once

#include "app_shared.h"

namespace StatusLed
{
    void set(uint8_t r, uint8_t g, uint8_t b);
    void off();
    void setWaitingForTime(bool active);
    void startBootWink();
    void blinkSyncSuccess();
    void update();
}
