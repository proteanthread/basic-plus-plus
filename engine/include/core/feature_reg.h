/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file feature_reg.h
 * @brief Self-Registering Feature Hooks & Dynamic Introspection Registry API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Provides self-registration macros and API routines allowing
 *   modular C17 micro-libraries to register statement keywords, built-in functions,
 *   HELP text topics, and CATALOG entries at initialization.
 * - Why it exists: Eliminates hardcoded central keyword/help tables, enabling
 *   ultra-granular builds (from resource-constrained headless IoT nodes to desktop engines)
 *   where unlinked micro-libraries leave zero footprint and zero phantom HELP entries.
 * - Why it works this way: Micro-libraries invoke self-registration macros during module
 *   initialization. The HELP statement and CATALOG query the registry dynamically.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Capacity limits, registration callback signatures, query sorting.
 * - What cannot be changed: Dynamic self-registration interface contracts.
 * - What to expect: Unlinked micro-libraries automatically omit their keywords from CATALOG/HELP.
 * - What to do if something breaks: If a keyword is missing from HELP, ensure its micro-library
 *   initializer calls feature_register_keyword() and feature_register_help().
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Feature registration occurs before script parsing starts.
 * - Portability concerns: Strict C17 compliant, no platform-specific extensions.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add metadata fields to FeatureHelpEntry.
 * - How to write external extensions: External plugins call feature_register_* during activation.
 */

#ifndef CORE_FEATURE_REG_H
#define CORE_FEATURE_REG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_FEATURE_KEYWORDS 512
#define MAX_FEATURE_BUILTINS 512
#define MAX_FEATURE_HELP_ENTRIES 512

/* Feature Metadata Descriptor for Statement Keywords */
typedef struct {
    char name[32];
    int token_id;
    const char *category;
} FeatureKeywordEntry;

/* Feature Metadata Descriptor for Builtin Functions */
typedef struct {
    char name[32];
    const char *help_text;
    const char *category;
} FeatureBuiltinEntry;

/* Feature Metadata Descriptor for HELP topics */
typedef struct {
    char topic[32];
    const char *summary;
    const char *usage;
    const char *category;
} FeatureHelpEntry;

/* Public Self-Registration API */
void feature_reg_init(void);
bool feature_register_keyword(const char *keyword, int token_id, const char *category);
bool feature_register_builtin(const char *name, const char *help_text, const char *category);
bool feature_register_help(const char *topic, const char *summary, const char *usage, const char *category);

/* Dynamic Introspection Queries */
size_t feature_get_keyword_count(void);
const FeatureKeywordEntry* feature_get_keyword_by_index(size_t index);
const FeatureKeywordEntry* feature_find_keyword(const char *name);

size_t feature_get_builtin_count(void);
const FeatureBuiltinEntry* feature_get_builtin_by_index(size_t index);
const FeatureBuiltinEntry* feature_find_builtin(const char *name);

size_t feature_get_help_count(void);
const FeatureHelpEntry* feature_get_help_by_index(size_t index);
const FeatureHelpEntry* feature_find_help(const char *topic);

#endif /* CORE_FEATURE_REG_H */
