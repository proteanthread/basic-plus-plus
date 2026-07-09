/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: help_core.h
 * Subsystem: Built-In Interactive Catalog Database
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Retrieves quick-reference summaries and lists syntax catalogs.
 *
 * 2. WHAT TO EXPECT:
 *    Locates topics and prints references.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Catalog categories, topic strings.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Topic search indexing algorithm.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If topic is missing, verify spelling.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE HELP CATALOG CORE
 * File: help_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_HELP_CORE_H
#define BASICPP_STANDALONE_HELP_CORE_H

typedef enum HelpCoreCatId {
    HCAT_CORE_FLOW = 0,
    HCAT_CORE_IO,
    HCAT_CORE_MATH,
    HCAT_CORE_STRING,
    HCAT_CORE_VARMEM,
    HCAT_CORE_FILEIO,
    HCAT_CORE_GFX,
    HCAT_CORE_SOUND,
    HCAT_CORE_SYSENV,
    HCAT_CORE_DEBUG,
    HCAT_CORE_PROGMGMT,
    HCAT_CORE_EDIT,
    HCAT_CORE_DEVICE,
    HCAT_CORE_OPER,
    HCAT_CORE_INTRO,
    HCAT_CORE_SYSVAR,
    HCAT_CORE_MATRIX,
    HCAT_CORE_ADV_MATH,
    HCAT_CORE_COUNT
} HelpCoreCatId;

typedef struct HelpCoreEntry {
    const char *keyword;
    const char *summary;
    const char *usage;
    int         category;
} HelpCoreEntry;

const HelpCoreEntry *help_core_get_db(int *size);
const char *help_core_get_category_name(int cat_id);

#endif // BASICPP_STANDALONE_HELP_CORE_H
