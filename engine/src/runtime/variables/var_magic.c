// FILENAME: var_magic.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (variables_internal.h)
// Provides core logic and interface definitions for var_magic within BASIC++.
//
// ---- Includes ----

#include "runtime/variables_internal.h"

//
// ---- Magic Variable Detection ----

bool is_magic_virtual_var(const char *name) {
    if (!name || !*name) return false;
    if (name[0] == '_') return true;
    if ((name[0] == 'M' || name[0] == 'm') && (basic_strcasecmp(name, "MOUSE") == 0 || basic_strcasecmp(name, "MOUSE$") == 0)) return true;
    if ((name[0] == 'H' || name[0] == 'h') && basic_strcasecmp(name, "HMOUSE") == 0) return true;
    if ((name[0] == 'V' || name[0] == 'v') && basic_strcasecmp(name, "VMOUSE") == 0) return true;
    if ((name[0] == 'T' || name[0] == 't') && basic_strcasecmp(name, "TRIG") == 0) return true;
    return false;
}
