#include "status_led.h"

namespace StatusLed
{
    constexpr unsigned long BootStepMs = 250;
    constexpr unsigned long SyncPulseMs = 200;
    constexpr unsigned long WaitingStepMs = 500;

    bool bootWinkActive = false;
    uint8_t bootWinkStep = 0;
    unsigned long lastBootStepMs = 0;

    bool syncPulseActive = false;
    unsigned long syncPulseStartedMs = 0;

    bool waitingForTimeActive = false;
    bool waitingForTimeOn = false;
    unsigned long lastWaitingStepMs = 0;

    void write(uint8_t r, uint8_t g, uint8_t b)
    {
        // Built into Arduino-ESP32. WS2812B expects GRB internally,
        // but this helper accepts normal RGB values.
        neopixelWrite(Pins::StatusLed, r, g, b);
    }

    void cancelBootWink()
    {
        bootWinkActive = false;
    }

    void cancelSyncPulse()
    {
        syncPulseActive = false;
    }

    void cancelWaitingForTime()
    {
        waitingForTimeActive = false;
        waitingForTimeOn = false;
        lastWaitingStepMs = 0;
    }

    void set(uint8_t r, uint8_t g, uint8_t b)
    {
        cancelBootWink();
        cancelSyncPulse();
        cancelWaitingForTime();
        write(r, g, b);
    }

    void off()
    {
        set(0, 0, 0);
    }

    void startBootWink()
    {
        bootWinkActive = true;
        bootWinkStep = 0;
        lastBootStepMs = 0;
    }

    void setWaitingForTime(bool active)
    {
        if (!active)
        {
            waitingForTimeActive = false;
            waitingForTimeOn = false;
            lastWaitingStepMs = 0;
            return;
        }

        if (!waitingForTimeActive)
        {
            waitingForTimeActive = true;
            waitingForTimeOn = false;
            lastWaitingStepMs = 0;
        }
    }

    void blinkSyncSuccess()
    {
        cancelBootWink();
        cancelWaitingForTime();
        syncPulseActive = true;
        syncPulseStartedMs = millis();
        write(0, BRIGHTNESS, 0);
    }

    void update()
    {
        if (syncPulseActive)
        {
            const unsigned long nowMs = millis();
            if (nowMs - syncPulseStartedMs >= SyncPulseMs)
            {
                write(0, 0, 0);
                syncPulseActive = false;
            }
            return;
        }

        if (!bootWinkActive)
        {
            if (!waitingForTimeActive)
            {
                return;
            }

            const unsigned long nowMs = millis();
            if (lastWaitingStepMs != 0 && nowMs - lastWaitingStepMs < WaitingStepMs)
            {
                return;
            }
            lastWaitingStepMs = nowMs;

            waitingForTimeOn = !waitingForTimeOn;

            if (waitingForTimeOn)
            {
                write(BRIGHTNESS, BRIGHTNESS / 4, 0);
            }
            else
            {
                write(0, 0, 0);
            }

            return;
        }

        const unsigned long nowMs = millis();
        if (lastBootStepMs != 0 && nowMs - lastBootStepMs < BootStepMs)
        {
            return;
        }
        lastBootStepMs = nowMs;

        switch (bootWinkStep)
        {
        case 0:
            write(BRIGHTNESS, 0, 0);
            break;
        case 1:
            write(0, BRIGHTNESS, 0);
            break;
        case 2:
            write(0, 0, BRIGHTNESS);
            break;
        default:
            write(0, 0, 0);
            bootWinkActive = false;
            break;
        }

        ++bootWinkStep;
    }
}
