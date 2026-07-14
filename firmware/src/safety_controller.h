#pragma once

class HighVoltagePower;
class NixieDisplay;

class NixieSafetyController
{
public:
    NixieSafetyController(NixieDisplay &display, HighVoltagePower &power);

    void beginSafe();
    void enableHighVoltageSafely();
    void disableHighVoltageSafely();
    void blankDisplaySafely();

private:
    NixieDisplay &display;
    HighVoltagePower &power;
};
