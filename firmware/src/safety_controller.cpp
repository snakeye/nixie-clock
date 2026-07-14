#include "safety_controller.h"

#include "debug.h"
#include "high_voltage_power.h"
#include "nixie_display.h"

NixieSafetyController::NixieSafetyController(NixieDisplay &display, HighVoltagePower &power)
    : display(display), power(power)
{
}

void NixieSafetyController::beginSafe()
{
    display.beginSafe();
    power.beginSafe();
    Debug::log("[safety] safe startup state applied");
}

void NixieSafetyController::enableHighVoltageSafely()
{
    Debug::log("[safety] enabling HV safely");
    display.disableDisplay();
    display.blank();
    power.enableRaw();
}

void NixieSafetyController::disableHighVoltageSafely()
{
    Debug::log("[safety] disabling HV safely");
    display.disableDisplay();
    display.blank();
    power.disable();
}

void NixieSafetyController::blankDisplaySafely()
{
    Debug::log("[safety] blank display");
    display.disableDisplay();
    display.blank();
}
