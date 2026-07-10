/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: help.h
 * =====================================================================
 */

#ifndef BASICPP_HELP_H
#define BASICPP_HELP_H

#include "runtime.h"
#include <stdbool.h>

typedef enum HelpCatId {
    HCAT_FLOW = 0,
    HCAT_IO,
    HCAT_MATH,
    HCAT_STRING,
    HCAT_VARMEM,
    HCAT_FILEIO,
    HCAT_GFX,
    HCAT_SOUND,
    HCAT_SYSENV,
    HCAT_DEBUG,
    HCAT_PROGMGMT,
    HCAT_EDIT,
    HCAT_DEVICE,
    HCAT_OPER,
    HCAT_INTRO,
    HCAT_SYSVAR,
    HCAT_COUNT
} HelpCatId;

typedef struct HelpEntry {
    const char *keyword;
    const char *summary;
    const char *usage;
    HelpCatId   category;
} HelpEntry;

void help_show(const char *topic);
void help_info(RuntimeState *rt);

#ifndef BPP_LITE_BUILD
void help_catalog(void);
#endif

bool help_has_keyword(const char *keyword);

#endif // BASICPP_HELP_H
