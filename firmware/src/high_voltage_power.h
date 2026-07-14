#pragma once

#include "app_shared.h"

class NixieSafetyController;

class HighVoltagePower
{
public:
    void beginSafe();
    void disable();
    bool isEnabled() const;

private:
    bool enabled = false;

    void enableRaw();
    void disableRaw();

    friend class NixieSafetyController;
};
