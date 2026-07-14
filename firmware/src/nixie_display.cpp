#include "nixie_display.h"

#include "debug.h"

void NixieDisplay::beginSafe()
{
    disableDisplay();
    blank();
    Debug::log("[nixie] display safe blank");
}

void NixieDisplay::enableDisplay()
{
    // 74HC595 OE is active-low.
    digitalWrite(Pins::ShiftRegisterOutputEnable, Levels::ShiftRegisterOutputEnabled);
}

void NixieDisplay::disableDisplay()
{
    // 74HC595 OE is active-low.
    digitalWrite(Pins::ShiftRegisterOutputEnable, Levels::ShiftRegisterOutputDisabled);
}

void NixieDisplay::blank()
{
    stopMultiplexing();
    writeRawChecked(AllAnodesDisabledMask);
}

void NixieDisplay::showTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    (void)seconds;

    if (hours > 23 || minutes > 59)
    {
        return;
    }

    displayDigits[0] = hours < 10 ? BlankDigit : hours / 10;
    displayDigits[1] = hours % 10;
    displayDigits[2] = minutes / 10;
    displayDigits[3] = minutes % 10;
    displayBufferValid = true;

    if (!multiplexEnabled)
    {
        startMultiplexing();
    }
}

void NixieDisplay::showUniformDigits(uint8_t digit)
{
    if (digit > 9)
    {
        return;
    }

    displayDigits[0] = digit;
    displayDigits[1] = digit;
    displayDigits[2] = digit;
    displayDigits[3] = digit;
    displayBufferValid = true;

    if (!multiplexEnabled)
    {
        startMultiplexing();
    }
}

bool NixieDisplay::isDisplayBufferValid() const
{
    return displayBufferValid;
}

void NixieDisplay::startMultiplexing()
{
    if (!displayBufferValid)
    {
        return;
    }

    multiplexEnabled = true;
    currentMultiplexTube = 0;
    lastMultiplexStepUs = 0;
}

void NixieDisplay::stopMultiplexing()
{
    multiplexEnabled = false;
}

bool NixieDisplay::isMultiplexing() const
{
    return multiplexEnabled;
}

void NixieDisplay::updateMultiplex()
{
    if (!multiplexEnabled || !displayBufferValid)
    {
        return;
    }

    const unsigned long nowUs = micros();
    if (lastMultiplexStepUs != 0 && nowUs - lastMultiplexStepUs < MultiplexSlotUs)
    {
        return;
    }
    lastMultiplexStepUs = nowUs;

    const uint8_t digit = displayDigits[currentMultiplexTube];

    // During blanking all anodes must be actively disabled,
    // but the cathode driver should already be set to the next digit.
    // If blanking uses BCD=0, any residual anode current shows ghost zeroes.
    writeRawChecked(makeBlankingValue(digit), false);
    enableDisplay();
    delayMicroseconds(MultiplexBlankingUs);

    if (writeRawChecked(makeValue(currentMultiplexTube, digit), false))
    {
        delayMicroseconds(MultiplexSettleUs);
        enableDisplay();
    }

    currentMultiplexTube = (currentMultiplexTube + 1) % TubeCount;
}

uint8_t NixieDisplay::anodeBit(uint8_t tubeIndex)
{
    switch (tubeIndex)
    {
    case 0:
        return AnodeBit0;
    case 1:
        return AnodeBit1;
    case 2:
        return AnodeBit2;
    case 3:
        return AnodeBit3;
    default:
        return AnodeBit0;
    }
}

uint8_t NixieDisplay::enabledAnodeCount(uint8_t value)
{
    uint8_t count = 0;

    for (uint8_t i = 0; i < TubeCount; ++i)
    {
        if ((value & (1U << anodeBit(i))) == Levels::AnodeEnabled)
        {
            ++count;
        }
    }

    return count;
}

uint8_t NixieDisplay::digitFromRawValue(uint8_t value)
{
    return (value & DigitMask) >> DigitBit0;
}

bool NixieDisplay::isSafeRawValue(uint8_t value)
{
    const uint8_t activeAnodes = enabledAnodeCount(value);
    const uint8_t digit = digitFromRawValue(value);

    // Safe states:
    // - no active anodes: display blank, any BCD bits are harmless
    // - exactly one active anode with a valid BCD digit 0..9
    if (activeAnodes == 0)
    {
        return true;
    }

    return activeAnodes == 1 && digit <= 9;
}

uint8_t NixieDisplay::makeValue(uint8_t tubeIndex, uint8_t digit)
{
    if (tubeIndex >= TubeCount || digit == BlankDigit || digit > 9)
    {
        return AllAnodesDisabledMask;
    }

    uint8_t value = AllAnodesDisabledMask;

    // Enable exactly one anode by pulling the optocoupler LED cathode low.
    value &= ~(1U << anodeBit(tubeIndex));

    // Put BCD digit on 155ИД1 inputs A..D, connected to 74HC595 QE..QH.
    value |= (digit & 0x0F) << DigitBit0;

    return value;
}

uint8_t NixieDisplay::makeBlankingValue(uint8_t digit)
{
    if (digit == BlankDigit || digit > 9)
    {
        return AllAnodesDisabledMask;
    }

    // Keep all anodes actively disabled while preloading the next cathode digit.
    // This avoids showing BCD=0 during blanking when the previous anode path is still decaying.
    return AllAnodesDisabledMask | ((digit & 0x0F) << DigitBit0);
}

void NixieDisplay::shiftOutRawUnsafe(uint8_t value, bool logWrite)
{
    currentRawValue = value;
    if (logWrite)
    {
        Debug::logRawWritten(value);
    }

    digitalWrite(Pins::ShiftRegisterLatch, LOW);
    digitalWrite(Pins::ShiftRegisterClock, LOW);

    for (int8_t bit = 7; bit >= 0; --bit)
    {
        digitalWrite(Pins::ShiftRegisterData, (value & (1U << bit)) ? HIGH : LOW);
        digitalWrite(Pins::ShiftRegisterClock, HIGH);
        digitalWrite(Pins::ShiftRegisterClock, LOW);
    }

    digitalWrite(Pins::ShiftRegisterLatch, HIGH);
    digitalWrite(Pins::ShiftRegisterLatch, LOW);
}

bool NixieDisplay::writeRawChecked(uint8_t value, bool logWrite)
{
    if (!isSafeRawValue(value))
    {
        Debug::logRawRejected(value);
        disableDisplay();
        shiftOutRawUnsafe(AllAnodesDisabledMask, logWrite);
        return false;
    }

    shiftOutRawUnsafe(value, logWrite);
    return true;
}
