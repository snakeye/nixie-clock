#include "ota_maintenance.h"

#include <ArduinoOTA.h>

#include "app_state.h"
#include "cathode_cleaning.h"
#include "clock_display.h"
#include "clock_time.h"
#include "debug.h"
#include "status_led.h"

namespace OtaMaintenance
{
    constexpr unsigned long AutoWindowMs = 60UL * 1000UL;

    bool active = false;
    bool otaStarted = false;
    bool uploadActive = false;
    unsigned long windowStartedMs = 0;

    void beginWindow();
    void endWindow();

    void onOtaStart()
    {
        uploadActive = true;
        Debug::log("[ota] upload started");
        safety.disableHighVoltageSafely();
        StatusLed::set(0, BRIGHTNESS / 2, BRIGHTNESS);
    }

    void onOtaEnd()
    {
        uploadActive = false;
        Debug::log("[ota] upload finished");
    }

    void onOtaError(ota_error_t error)
    {
        uploadActive = false;
        Debug::log("[ota] upload error");
        (void)error;
    }

    void configure()
    {
        static bool configured = false;
        if (configured)
        {
            return;
        }

        configured = true;
        ArduinoOTA.setHostname("nixie-clock");
        ArduinoOTA.onStart(onOtaStart);
        ArduinoOTA.onEnd(onOtaEnd);
        ArduinoOTA.onError(onOtaError);
    }

    void beginWindow()
    {
        if (active || !ClockTime::isTimeValid() || !ClockTime::hasNtpSync())
        {
            return;
        }

        configure();
        active = true;
        otaStarted = false;
        uploadActive = false;
        windowStartedMs = millis();

        Debug::log("[ota] maintenance window opened");
        CathodeCleaning::stop();
        ClockDisplay::enable();
        ArduinoOTA.begin();
        otaStarted = true;
        StatusLed::set(0, BRIGHTNESS / 2, BRIGHTNESS);
    }

    void endWindow()
    {
        if (!active)
        {
            return;
        }

        Debug::log("[ota] maintenance window closed");
        active = false;
        otaStarted = false;
        uploadActive = false;

        ClockTime::shutdownWifi();
        StatusLed::off();
    }

    void update()
    {
        if (!active)
        {
            return;
        }

        if (otaStarted)
        {
            ArduinoOTA.handle();
        }

        StatusLed::set(0, BRIGHTNESS / 2, BRIGHTNESS);

        const unsigned long nowMs = millis();
        if (!uploadActive && nowMs - windowStartedMs >= AutoWindowMs)
        {
            endWindow();
        }
    }

    bool isWindowActive()
    {
        return active;
    }

    bool isUploadActive()
    {
        return uploadActive;
    }
}
