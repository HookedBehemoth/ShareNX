#pragma once
#include "../translation.hpp"

constexpr static const char *const english[Total_Strings] = {
    [OK] = "Ok",
    [BACK] = "Back",
    [CANCEL] = "Cancel",
    [FINISHED] = "Finished",
    [EXIT] = "Exit",
    [ERROR] = "Error",
    [ACCESSOR_INIT] = "Couldn't reach album accessor!",
    [TOKEN_MISSING] = "Authentication token missing from callback URL!",
    [WEB_APPLET_MODE] = "Can't use web browser in applet mode!",
    [WEB_EXIT_REASON] = "Canceled!",
    [CONFIG_SAVED_FMT] = "Configuration was saved for %s.",
};
