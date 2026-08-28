// FILENAME: feature_reg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libengine (mux.c)
// NEEDS: libcore (ctype.h, ctype.c, feature_reg.h, hal.h, memops.h, memops.c)
// NEEDS: libcore (strops.h, strops.c)
// Provides core logic and interface definitions for feature_reg within BASIC++.
//
// ---- Includes ----

#include "core/feature_reg.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "hal/hal.h"

static FeatureKeywordEntry s_keywords[MAX_FEATURE_KEYWORDS];
static size_t s_keyword_count = 0;

static FeatureBuiltinEntry s_builtins[MAX_FEATURE_BUILTINS];
static size_t s_builtin_count = 0;

static FeatureHelpEntry s_help_entries[MAX_FEATURE_HELP_ENTRIES];
static size_t s_help_count = 0;

static void helper_str_toupper(char *dest, const char *src, size_t max_len) {
    size_t i = 0;
    if (!dest || !src || max_len == 0) return;
    for (i = 0; i < max_len - 1 && src[i] != '\0'; i++) {
        dest[i] = (char)runtime_toupper((unsigned char)src[i]);
    }
    dest[i] = '\0';
}

void feature_reg_init(void) {
    s_keyword_count = 0;
    s_builtin_count = 0;
    s_help_count = 0;
    runtime_memset(s_keywords, 0, sizeof(s_keywords));
    runtime_memset(s_builtins, 0, sizeof(s_builtins));
    runtime_memset(s_help_entries, 0, sizeof(s_help_entries));
}

bool feature_register_keyword(const char *keyword, int token_id, const char *category) {
    if (!keyword || s_keyword_count >= MAX_FEATURE_KEYWORDS) return false;
    helper_str_toupper(s_keywords[s_keyword_count].name, keyword, sizeof(s_keywords[0].name));
    s_keywords[s_keyword_count].token_id = token_id;
    s_keywords[s_keyword_count].category = category ? category : "General";
    s_keyword_count++;
    return true;
}

bool feature_register_builtin(const char *name, const char *help_text, const char *category) {
    if (!name || s_builtin_count >= MAX_FEATURE_BUILTINS) return false;
    helper_str_toupper(s_builtins[s_builtin_count].name, name, sizeof(s_builtins[0].name));
    s_builtins[s_builtin_count].help_text = help_text ? help_text : "";
    s_builtins[s_builtin_count].category = category ? category : "General";
    s_builtin_count++;
    return true;
}

bool feature_register_help(const char *topic, const char *summary, const char *usage, const char *category) {
    if (!topic || s_help_count >= MAX_FEATURE_HELP_ENTRIES) return false;
    helper_str_toupper(s_help_entries[s_help_count].topic, topic, sizeof(s_help_entries[0].topic));
    s_help_entries[s_help_count].summary = summary ? summary : "";
    s_help_entries[s_help_count].usage = usage ? usage : "";
    s_help_entries[s_help_count].category = category ? category : "General";
    s_help_count++;
    return true;
}

size_t feature_get_keyword_count(void) {
    return s_keyword_count;
}

const FeatureKeywordEntry* feature_get_keyword_by_index(size_t index) {
    if (index < s_keyword_count) return &s_keywords[index];
    return NULL;
}

const FeatureKeywordEntry* feature_find_keyword(const char *name) {
    char target[32];
    size_t i;
    if (!name) return NULL;
    helper_str_toupper(target, name, sizeof(target));
    for (i = 0; i < s_keyword_count; i++) {
        if (runtime_strcmp(s_keywords[i].name, target) == 0) return &s_keywords[i];
    }
    return NULL;
}

size_t feature_get_builtin_count(void) {
    return s_builtin_count;
}

const FeatureBuiltinEntry* feature_get_builtin_by_index(size_t index) {
    if (index < s_builtin_count) return &s_builtins[index];
    return NULL;
}

const FeatureBuiltinEntry* feature_find_builtin(const char *name) {
    char target[32];
    size_t i;
    if (!name) return NULL;
    helper_str_toupper(target, name, sizeof(target));
    for (i = 0; i < s_builtin_count; i++) {
        if (runtime_strcmp(s_builtins[i].name, target) == 0) return &s_builtins[i];
    }
    return NULL;
}

size_t feature_get_help_count(void) {
    return s_help_count;
}

const FeatureHelpEntry* feature_get_help_by_index(size_t index) {
    if (index < s_help_count) return &s_help_entries[index];
    return NULL;
}

const FeatureHelpEntry* feature_find_help(const char *topic) {
    char target[32];
    size_t i;
    if (!topic) return NULL;
    helper_str_toupper(target, topic, sizeof(target));
    for (i = 0; i < s_help_count; i++) {
        if (runtime_strcmp(s_help_entries[i].topic, target) == 0) return &s_help_entries[i];
    }
    return NULL;
}

