/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: target.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Defines the cross-compilation target platforms registry. Enables target
 *    platform independence by separating compile-time host environments from
 *    the targeted execution platforms.
 *
 * 2. WHAT TO EXPECT:
 *    A static read-only database of target system properties (Windows, Linux,
 *    FreeDOS, Embedded) used during code generation.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Add new target platform specifications or modify memory limit parameters.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Registry interface signatures needed by codegen.
 *
 * 5. PORTABILITY & MEMORY SYSTEM:
 *    Employs standard C string checks. Uses zero runtime heap allocations.
 * ===================================================================== */

#include <string.h>
#include "target.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

static const TargetConfig g_targets[] = {
    {
        "windows",
        TARGET_WINDOWS,
        "BPP_TARGET_WINDOWS",
        true,   /* has_multitasking */
        true,   /* has_graphics */
        true,   /* has_filesystem */
        16777216L, /* default_string_limit: 16 MB */
        4194304L   /* default_array_limit: 4M elements */
    },
    {
        "linux",
        TARGET_LINUX,
        "BPP_TARGET_LINUX",
        true,   /* has_multitasking */
        true,   /* has_graphics */
        true,   /* has_filesystem */
        16777216L, /* default_string_limit: 16 MB */
        4194304L   /* default_array_limit: 4M elements */
    },
    {
        "freedos",
        TARGET_FREEDOS,
        "BPP_TARGET_FREEDOS",
        false,  /* has_multitasking */
        false,  /* has_graphics */
        true,   /* has_filesystem */
        65536L,    /* default_string_limit: 64 KB */
        65536L     /* default_array_limit: 64K elements */
    },
    {
        "embedded",
        TARGET_EMBEDDED,
        "BPP_TARGET_EMBEDDED",
        false,  /* has_multitasking */
        false,  /* has_graphics */
        false,  /* has_filesystem */
        8192L,     /* default_string_limit: 8 KB */
        1024L      /* default_array_limit: 1024 elements */
    }
};

static const int g_target_count = sizeof(g_targets) / sizeof(g_targets[0]);

/* =====================================================================
 * Function: target_find
 * ---------------------------------------------------------------------
 * WHAT IT DOES: 
 *   Searches the target registry database by name (case-insensitive).
 * ASSUMPTIONS:
 *   The name parameter is a null-terminated string.
 * ===================================================================== */
const TargetConfig* target_find(const char *name)
{
    int i;
    if (!name || name[0] == '\0') {
        return NULL;
    }
    for (i = 0; i < g_target_count; i++) {
        if (strcasecmp(g_targets[i].name, name) == 0) {
            return &g_targets[i];
        }
    }
    return NULL;
}

/* =====================================================================
 * Function: target_get_default
 * ---------------------------------------------------------------------
 * WHAT IT DOES: 
 *   Retrieves the default target matching the current host operating system.
 * ===================================================================== */
const TargetConfig* target_get_default(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return target_find("windows");
#else
    return target_find("linux");
#endif
}
