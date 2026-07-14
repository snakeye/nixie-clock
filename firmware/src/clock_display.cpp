#include "clock_display.h"

#include "app_state.h"
#include "clock_time.h"

namespace ClockDisplay
{
    bool enabled = true;
    bool timeRendered = false;
    int lastRenderedMinute = -1;

    void enable()
    {
        enabled = true;
        lastRenderedMinute = -1;
    }

    void disable()
    {
        enabled = false;
        nixie.stopMultiplexing();
        nixie.blank();
        lastRenderedMinute = -1;
        timeRendered = false;
    }

    void suspend()
    {
        enabled = false;
        lastRenderedMinute = -1;
        timeRendered = false;
    }

    void update()
    {
        if (!enabled || !ClockTime::isTimeValid())
        {
            if (timeRendered)
            {
                nixie.blank();
                lastRenderedMinute = -1;
                timeRendered = false;
            }
            return;
        }

        const int minuteKey = ClockTime::hours() * 60 + ClockTime::minutes();
        if (lastRenderedMinute == minuteKey)
        {
            return;
        }

        lastRenderedMinute = minuteKey;
        nixie.showTime(ClockTime::hours(), ClockTime::minutes(), ClockTime::seconds());
        timeRendered = true;
    }
}
