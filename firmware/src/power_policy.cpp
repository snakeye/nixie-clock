#include "power_policy.h"

#include "app_state.h"
#include "clock_time.h"
#include "ota_maintenance.h"

void updateHighVoltagePolicy()
{
    if (OtaMaintenance::isUploadActive())
    {
        if (hv.isEnabled())
        {
            safety.disableHighVoltageSafely();
        }
        return;
    }

    if (ClockTime::isTimeValid())
    {
        if (!hv.isEnabled())
        {
            safety.enableHighVoltageSafely();
        }
        return;
    }

    if (hv.isEnabled())
    {
        safety.disableHighVoltageSafely();
    }
}
