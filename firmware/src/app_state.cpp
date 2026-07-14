#include "app_state.h"

HighVoltagePower hv;
NixieDisplay nixie;
NixieSafetyController safety(nixie, hv);
