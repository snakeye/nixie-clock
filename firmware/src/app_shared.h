#pragma once

#include <Arduino.h>

namespace Pins
{
    constexpr uint8_t ShiftRegisterLatch = 0;
    constexpr uint8_t ShiftRegisterClock = 1;
    constexpr uint8_t ShiftRegisterData = 3;
    constexpr uint8_t ShiftRegisterOutputEnable = 21;

    constexpr uint8_t StatusLed = 4;
    constexpr uint8_t HighVoltageEnable = 7;

    constexpr uint8_t Button1 = 10;
    constexpr uint8_t Button2 = 20;
}

namespace Levels
{
    constexpr uint8_t ShiftRegisterOutputEnabled = LOW;
    constexpr uint8_t ShiftRegisterOutputDisabled = HIGH;

    constexpr uint8_t HighVoltageEnabled = HIGH;
    constexpr uint8_t HighVoltageDisabled = LOW;

    // Optocoupler LEDs have their anodes tied to VCC through resistors.
    // The shift register sinks current through their cathodes.
    constexpr uint8_t AnodeEnabled = LOW;
    constexpr uint8_t AnodeDisabled = HIGH;
}

constexpr uint8_t BRIGHTNESS = 24;
constexpr uint8_t DisplayBlankDigit = 0xFF;
