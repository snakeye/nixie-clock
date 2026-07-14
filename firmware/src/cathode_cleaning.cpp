#include "cathode_cleaning.h"

#include "app_state.h"
#include "clock_time.h"
#include "debug.h"

namespace CathodeCleaning
{
    constexpr unsigned long StepMs = 200;
    constexpr uint8_t CycleCount = 3;
    constexpr uint8_t DigitCount = 10;

    bool active = false;
    uint8_t digit = 0;
    uint8_t cycle = 0;
    unsigned long lastStepMs = 0;
    int lastRunDayKey = -1;

    bool shouldStartNow()
    {
        if (active)
        {
            return false;
        }

        if (!ClockTime::isTimeValid() || !ClockTime::hasNtpSync() || !hv.isEnabled())
        {
            return false;
        }

        if (ClockTime::hours() != 0 || ClockTime::minutes() != 0 || ClockTime::seconds() != 0)
        {
            return false;
        }

        return lastRunDayKey != ClockTime::dayKey();
    }

    void start()
    {
        active = true;
        digit = 0;
        cycle = 0;
        lastStepMs = 0;
        lastRunDayKey = ClockTime::dayKey();

        nixie.showUniformDigits(digit);
        Debug::log("[cleaning] cathode cleaning started");
    }

    void finish()
    {
        active = false;
        Debug::log("[cleaning] cathode cleaning finished");
    }

    void stop()
    {
        active = false;
        digit = 0;
        cycle = 0;
        lastStepMs = 0;
        lastRunDayKey = -1;
    }

    void update()
    {
        if (!active)
        {
            if (shouldStartNow())
            {
                start();
            }
            return;
        }

        const unsigned long nowMs = millis();
        if (lastStepMs != 0 && nowMs - lastStepMs < StepMs)
        {
            return;
        }
        lastStepMs = nowMs;

        nixie.showUniformDigits(digit);

        ++digit;
        if (digit < DigitCount)
        {
            return;
        }

        digit = 0;
        ++cycle;
        if (cycle >= CycleCount)
        {
            finish();
        }
    }

    void requestManualStart()
    {
        if (active || !ClockTime::isTimeValid() || !hv.isEnabled())
        {
            return;
        }

        start();
    }

    bool isActive()
    {
        return active;
    }
}
