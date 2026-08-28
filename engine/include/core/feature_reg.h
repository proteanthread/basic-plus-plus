// FILENAME: feature_reg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (feature_reg.c)
// NEEDED BY: libengine (mux.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for feature_reg within BASIC++.
//
// ---- Includes ----

#ifndef CORE_FEATURE_REG_H
#define CORE_FEATURE_REG_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_FEATURE_KEYWORDS 512
#define MAX_FEATURE_BUILTINS 512
#define MAX_FEATURE_HELP_ENTRIES 512

// Feature Metadata Descriptor for Statement Keywords
typedef struct {
    char name[32];
    int token_id;
    const char *category;
} FeatureKeywordEntry;

// Feature Metadata Descriptor for Builtin Functions
typedef struct {
    char name[32];
    const char *help_text;
    const char *category;
} FeatureBuiltinEntry;

// Feature Metadata Descriptor for HELP topics
typedef struct {
    char topic[32];
    const char *summary;
    const char *usage;
    const char *category;
} FeatureHelpEntry;

// Public Self-Registration API
void feature_reg_init(void);
bool feature_register_keyword(const char *keyword, int token_id, const char *category);
bool feature_register_builtin(const char *name, const char *help_text, const char *category);
bool feature_register_help(const char *topic, const char *summary, const char *usage, const char *category);

// Dynamic Introspection Queries
size_t feature_get_keyword_count(void);
const FeatureKeywordEntry* feature_get_keyword_by_index(size_t index);
const FeatureKeywordEntry* feature_find_keyword(const char *name);

size_t feature_get_builtin_count(void);
const FeatureBuiltinEntry* feature_get_builtin_by_index(size_t index);
const FeatureBuiltinEntry* feature_find_builtin(const char *name);

size_t feature_get_help_count(void);
const FeatureHelpEntry* feature_get_help_by_index(size_t index);
const FeatureHelpEntry* feature_find_help(const char *topic);

#endif // CORE_FEATURE_REG_H
