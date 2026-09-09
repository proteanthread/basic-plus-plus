// FILENAME: var_magic.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for var_magic within BASIC++.

#include "runtime/variables_internal.h"

//
// ---- Magic & Virtual Variable Detection ----
//

bool is_magic_virtual_var(const char *name) {
    if (!name || !*name) return false;

    // Platform Hardware & Telemetry Hooks
    if (name[0] == '_') {
        if (basic_strcasecmp(name, "_CLIPBOARD$") == 0 ||
            basic_strcasecmp(name, "_HMOUSE") == 0 ||
            basic_strcasecmp(name, "_VMOUSE") == 0 ||
            basic_strcasecmp(name, "_MOUSE") == 0 ||
            basic_strcasecmp(name, "_MOUSE$") == 0 ||
            basic_strcasecmp(name, "_TRIG") == 0 ||
            basic_strcasecmp(name, "_ST") == 0 ||
            basic_strcasecmp(name, "_BATTERY%") == 0 ||
            basic_strcasecmp(name, "_BATTERY") == 0 ||
            basic_strcasecmp(name, "_TEMPERATURE%") == 0 ||
            basic_strcasecmp(name, "_TEMPERATURE") == 0 ||
            basic_strcasecmp(name, "_CPU_LOAD%") == 0 ||
            basic_strcasecmp(name, "_CPU_LOAD") == 0 ||
            basic_strcasecmp(name, "_WIFI_RSSI%") == 0 ||
            basic_strcasecmp(name, "_WIFI_RSSI") == 0 ||
            basic_strcasecmp(name, "_FREE_STACK") == 0) {
            return true;
        }
        return false;
    }

    // Mouse & Input Virtual Variables
    if ((name[0] == 'M' || name[0] == 'm') &&
        (basic_strcasecmp(name, "MOUSE") == 0 || basic_strcasecmp(name, "MOUSE$") == 0 ||
         basic_strcasecmp(name, "MEM") == 0)) return true;
    if ((name[0] == 'H' || name[0] == 'h') && basic_strcasecmp(name, "HMOUSE") == 0) return true;
    if ((name[0] == 'V' || name[0] == 'v') && basic_strcasecmp(name, "VMOUSE") == 0) return true;
    if ((name[0] == 'T' || name[0] == 't') && basic_strcasecmp(name, "TRIG") == 0) return true;

    // Retro Status & Reset Vectors
    if ((name[0] == 'S' || name[0] == 's') &&
        (basic_strcasecmp(name, "ST") == 0 || basic_strcasecmp(name, "SOFTEV") == 0)) return true;
    if ((name[0] == 'D' || name[0] == 'd') &&
        (basic_strcasecmp(name, "DSTATS") == 0 || basic_strcasecmp(name, "DSTAT") == 0)) return true;
    if ((name[0] == 'P' || name[0] == 'p') && basic_strcasecmp(name, "PWRED") == 0) return true;
    if ((name[0] == 'R' || name[0] == 'r') && basic_strcasecmp(name, "RESET_VECTOR") == 0) return true;

    return false;
}
