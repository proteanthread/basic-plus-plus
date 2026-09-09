// FILENAME: semantic_harvester.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_rename.c, stmt_revert.c, check.c)
// NEEDS: libkernel (types.h, vm.h, memory.h)
// Provides semantic context clue harvesting from PRINT/INPUT strings and REM comments,
// along with RAMbank snapshot and rollback facilities for legacy BASIC modernization.
//
// ---- Includes ----

#include "statements/program/modernize/semantic_harvester.h"
#include "memory/memory.h"
#include "lexer/lexer.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/ctype/ctype.h"
#include "runtime/funcreg.h"
#include "runtime/language_descriptor.h"
#include "eval/eval_internal.h"

// ---- RAMbank Multi-Level Snapshot Stack ----

typedef struct {
    BppLineNumber line_number;
    char *text;
} SavedProgramLine;

typedef struct {
    char tool_tag[32];
    SavedProgramLine *lines;
    size_t count;
    bool valid;
} RambankSnapshotEntry;

static RambankSnapshotEntry s_rambank_stack[RAMBANK_MAX_SNAPSHOTS];
static int s_rambank_count = 0;

static void rambank_entry_free(RambankSnapshotEntry *entry) {
    if (!entry || !entry->lines) return;
    for (size_t i = 0; i < entry->count; i++) {
        if (entry->lines[i].text) {
            runtime_free(entry->lines[i].text);
        }
    }
    runtime_free(entry->lines);
    entry->lines = NULL;
    entry->count = 0;
    entry->valid = false;
    entry->tool_tag[0] = '\0';
}

int rambank_snapshot_push(VMContext *vm, const char *tool_tag) {
    if (!vm) return -1;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return -1;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines || count == 0) return 0;

    // If stack is full, discard oldest entry (slot 0) and shift left
    if (s_rambank_count >= RAMBANK_MAX_SNAPSHOTS) {
        rambank_entry_free(&s_rambank_stack[0]);
        for (int i = 0; i < RAMBANK_MAX_SNAPSHOTS - 1; i++) {
            s_rambank_stack[i] = s_rambank_stack[i + 1];
        }
        s_rambank_count = RAMBANK_MAX_SNAPSHOTS - 1;
        runtime_memset(&s_rambank_stack[s_rambank_count], 0, sizeof(RambankSnapshotEntry));
    }

    int slot = s_rambank_count;
    s_rambank_stack[slot].lines = (SavedProgramLine *)runtime_malloc(count * sizeof(SavedProgramLine));
    if (!s_rambank_stack[slot].lines) return -1;

    for (size_t i = 0; i < count; i++) {
        s_rambank_stack[slot].lines[i].line_number = lines[i].line_number;
        size_t len = runtime_strlen(lines[i].text);
        s_rambank_stack[slot].lines[i].text = (char *)runtime_malloc(len + 1);
        if (s_rambank_stack[slot].lines[i].text) {
            runtime_memcpy(s_rambank_stack[slot].lines[i].text, lines[i].text, len + 1);
        }
    }
    s_rambank_stack[slot].count = count;
    s_rambank_stack[slot].valid = true;
    runtime_strncpy(s_rambank_stack[slot].tool_tag, tool_tag ? tool_tag : "GENERIC", sizeof(s_rambank_stack[slot].tool_tag) - 1);
    s_rambank_count++;
    return s_rambank_count;
}

bool rambank_snapshot_pop(VMContext *vm, const char *tool_tag) {
    if (!vm || s_rambank_count <= 0) return false;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return false;

    // Find the most recent snapshot matching tool_tag (or any if tool_tag == NULL)
    int target_idx = -1;
    for (int i = s_rambank_count - 1; i >= 0; i--) {
        if (!s_rambank_stack[i].valid) continue;
        if (!tool_tag || runtime_strcasecmp(s_rambank_stack[i].tool_tag, tool_tag) == 0) {
            target_idx = i;
            break;
        }
    }
    // Fallback: if tag not found, pop most recent valid snapshot
    if (target_idx < 0) {
        for (int i = s_rambank_count - 1; i >= 0; i--) {
            if (s_rambank_stack[i].valid) {
                target_idx = i;
                break;
            }
        }
    }
    if (target_idx < 0) return false;

    mem_program_clear(mem);
    for (size_t i = 0; i < s_rambank_stack[target_idx].count; i++) {
        if (s_rambank_stack[target_idx].lines[i].text) {
            mem_program_insert(mem, s_rambank_stack[target_idx].lines[i].line_number, s_rambank_stack[target_idx].lines[i].text);
        }
    }

    rambank_entry_free(&s_rambank_stack[target_idx]);
    for (int i = target_idx; i < s_rambank_count - 1; i++) {
        s_rambank_stack[i] = s_rambank_stack[i + 1];
    }
    s_rambank_count--;
    runtime_memset(&s_rambank_stack[s_rambank_count], 0, sizeof(RambankSnapshotEntry));
    return true;
}

int rambank_snapshot_depth(const char *tool_tag) {
    int depth = 0;
    for (int i = 0; i < s_rambank_count; i++) {
        if (!s_rambank_stack[i].valid) continue;
        if (!tool_tag || runtime_strcasecmp(s_rambank_stack[i].tool_tag, tool_tag) == 0) {
            depth++;
        }
    }
    if (depth == 0 && s_rambank_count > 0) {
        return s_rambank_count;
    }
    return depth;
}

void rambank_snapshot_clear_all(void) {
    for (int i = 0; i < s_rambank_count; i++) {
        rambank_entry_free(&s_rambank_stack[i]);
    }
    s_rambank_count = 0;
}

// Backward compatibility wrappers
bool modernize_snapshot_save(VMContext *vm) { return rambank_snapshot_push(vm, "MODERNIZE") >= 0; }
bool modernize_snapshot_restore(VMContext *vm) { return rambank_snapshot_pop(vm, "MODERNIZE"); }
bool modernize_snapshot_available(void) { return rambank_snapshot_depth("MODERNIZE") > 0; }
void modernize_snapshot_clear(void) { rambank_snapshot_clear_all(); }

// ---- Semantic Casing Formatter ----

void semantic_apply_casing(const char *input_words, char *out_buf, size_t out_cap, ModernizeCaseStyle style) {
    if (!input_words || !out_buf || out_cap == 0) return;
    out_buf[0] = '\0';

    size_t out_pos = 0;
    const char *p = input_words;
    bool new_word = true;
    bool first_word = true;

    while (*p && out_pos < out_cap - 2) {
        if (runtime_isspace((unsigned char)*p) || *p == '_' || *p == '-') {
            if (style == MODERNIZE_CASE_SNAKE || style == MODERNIZE_CASE_SCREAMING) {
                if (out_pos > 0 && out_buf[out_pos - 1] != '_') {
                    out_buf[out_pos++] = '_';
                }
            }
            new_word = true;
            p++;
            continue;
        }

        if (style == MODERNIZE_CASE_SNAKE) {
            out_buf[out_pos++] = (char)runtime_tolower((unsigned char)*p);
        } else if (style == MODERNIZE_CASE_SCREAMING || style == MODERNIZE_CASE_UPPER) {
            out_buf[out_pos++] = (char)runtime_toupper((unsigned char)*p);
        } else if (style == MODERNIZE_CASE_CAMEL) {
            if (first_word) {
                out_buf[out_pos++] = (char)runtime_tolower((unsigned char)*p);
            } else if (new_word) {
                out_buf[out_pos++] = (char)runtime_toupper((unsigned char)*p);
            } else {
                out_buf[out_pos++] = (char)runtime_tolower((unsigned char)*p);
            }
        } else { // MODERNIZE_CASE_MIXED (Default PascalCase)
            if (new_word) {
                out_buf[out_pos++] = (char)runtime_toupper((unsigned char)*p);
            } else {
                out_buf[out_pos++] = (char)runtime_tolower((unsigned char)*p);
            }
        }

        new_word = false;
        if (runtime_isspace((unsigned char)*p)) first_word = false;
        p++;
    }
    out_buf[out_pos] = '\0';
}

// ---- Context Clue Semantic Engine ----

static bool is_stopword(const char *word) {
    static const char *stopwords[] = {
        "input", "enter", "please", "the", "number", "of", "a", "an",
        "you", "was", "is", "to", "for", "in", "with", "and", "or",
        "me", "list", "all", "output", "your", "give", "value", NULL
    };
    for (int i = 0; stopwords[i] != NULL; i++) {
        if (runtime_strcasecmp(word, stopwords[i]) == 0) return true;
    }
    return false;
}

static bool extract_matching_noun_from_string(const char *str, char initial, char *out_noun, size_t out_cap) {
    if (!str || !out_noun || out_cap == 0) return false;
    const char *p = str;
    while (*p) {
        while (*p && !runtime_isalpha((unsigned char)*p)) p++;
        if (!*p) break;
        const char *wstart = p;
        while (*p && runtime_isalpha((unsigned char)*p)) p++;
        size_t wlen = (size_t)(p - wstart);
        if (wlen >= 3 && wlen < 32) {
            char temp[32];
            runtime_snprintf(temp, sizeof(temp), "%.*s", (int)wlen, wstart);
            if (!is_stopword(temp)) {
                if (runtime_toupper((unsigned char)temp[0]) == runtime_toupper((unsigned char)initial)) {
                    runtime_strncpy(out_noun, temp, out_cap - 1);
                    out_noun[out_cap - 1] = '\0';
                    return true;
                }
            }
        }
    }
    return false;
}

static bool is_reserved_or_builtin(const char *name) {
    if (!name || !*name) return false;
    if (lex_find_keyword_by_name(name) != KW_NONE) return true;
    if (funcreg_find_by_name(name) != NULL) return true;
    if (eval_is_builtin_function(name)) return true;
    if (lang_desc_find(name) != NULL) return true;

    char with_dollar[64];
    runtime_snprintf(with_dollar, sizeof(with_dollar), "%s$", name);
    if (lex_find_keyword_by_name(with_dollar) != KW_NONE) return true;
    if (funcreg_find_by_name(with_dollar) != NULL) return true;
    if (eval_is_builtin_function(with_dollar)) return true;
    if (lang_desc_find(with_dollar) != NULL) return true;

    size_t len = runtime_strlen(name);
    if (len > 1 && (name[len - 1] == '$' || name[len - 1] == '%' || name[len - 1] == '!' || name[len - 1] == '#' || name[len - 1] == '&')) {
        char stripped[64];
        runtime_snprintf(stripped, sizeof(stripped), "%.*s", (int)(len - 1), name);
        if (lex_find_keyword_by_name(stripped) != KW_NONE) return true;
        if (funcreg_find_by_name(stripped) != NULL) return true;
        if (eval_is_builtin_function(stripped)) return true;
        if (lang_desc_find(stripped) != NULL) return true;
    }

    if (runtime_strcasecmp(name, "SMART") == 0 ||
        runtime_strcasecmp(name, "UNDO") == 0 ||
        runtime_strcasecmp(name, "RENAME") == 0 ||
        runtime_strcasecmp(name, "REFORMAT") == 0 ||
        runtime_strcasecmp(name, "RENUM") == 0 ||
        runtime_strcasecmp(name, "CHECK") == 0 ||
        runtime_strcasecmp(name, "VERIFY") == 0 ||
        runtime_strcasecmp(name, "MODERNIZE") == 0 ||
        runtime_strcasecmp(name, "REVERT") == 0 ||
        runtime_strcasecmp(name, "CONFLICTS") == 0 ||
        runtime_strcasecmp(name, "UNPACK") == 0 ||
        runtime_strcasecmp(name, "EXPLICIT") == 0 ||
        runtime_strcasecmp(name, "DEFINT") == 0 ||
        runtime_strcasecmp(name, "DEFSNG") == 0 ||
        runtime_strcasecmp(name, "DEFDBL") == 0 ||
        runtime_strcasecmp(name, "DEFLNG") == 0 ||
        runtime_strcasecmp(name, "DEFSTR") == 0 ||
        runtime_strcasecmp(name, "DEFSEG") == 0 ||
        runtime_strcasecmp(name, "DEFUSR") == 0 ||
        runtime_strcasecmp(name, "BLOCKS") == 0) {
        return true;
    }

    return false;
}

static bool is_compound_keyword(const char *name) {
    if (!name || !*name) return false;
    static const char * const kw_prefixes[] = {
        "THEN", "ELSE", "FOR", "NEXT", "IF", "ON", "GOTO", "GOSUB",
        "PRINT", "INPUT", "RETURN", "READ", "DATA", "DIM", "DEF",
        "WEND", "WHILE", "LOOP", "DO", NULL
    };
    for (int i = 0; kw_prefixes[i] != NULL; i++) {
        size_t plen = runtime_strlen(kw_prefixes[i]);
        if (runtime_strncasecmp(name, kw_prefixes[i], plen) == 0) {
            const char *rest = name + plen;
            if (*rest != '\0') {
                return true;
            }
        }
    }
    return false;
}

static bool line_contains_exact_identifier(const char *text, const char *var_name) {
    if (!text || !var_name) return false;
    size_t vlen = runtime_strlen(var_name);
    if (vlen == 0) return false;
    const char *p = text;
    while (*p) {
        if (*p == '"') {
            p++;
            while (*p && *p != '"') p++;
            if (*p == '"') p++;
            continue;
        }
        if (*p == '\'' || (runtime_strncasecmp(p, "REM", 3) == 0 && (p == text || runtime_isspace((unsigned char)*(p - 1))))) {
            break;
        }
        if (runtime_isalpha((unsigned char)*p) || *p == '_') {
            const char *start = p;
            while (*p && (runtime_isalnum((unsigned char)*p) || *p == '_' || *p == '.')) p++;
            size_t len = (size_t)(p - start);
            if (len == vlen && runtime_strncasecmp(start, var_name, vlen) == 0) {
                return true;
            }
        } else {
            p++;
        }
    }
    return false;
}

static bool is_exact_for_loop_counter(const char *text, const char *var_name) {
    if (!text || !var_name) return false;
    size_t vlen = runtime_strlen(var_name);
    if (vlen == 0) return false;
    const char *p = text;
    while (*p) {
        if (*p == '"') {
            p++;
            while (*p && *p != '"') p++;
            if (*p == '"') p++;
            continue;
        }
        if (*p == '\'' || (runtime_strncasecmp(p, "REM", 3) == 0 && (p == text || runtime_isspace((unsigned char)*(p - 1))))) {
            break;
        }
        if (runtime_strncasecmp(p, "FOR", 3) == 0 &&
            (p == text || !runtime_isalnum((unsigned char)*(p - 1)))) {
            const char *after_for = p + 3;
            while (*after_for && runtime_isspace((unsigned char)*after_for)) after_for++;
            if (runtime_strncasecmp(after_for, var_name, vlen) == 0) {
                const char *after_var = after_for + vlen;
                while (*after_var && runtime_isspace((unsigned char)*after_var)) after_var++;
                if (*after_var == '=') {
                    const char *scan_to = after_var + 1;
                    bool has_to = false;
                    while (*scan_to && *scan_to != ':' && *scan_to != '\'' && *scan_to != '\n') {
                        if (runtime_strncasecmp(scan_to, "TO", 2) == 0 &&
                            (scan_to == after_var + 1 || !runtime_isalnum((unsigned char)*(scan_to - 1))) &&
                            !runtime_isalnum((unsigned char)*(scan_to + 2))) {
                            has_to = true;
                            break;
                        }
                        scan_to++;
                    }
                    if (has_to) {
                        return true;
                    }
                }
            }
        }
        p++;
    }
    return false;
}

static bool is_known_array(VMContext *vm, const char *var_name) {
    if (!vm || !var_name) return false;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return false;
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines) return false;
    size_t vlen = runtime_strlen(var_name);

    for (size_t i = 0; i < count; i++) {
        const char *text = lines[i].text;
        if (!text) continue;
        const char *p = text;
        while (*p) {
            if (*p == '"') {
                p++;
                while (*p && *p != '"') p++;
                if (*p == '"') p++;
                continue;
            }
            if (*p == '\'' || (runtime_strncasecmp(p, "REM", 3) == 0 && (p == text || runtime_isspace((unsigned char)*(p - 1))))) {
                break;
            }
            if (runtime_strncasecmp(p, "DIM", 3) == 0 && (p == text || !runtime_isalnum((unsigned char)*(p - 1)))) {
                const char *dp = p + 3;
                while (*dp && *dp != ':' && *dp != '\n') {
                    while (*dp && (runtime_isspace((unsigned char)*dp) || *dp == ',')) dp++;
                    const char *id_start = dp;
                    while (*dp && (runtime_isalnum((unsigned char)*dp) || *dp == '_' || *dp == '$' || *dp == '%' || *dp == '!' || *dp == '#' || *dp == '&')) dp++;
                    size_t id_len = (size_t)(dp - id_start);
                    while (*dp && runtime_isspace((unsigned char)*dp)) dp++;
                    if (*dp == '(') {
                        if (id_len == vlen && runtime_strncasecmp(id_start, var_name, vlen) == 0) {
                            return true;
                        }
                        int depth = 1;
                        dp++;
                        while (*dp && depth > 0) {
                            if (*dp == '(') depth++;
                            else if (*dp == ')') depth--;
                            dp++;
                        }
                    }
                }
            }
            if (runtime_isalpha((unsigned char)*p) || *p == '_') {
                const char *id_start = p;
                while (*p && (runtime_isalnum((unsigned char)*p) || *p == '_' || *p == '$' || *p == '%' || *p == '!' || *p == '#' || *p == '&')) p++;
                size_t id_len = (size_t)(p - id_start);
                const char *after = p;
                while (*after && runtime_isspace((unsigned char)*after)) after++;
                if (*after == '(') {
                    if (id_len == vlen && runtime_strncasecmp(id_start, var_name, vlen) == 0) {
                        if (!is_reserved_or_builtin(var_name)) {
                            return true;
                        }
                    }
                }
            } else {
                p++;
            }
        }
    }
    return false;
}

bool harvest_defint_map(VMContext *vm, bool out_defint_map[26]) {
    if (out_defint_map) runtime_memset(out_defint_map, 0, 26 * sizeof(bool));
    if (!vm || !out_defint_map) return false;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return false;
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines || count == 0) return false;
    bool found = false;
    for (size_t i = 0; i < count; i++) {
        const char *t = lines[i].text;
        if (!t) continue;
        const char *p = runtime_strcasestr(t, "DEFINT");
        if (p && (p == t || !runtime_isalnum((unsigned char)*(p - 1)))) {
            p += 6;
            while (*p && *p != ':' && *p != '\'') {
                while (*p && (runtime_isspace((unsigned char)*p) || *p == ',')) p++;
                if (runtime_isalpha((unsigned char)*p)) {
                    char s = (char)runtime_toupper((unsigned char)*p++), e = s;
                    while (*p && runtime_isspace((unsigned char)*p)) p++;
                    if (*p == '-') {
                        p++;
                        while (*p && runtime_isspace((unsigned char)*p)) p++;
                        if (runtime_isalpha((unsigned char)*p)) e = (char)runtime_toupper((unsigned char)*p++);
                    }
                    if (s >= 'A' && s <= 'Z' && e >= 'A' && e <= 'Z') {
                        if (s > e) { char tmp = s; s = e; e = tmp; }
                        for (char c = s; c <= e; c++) { out_defint_map[c - 'A'] = true; found = true; }
                    }
                } else if (*p) p++;
            }
        }
    }
    return found;
}

bool semantic_infer_variable(VMContext *vm, const char *var_name, char *out_name, size_t out_cap, ModernizeCaseStyle style, int *out_confidence) {
    if (!vm || !var_name || !out_name || out_cap == 0) return false;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return false;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines || count == 0) return false;

    size_t var_len = runtime_strlen(var_name);
    if (var_len == 0) return false;
    char initial = var_name[0];

    // Check if variable is already modernized (mixed case with both upper & lower, length > 2)
    bool has_upper = false;
    bool has_lower = false;
    for (size_t c = 0; c < var_len; c++) {
        if (runtime_isupper((unsigned char)var_name[c])) has_upper = true;
        if (runtime_islower((unsigned char)var_name[c])) has_lower = true;
    }
    if (has_upper && has_lower && var_len > 2) {
        return false;
    }

    if (is_reserved_or_builtin(var_name) || is_compound_keyword(var_name)) {
        return false;
    }

    // Protect dimensioned and indexed arrays from scalar/loop renaming
    if (is_known_array(vm, var_name)) {
        return false;
    }

    // Check if it is an all-caps descriptive identifier (>= 3 chars, e.g. GRAIN, RESIDENTS, BUSHELS)
    if (var_len >= 3) {
        bool all_caps = true;
        for (size_t c = 0; c < var_len; c++) {
            if (!runtime_isupper((unsigned char)var_name[c]) && var_name[c] != '_') {
                all_caps = false;
                break;
            }
        }
        if (all_caps) {
            semantic_apply_casing(var_name, out_name, out_cap, style);
            if (out_confidence) *out_confidence = 100;
            return true;
        }
    }

    // 1. Scan for prompt string adjacency: PRINT "..." ; var  or  INPUT "..." , var
    for (size_t i = 0; i < count; i++) {
        const char *text = lines[i].text;
        if (!text) continue;

        if (runtime_strcasestr(text, "PRINT") || runtime_strcasestr(text, "INPUT")) {
            const char *quote1 = runtime_strchr(text, '"');
            if (quote1) {
                const char *quote2 = runtime_strchr(quote1 + 1, '"');
                if (quote2 && quote2 > quote1) {
                    const char *after_quote = quote2 + 1;
                    while (*after_quote && (runtime_isspace((unsigned char)*after_quote) || *after_quote == ';' || *after_quote == ',')) after_quote++;
                    if (runtime_strncasecmp(after_quote, var_name, var_len) == 0 &&
                        !runtime_isalnum((unsigned char)after_quote[var_len])) {
                        char prompt_buf[128];
                        runtime_snprintf(prompt_buf, sizeof(prompt_buf), "%.*s", (int)(quote2 - quote1 - 1), quote1 + 1);
                        char noun[64] = {0};
                        if (extract_matching_noun_from_string(prompt_buf, initial, noun, sizeof(noun))) {
                            semantic_apply_casing(noun, out_name, out_cap, style);
                            if (out_confidence) *out_confidence = 98;
                            return true;
                        }
                    }
                }
            }
        }
    }

    // 2. Scan for exact FOR loop counters
    bool is_for_counter = false;
    for (size_t i = 0; i < count; i++) {
        if (lines[i].text && is_exact_for_loop_counter(lines[i].text, var_name)) {
            is_for_counter = true;
            break;
        }
    }

    if (is_for_counter) {
        char rem_noun[64] = {0};
        for (size_t i = 0; i < count; i++) {
            if (lines[i].text && is_exact_for_loop_counter(lines[i].text, var_name)) {
                size_t start_idx = (i >= 3) ? (i - 3) : 0;
                for (size_t r = start_idx; r <= i; r++) {
                    const char *rtext = lines[r].text;
                    if (!rtext) continue;
                    if (runtime_strcasestr(rtext, "REM") || runtime_strchr(rtext, '\'')) {
                        if (extract_matching_noun_from_string(rtext, initial, rem_noun, sizeof(rem_noun))) {
                            break;
                        }
                    }
                }
                if (rem_noun[0]) break;
            }
        }
        if (rem_noun[0]) {
            char candidate[64];
            runtime_snprintf(candidate, sizeof(candidate), "%sIdx", rem_noun);
            semantic_apply_casing(candidate, out_name, out_cap, style);
            if (out_confidence) *out_confidence = 88;
            return true;
        }

        // Hybrid loop index formula: LoopIdx.<var_name> (preserves loop identity, adds % if integer/DEFINT)
        bool defint_map[26] = {false};
        bool has_defint = harvest_defint_map(vm, defint_map);
        char first_c = (char)runtime_toupper((unsigned char)var_name[0]);
        bool is_int_var = (runtime_strchr(var_name, '%') != NULL) ||
                          (has_defint && first_c >= 'A' && first_c <= 'Z' && defint_map[first_c - 'A']);
        const char *type_suf = is_int_var ? "%" : "";
        char loop_buf[64];
        if (style == MODERNIZE_CASE_SNAKE) {
            runtime_snprintf(loop_buf, sizeof(loop_buf), "loop_idx.%s%s", var_name, type_suf);
        } else {
            runtime_snprintf(loop_buf, sizeof(loop_buf), "LoopIdx.%s%s", var_name, type_suf);
        }
        runtime_strncpy(out_name, loop_buf, out_cap - 1);
        out_name[out_cap - 1] = '\0';
        if (out_confidence) *out_confidence = 85;
        return true;
    }

    // 3. Scan for preceding REM comments within 3 lines for non-loop variables
    for (size_t i = 0; i < count; i++) {
        const char *text = lines[i].text;
        if (!text) continue;

        if (line_contains_exact_identifier(text, var_name)) {
            size_t start_idx = (i >= 3) ? (i - 3) : 0;
            for (size_t r = start_idx; r <= i; r++) {
                const char *rtext = lines[r].text;
                if (!rtext) continue;
                if (runtime_strcasestr(rtext, "REM") || runtime_strchr(rtext, '\'')) {
                    if (runtime_strcasestr(rtext, "ascii") || runtime_strcasestr(rtext, "character")) {
                        if (runtime_strcasestr(text, "CHR$") || runtime_strcasestr(text, "COLOR")) {
                            semantic_apply_casing("AsciiChar", out_name, out_cap, style);
                            if (out_confidence) *out_confidence = 95;
                            return true;
                        }
                    }
                }
            }
        }
    }

    // 4. Scan for AST function affinities using exact identifier checking
    for (size_t i = 0; i < count; i++) {
        const char *text = lines[i].text;
        if (!text) continue;

        if (line_contains_exact_identifier(text, var_name)) {
            if (runtime_strcasestr(text, "CHR$")) {
                semantic_apply_casing("CharCode", out_name, out_cap, style);
                if (out_confidence) *out_confidence = 90;
                return true;
            }
            if (runtime_strcasestr(text, "ASC(")) {
                semantic_apply_casing("AsciiCode", out_name, out_cap, style);
                if (out_confidence) *out_confidence = 90;
                return true;
            }
            if (runtime_strcasestr(text, "CLOSE #") || runtime_strcasestr(text, "EOF(")) {
                semantic_apply_casing("FileChan", out_name, out_cap, style);
                if (out_confidence) *out_confidence = 90;
                return true;
            }
            if (runtime_strcasestr(text, "COLOR ")) {
                semantic_apply_casing("ColorVal", out_name, out_cap, style);
                if (out_confidence) *out_confidence = 85;
                return true;
            }
        }
    }

    return false;
}

void semantic_harvest_program(VMContext *vm, ContextLexicon *lexicon, ModernizeCaseStyle style) {
    if (!vm || !lexicon) return;
    lexicon->candidate_count = 0;
    lexicon->case_style = style;

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines) return;

    // Scan for single-letter and conflict variables across all lines
    for (size_t i = 0; i < count; i++) {
        const char *text = lines[i].text;
        if (!text) continue;

        const char *p = text;
        while (*p && runtime_isdigit((unsigned char)*p)) p++;
        while (*p && runtime_isspace((unsigned char)*p)) p++;

        while (*p && lexicon->candidate_count < 128) {
            // Skip string literals
            if (*p == '"') {
                p++;
                while (*p && *p != '"') p++;
                if (*p == '"') p++;
                continue;
            }
            // Skip comments
            if (*p == '\'' || (runtime_strncasecmp(p, "REM", 3) == 0 && (p == text || runtime_isspace((unsigned char)*(p - 1))))) {
                break;
            }

            if (runtime_isalpha((unsigned char)*p)) {
                const char *vstart = p;
                while (*p && (runtime_isalnum((unsigned char)*p) || *p == '_')) p++;
                if (*p == '$' || *p == '%' || *p == '!' || *p == '#' || *p == '&') p++;
                size_t vlen = (size_t)(p - vstart);
                char vname[64];
                runtime_snprintf(vname, sizeof(vname), "%.*s", (int)vlen, vstart);

                // Skip built-in keywords, registered functions, and compound keywords
                if (is_reserved_or_builtin(vname) || is_compound_keyword(vname)) {
                    continue;
                }
                char base_vname[64];
                runtime_strncpy(base_vname, vname, sizeof(base_vname) - 1);
                base_vname[sizeof(base_vname) - 1] = '\0';
                size_t bvl = runtime_strlen(base_vname);
                char type_suffix = '\0';
                if (bvl > 1 && (base_vname[bvl - 1] == '$' || base_vname[bvl - 1] == '%' || base_vname[bvl - 1] == '!' || base_vname[bvl - 1] == '#' || base_vname[bvl - 1] == '&')) {
                    type_suffix = base_vname[bvl - 1];
                    base_vname[bvl - 1] = '\0';
                    if (is_reserved_or_builtin(base_vname) || is_compound_keyword(base_vname)) {
                        continue;
                    }
                }

                char modern[64] = {0};
                int conf = 0;
                if (semantic_infer_variable(vm, base_vname, modern, sizeof(modern), style, &conf)) {
                    if (type_suffix != '\0') {
                        size_t mlen = runtime_strlen(modern);
                        if (mlen < sizeof(modern) - 2) {
                            modern[mlen] = type_suffix;
                            modern[mlen + 1] = '\0';
                        }
                    }
                    if (runtime_strcmp(vname, modern) != 0) {
                        // Bijective Safety Invariant: Check if target name collides with another candidate
                        bool target_collision = false;
                        for (size_t c = 0; c < lexicon->candidate_count; c++) {
                            if (runtime_strcasecmp(lexicon->candidates[c].modern_name, modern) == 0) {
                                target_collision = true;
                                break;
                            }
                        }
                        if (target_collision) {
                            continue; // Prevent multiple different variables sharing one target name
                        }

                        // Check if already in candidate list as old_name
                        bool exists = false;
                        for (size_t c = 0; c < lexicon->candidate_count; c++) {
                            if (runtime_strcasecmp(lexicon->candidates[c].old_name, vname) == 0) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            size_t idx = lexicon->candidate_count++;
                            runtime_strncpy(lexicon->candidates[idx].old_name, vname, sizeof(lexicon->candidates[idx].old_name) - 1);
                            runtime_strncpy(lexicon->candidates[idx].modern_name, modern, sizeof(lexicon->candidates[idx].modern_name) - 1);
                            lexicon->candidates[idx].confidence = conf;
                            lexicon->candidates[idx].line_number = (int)lines[i].line_number;
                            lexicon->candidates[idx].source_rule = (conf >= 95) ? "Deep Context Clue Harvester" : "AST Function Affinity";
                        }
                    }
                }
            } else {
                p++;
            }
        }
    }
}
