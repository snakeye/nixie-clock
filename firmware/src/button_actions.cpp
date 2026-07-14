#include "button_actions.h"

#include "cathode_cleaning.h"

void onButton1LongPressStart()
{
    CathodeCleaning::requestManualStart();
}
