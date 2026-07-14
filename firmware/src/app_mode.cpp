#include "app_mode.h"

#include "app_state.h"
#include "cathode_cleaning.h"
#include "clock_display.h"
#include "clock_time.h"
#include "ota_maintenance.h"
#include "status_led.h"

namespace
{
    enum class Mode
    {
        WaitingForTime,
        Normal,
        Ota,
        Cleaning,
    };

    Mode currentMode()
    {
        if (OtaMaintenance::isUploadActive())
        {
            return Mode::Ota;
        }

        if (CathodeCleaning::isActive())
        {
            return Mode::Cleaning;
        }

        if (!ClockTime::isTimeValid())
        {
            return Mode::WaitingForTime;
        }

        return Mode::Normal;
    }

    void enterMode(Mode mode)
    {
        switch (mode)
        {
        case Mode::WaitingForTime:
            ClockDisplay::disable();
            StatusLed::setWaitingForTime(true);
            break;
        case Mode::Normal:
            ClockDisplay::enable();
            StatusLed::setWaitingForTime(false);
            break;
        case Mode::Ota:
        case Mode::Cleaning:
            ClockDisplay::suspend();
            StatusLed::setWaitingForTime(false);
            break;
        }
    }

    void exitMode(Mode)
    {
    }

    void updateMode(Mode mode)
    {
        switch (mode)
        {
        case Mode::WaitingForTime:
            StatusLed::update();
            break;
        case Mode::Normal:
            ClockDisplay::update();
            nixie.updateMultiplex();
            StatusLed::update();
            break;
        case Mode::Ota:
            StatusLed::update();
            break;
        case Mode::Cleaning:
            nixie.updateMultiplex();
            StatusLed::update();
            break;
        }
    }

    void updateStateMachine()
    {
        static bool initialized = false;
        static Mode lastMode = Mode::WaitingForTime;

        const Mode mode = currentMode();

        if (!initialized)
        {
            enterMode(mode);
            initialized = true;
            lastMode = mode;
        }
        else if (mode != lastMode)
        {
            exitMode(lastMode);
            enterMode(mode);
            lastMode = mode;
        }

        updateMode(mode);
    }
}

void AppMode::update()
{
    updateStateMachine();
}
