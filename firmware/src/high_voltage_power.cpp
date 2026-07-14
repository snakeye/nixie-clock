#include "high_voltage_power.h"

#include "debug.h"

void HighVoltagePower::beginSafe()
{
    disableRaw();
}

void HighVoltagePower::disable()
{
    disableRaw();
}

bool HighVoltagePower::isEnabled() const
{
    return enabled;
}

void HighVoltagePower::enableRaw()
{
    if (enabled)
    {
        return;
    }

    enabled = true;
    digitalWrite(Pins::HighVoltageEnable, Levels::HighVoltageEnabled);
    Debug::log("[hv] enabled");
}

void HighVoltagePower::disableRaw()
{
    if (!enabled)
    {
        digitalWrite(Pins::HighVoltageEnable, Levels::HighVoltageDisabled);
        return;
    }

    enabled = false;
    digitalWrite(Pins::HighVoltageEnable, Levels::HighVoltageDisabled);
    Debug::log("[hv] disabled");
}
