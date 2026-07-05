/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: alias_lang_core.h
 * Subsystem: Natural Language Translation Alias Pack
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Provides translation alias pack registers and mappings for Spanish, French, Italian, German, Portuguese, and Japanese.
 *
 * 2. WHAT TO EXPECT:
 *    Maps localized aliases to core KeywordIds in constant time.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Aliases, language mappings, dictionary keys.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Core keyword index IDs.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If localized keywords fail to parse, verify dialect compatibility flags.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE ALIAS LANGUAGE CORE
 * File: alias_lang_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_ALIAS_LANG_CORE_H
#define BASICPP_STANDALONE_ALIAS_LANG_CORE_H

#include "lexer.h"

typedef struct {
    KeywordId kw;
    const char *alias;
} AliasLangPackEntry;

typedef struct {
    const char *code;
    const char *name;
    const AliasLangPackEntry *entries;
} AliasLangPack;

const AliasLangPack *alias_lang_core_find(const char *code);
const AliasLangPack *alias_lang_core_get_all(int *out_count);

#endif // BASICPP_STANDALONE_ALIAS_LANG_CORE_H
