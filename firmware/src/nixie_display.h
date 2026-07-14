#pragma once

#include "app_shared.h"

class NixieDisplay
{
public:
    void beginSafe();
    void blank();
    void showTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
    void showUniformDigits(uint8_t digit);
    bool isDisplayBufferValid() const;
    void startMultiplexing();
    void stopMultiplexing();
    bool isMultiplexing() const;
    void updateMultiplex();
    void enableDisplay();
    void disableDisplay();

private:
    static constexpr uint8_t TubeCount = 4;
    static constexpr uint8_t BlankDigit = DisplayBlankDigit;
    static constexpr unsigned long MultiplexSlotUs = 4000;
    static constexpr unsigned long MultiplexBlankingUs = 200;
    static constexpr unsigned long MultiplexSettleUs = 10;

    // 74HC595 bit layout, matching outputs QA..QH.
    // QA..QD: active-low anode optocoupler LEDs, left to right.
    static constexpr uint8_t AnodeBit0 = 0;
    static constexpr uint8_t AnodeBit1 = 1;
    static constexpr uint8_t AnodeBit2 = 2;
    static constexpr uint8_t AnodeBit3 = 3;

    // QE..QH: BCD input for 155ИД1 / K155ID1 digit decoder A..D.
    static constexpr uint8_t DigitBit0 = 4;
    static constexpr uint8_t DigitBit1 = 5;
    static constexpr uint8_t DigitBit2 = 6;
    static constexpr uint8_t DigitBit3 = 7;

    static constexpr uint8_t AllAnodesDisabledMask =
        (Levels::AnodeDisabled << AnodeBit0) |
        (Levels::AnodeDisabled << AnodeBit1) |
        (Levels::AnodeDisabled << AnodeBit2) |
        (Levels::AnodeDisabled << AnodeBit3);

    static constexpr uint8_t DigitMask =
        (1U << DigitBit0) |
        (1U << DigitBit1) |
        (1U << DigitBit2) |
        (1U << DigitBit3);

    uint8_t currentRawValue = AllAnodesDisabledMask;
    uint8_t displayDigits[TubeCount] = {0, 0, 0, 0};
    bool displayBufferValid = false;
    bool multiplexEnabled = false;
    uint8_t currentMultiplexTube = 0;
    unsigned long lastMultiplexStepUs = 0;

    static uint8_t anodeBit(uint8_t tubeIndex);
    static uint8_t enabledAnodeCount(uint8_t value);
    static uint8_t digitFromRawValue(uint8_t value);
    static bool isSafeRawValue(uint8_t value);
    static uint8_t makeValue(uint8_t tubeIndex, uint8_t digit);
    static uint8_t makeBlankingValue(uint8_t digit);
    void shiftOutRawUnsafe(uint8_t value, bool logWrite = true);
    bool writeRawChecked(uint8_t value, bool logWrite = true);
};
