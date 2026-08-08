/* =====================================================================
 * What it does: Header file for the Micro-Library Embedded Metadata Registry.
 * Why it exists: Provides dynamic introspection (HELP, CATALOG, Error Codes) for all 1-to-1 micro-libraries.
 * Why it works this way: Stores embedded metadata registered by each micro-library at system boot.
 * What can be changed: Maximum metadata entries count.
 * What cannot be changed: MicroLibMetadata structure field layout, C17 standard compliance.
 * What to expect: Fast O(N) or hashed lookup of keyword metadata.
 * What to do if something breaks: Check initial call to microlib_init.
 * Assumptions: Metadata strings are static string literals embedded in source files.
 * Portability concerns: Portable C17 standard.
 * Future expansions: Dynamic filtering by category or tag.
 * ===================================================================== */

#ifndef RUNTIME_MICRO_LIB_METADATA_H
#define RUNTIME_MICRO_LIB_METADATA_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_MICROLIB_ENTRIES 256

typedef struct {
    const char *name;         /* Keyword or Function Name (e.g. "RND", "FOR") */
    const char *category;     /* Catalog Category (e.g. "Math Functions", "Looping / Control Flow") */
    const char *syntax;       /* Syntax Signature (e.g. "RND[(x)]", "FOR var = start TO end [STEP step]") */
    const char *help_text;    /* Introspection Help Text for HELP command */
    const char *error_codes;  /* Generated Error Codes (e.g. "Error 2: Syntax Error, Error 13: Type Mismatch") */
} MicroLibMetadata;

void microlib_init(void);
int microlib_register(const MicroLibMetadata *meta);
const MicroLibMetadata *microlib_find(const char *name);
int microlib_count(void);
const MicroLibMetadata *microlib_get(int index);

#endif /* RUNTIME_MICRO_LIB_METADATA_H */
