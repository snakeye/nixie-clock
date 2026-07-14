#include <OneButton.h>

#include "app_shared.h"
#include "app_state.h"
#include "button_actions.h"
#include "cathode_cleaning.h"
#include "clock_time.h"
#include "debug.h"
#include "ota_maintenance.h"
#include "power_policy.h"
#include "status_led.h"
#include "app_mode.h"

OneButton button1;
OneButton button2;

void setup()
{
    Debug::begin();

    // Preload safe output latch levels before switching GPIOs to OUTPUT.
    // This reduces startup glitches while ESP32 takes ownership of the pins.
    digitalWrite(Pins::ShiftRegisterOutputEnable, Levels::ShiftRegisterOutputDisabled);
    digitalWrite(Pins::HighVoltageEnable, Levels::HighVoltageDisabled);
    digitalWrite(Pins::ShiftRegisterLatch, LOW);
    digitalWrite(Pins::ShiftRegisterClock, LOW);
    digitalWrite(Pins::ShiftRegisterData, LOW);

    pinMode(Pins::StatusLed, OUTPUT);
    pinMode(Pins::ShiftRegisterOutputEnable, OUTPUT);
    pinMode(Pins::HighVoltageEnable, OUTPUT);
    pinMode(Pins::ShiftRegisterLatch, OUTPUT);
    pinMode(Pins::ShiftRegisterClock, OUTPUT);
    pinMode(Pins::ShiftRegisterData, OUTPUT);

    safety.beginSafe();

    button1.setup(Pins::Button1, INPUT_PULLUP, true);
    button2.setup(Pins::Button2, INPUT_PULLUP, true);

    button1.setDebounceMs(50);
    button2.setDebounceMs(50);
    button1.setPressMs(800);
    button2.setPressMs(800);

    button1.attachLongPressStart(onButton1LongPressStart);

    ClockTime::begin();

    StatusLed::startBootWink();
}

void loop()
{
    button1.tick();
    button2.tick();
    ClockTime::update();

    if (ClockTime::consumeJustSynced())
    {
        StatusLed::blinkSyncSuccess();
        OtaMaintenance::beginWindow();
    }

    OtaMaintenance::update();
    updateHighVoltagePolicy();
    CathodeCleaning::update();
    AppMode::update();
}
