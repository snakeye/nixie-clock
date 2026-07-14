#include "debug.h"

#include <time.h>

namespace Debug
{
#if defined(DEBUG_LOG)
    constexpr bool Enabled = true;
#else
    constexpr bool Enabled = false;
#endif
    constexpr unsigned long BaudRate = 115200;

    void begin()
    {
        if (!Enabled)
        {
            return;
        }

        Serial.begin(BaudRate);
        Serial.println();
        Serial.println("[boot] nixie clock firmware starting");
    }

    void log(const char *message)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.println(message);
    }

    void logIpAddress(const IPAddress &ip)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.print("[wifi] ip: ");
        Serial.println(ip);
    }

    void logWifiPowerStep(const char *label)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.print("[wifi] tx power: ");
        Serial.println(label);
    }

    void logHostname(const char *hostname)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.print("[wifi] hostname: ");
        Serial.println(hostname);
    }

    void logLocalTime(const tm &timeInfo)
    {
        if (!Enabled)
        {
            return;
        }

        char buffer[32] = {};
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeInfo);

        Serial.print("[ntp] local time: ");
        Serial.println(buffer);
    }

    void logRawRejected(uint8_t value)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.print("[nixie] unsafe raw value rejected: 0x");
        if (value < 0x10)
        {
            Serial.print('0');
        }
        Serial.println(value, HEX);
    }

    void logRawWritten(uint8_t value)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.print("[nixie] raw written: 0b");
        for (int8_t bit = 7; bit >= 0; --bit)
        {
            Serial.print((value & (1U << bit)) ? '1' : '0');
        }

        Serial.print(" 0x");
        if (value < 0x10)
        {
            Serial.print('0');
        }
        Serial.println(value, HEX);
    }

    void logDisplayBuffer(const uint8_t *digits, uint8_t count)
    {
        if (!Enabled)
        {
            return;
        }

        Serial.print("[display] buffer: ");
        for (uint8_t i = 0; i < count; ++i)
        {
            if (digits[i] == DisplayBlankDigit)
            {
                Serial.print(' ');
            }
            else
            {
                Serial.print(digits[i]);
            }
            if (i == 1)
            {
                Serial.print(':');
            }
        }
        Serial.println();
    }
}
