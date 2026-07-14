#include "clock_time.h"

#include <time.h>

#include <WiFi.h>

#include "app_shared.h"
#include "config.h"
#include "debug.h"

namespace ClockTime
{
    constexpr const char *Hostname = "nixie-clock";
    constexpr const char *Timezone = "CET-1CEST,M3.5.0/2,M10.5.0/3";
    constexpr const char *NtpServer1 = "pool.ntp.org";
    constexpr const char *NtpServer2 = "time.nist.gov";
    constexpr unsigned long UpdateIntervalMs = 1000;
    constexpr unsigned long WifiPowerStepIntervalMs = 10000;
    constexpr unsigned long NtpSettleMs = 5000;
    constexpr unsigned long NtpSyncTimeoutMs = 30000;
    constexpr unsigned long SyncRetryIntervalMs = 30000;
    constexpr unsigned long ResyncIntervalMs = 6UL * 60UL * 60UL * 1000UL;

    struct WifiPowerStep
    {
        wifi_power_t power;
        const char *label;
    };

    constexpr WifiPowerStep WifiPowerSteps[] = {
        {WIFI_POWER_2dBm, "2 dBm"},
        {WIFI_POWER_5dBm, "5 dBm"},
        {WIFI_POWER_8_5dBm, "8.5 dBm"},
        {WIFI_POWER_11dBm, "11 dBm"},
        {WIFI_POWER_15dBm, "15 dBm"},
    };

    constexpr uint8_t WifiPowerStepCount = sizeof(WifiPowerSteps) / sizeof(WifiPowerSteps[0]);

    bool wifiStarted = false;
    bool ntpConfigured = false;
    bool syncInProgress = false;
    bool timeValid = false;
    bool justSynced = false;
    tm currentTime = {};
    unsigned long lastUpdateMs = 0;
    unsigned long wifiAttemptStartedMs = 0;
    unsigned long lastSyncAttemptMs = 0;
    unsigned long lastSuccessfulSyncMs = 0;
    uint8_t wifiPowerStepIndex = 0;
    bool wifiConnectedLogged = false;

    bool hasWifiCredentials()
    {
        return WifiConfig::Ssid[0] != '\0';
    }

    void applyWifiPowerStep()
    {
        const WifiPowerStep &step = WifiPowerSteps[wifiPowerStepIndex];
        WiFi.setTxPower(step.power);
        Debug::logWifiPowerStep(step.label);
    }

    void resetWifiPowerSteps()
    {
        wifiPowerStepIndex = 0;
        wifiAttemptStartedMs = millis();
        applyWifiPowerStep();
    }

    void updateWifiPower(unsigned long nowMs)
    {
        if (!wifiStarted || WiFi.status() == WL_CONNECTED)
        {
            return;
        }

        if (wifiPowerStepIndex + 1 >= WifiPowerStepCount)
        {
            return;
        }

        if (nowMs - wifiAttemptStartedMs < WifiPowerStepIntervalMs)
        {
            return;
        }

        ++wifiPowerStepIndex;
        wifiAttemptStartedMs = nowMs;
        applyWifiPowerStep();
    }

    void shutdownWifiImpl()
    {
        if (!wifiStarted)
        {
            return;
        }

        WiFi.disconnect(true, false);
        WiFi.mode(WIFI_OFF);

        wifiStarted = false;
        ntpConfigured = false;
        syncInProgress = false;
        wifiConnectedLogged = false;
        wifiPowerStepIndex = 0;

        Debug::log("[wifi] off");
    }

    void beginWifi()
    {
        if (!hasWifiCredentials())
        {
            Debug::log("[wifi] skipped: no credentials");
            return;
        }

        WiFi.setHostname(Hostname);
        Debug::logHostname(Hostname);
        WiFi.mode(WIFI_STA);
        WiFi.persistent(false);
        WiFi.setAutoReconnect(true);
        WiFi.begin(WifiConfig::Ssid, WifiConfig::Password);
        wifiStarted = true;
        ntpConfigured = false;
        syncInProgress = false;
        wifiConnectedLogged = false;
        resetWifiPowerSteps();
        Debug::log("[wifi] connecting");
    }

    void beginNtp()
    {
        configTzTime(Timezone, NtpServer1, NtpServer2);
        ntpConfigured = true;
        Debug::log("[ntp] configured");
    }

    bool shouldTrySync(unsigned long nowMs)
    {
        if (!timeValid)
        {
            return lastSyncAttemptMs == 0 || nowMs - lastSyncAttemptMs >= SyncRetryIntervalMs;
        }

        return lastSuccessfulSyncMs == 0 || nowMs - lastSuccessfulSyncMs >= ResyncIntervalMs;
    }

    const char *syncReasonLogMessage()
    {
        return lastSuccessfulSyncMs == 0 ? "[ntp] sync requested" : "[ntp] scheduled resync";
    }

    void requestSync(unsigned long nowMs)
    {
        lastSyncAttemptMs = nowMs;
        syncInProgress = true;
        beginNtp();
    }

    void ensureWifiForSync()
    {
        if (wifiStarted || !hasWifiCredentials())
        {
            return;
        }

        beginWifi();
    }

    bool readSystemTimeFast()
    {
        time_t now = 0;
        time(&now);
        localtime_r(&now, &currentTime);

        return currentTime.tm_year > (2016 - 1900);
    }

    void updateImpl()
    {
        const unsigned long nowMs = millis();
        if (nowMs - lastUpdateMs < UpdateIntervalMs)
        {
            return;
        }
        lastUpdateMs = nowMs;

        if (syncInProgress && nowMs - lastSyncAttemptMs >= NtpSyncTimeoutMs)
        {
            syncInProgress = false;
            Debug::log("[ntp] sync timed out");
        }

        const bool wasTimeValid = timeValid;
        timeValid = readSystemTimeFast();

        if (shouldTrySync(nowMs))
        {
            ensureWifiForSync();
        }

        updateWifiPower(nowMs);

        if (!wifiStarted || WiFi.status() != WL_CONNECTED)
        {
            return;
        }

        if (!wifiConnectedLogged)
        {
            Debug::log("[wifi] connected");
            Debug::logIpAddress(WiFi.localIP());
            wifiConnectedLogged = true;
        }

        if (!syncInProgress && shouldTrySync(nowMs))
        {
            Debug::log(syncReasonLogMessage());
            requestSync(nowMs);
        }

        timeValid = readSystemTimeFast();

        if (!syncInProgress || !timeValid)
        {
            return;
        }

        const bool firstSync = !wasTimeValid;
        const bool ntpHadTimeToSettle = nowMs - lastSyncAttemptMs >= NtpSettleMs;

        if (!firstSync && !ntpHadTimeToSettle)
        {
            return;
        }

        lastSuccessfulSyncMs = nowMs;
        justSynced = true;
        syncInProgress = false;

        Debug::log("[ntp] time valid");
        Debug::logLocalTime(currentTime);
    }
}

void ClockTime::begin()
{
    beginWifi();
}

void ClockTime::update()
{
    updateImpl();
}

void ClockTime::shutdownWifi()
{
    shutdownWifiImpl();
}

bool ClockTime::isTimeValid()
{
    return timeValid;
}

bool ClockTime::hasNtpSync()
{
    return lastSuccessfulSyncMs != 0;
}

int ClockTime::dayKey()
{
    return currentTime.tm_year * 400 + currentTime.tm_yday;
}

bool ClockTime::consumeJustSynced()
{
    if (!justSynced)
    {
        return false;
    }

    justSynced = false;
    return true;
}

uint8_t ClockTime::hours()
{
    return static_cast<uint8_t>(currentTime.tm_hour);
}

uint8_t ClockTime::minutes()
{
    return static_cast<uint8_t>(currentTime.tm_min);
}

uint8_t ClockTime::seconds()
{
    return static_cast<uint8_t>(currentTime.tm_sec);
}
