// FILENAME: reformat_unpack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (reformat.c, stmt_rename.c)
// NEEDS: libcore (memory.h, string.h)
// NEEDS: libengine (reformat_unpack.h, vm.h)
// Provides decompression and space injection for packed vintage BASIC source lines.

// ---- Includes ----

#include "statements/program/reformat_unpack.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "device/vdev.h"

// Expanded vintage keyword table sorted by length descending
static const char * const s_vintage_keywords[] = {
    // 10+ chars
    "LINE INPUT", "LINEINPUT", "RANDOMIZE",
    // 7-9 chars
    "RESTORE", "LPRINT", "LOCATE", "SCREEN", "CIRCLE", "PRESET",
    "COMMON", "SELECT", "STATIC", "APPEND", "RETURN", "REFORMAT",
    // 5-6 chars
    "PRINT", "INPUT", "WHILE", "COLOR", "BLOAD", "BSAVE",
    "MERGE", "QSAVE", "QLOAD", "GOSUB", "GOTO", "THEN",
    "ELSE", "NEXT", "READ", "DATA", "POKE", "WAIT",
    "PSET", "PAINT", "DRAW", "OPEN", "CLOSE", "FIELD",
    "ERASE", "SWAP", "WEND", "LOOP", "CASE", "SOUND",
    "PLAY", "USING", "BRUN", "QRUN", "CLEAR", "RESET",
    "DEFINT", "DEFSNG", "DEFDBL", "DEFLNG", "DEFSTR",
    "DEFSEG", "DEFUSR", "RENAME", "VERIFY",
    // 4 chars
    "FOR", "STEP", "LET", "DEF", "DIM", "OUT", "CLS",
    "PUT", "GET", "SUB", "END", "NOT", "AND", "XOR",
    "IMP", "EQV", "MOD", "STOP", "RENUM", "CHECK",
    // 2-3 chars
    "IF", "TO", "ON", "DO", "OR", "AS",
    NULL
};

// Built-in function names that take parenthesized arguments
static const char * const s_vintage_funcs[] = {
    "MID$", "LEFT$", "RIGHT$", "CHR$", "STR$", "HEX$", "OCT$", "BIN$",
    "SPACE$", "STRING$", "INKEY$", "VAL", "LEN", "ASC", "INT", "RND",
    "SQR", "SIN", "COS", "TAN", "ATN", "LOG", "EXP", "ABS", "SGN",
    "PEEK", "TAB", "SPC", "POS", "CSRLIN",
    NULL
};

static const char *match_keyword(const char *p, size_t *out_kw_len) {
    if (!p) return NULL;
    for (int i = 0; s_vintage_keywords[i] != NULL; i++) {
        const char *kw = s_vintage_keywords[i];
        size_t kw_len = runtime_strlen(kw);
        if (runtime_strncasecmp(p, kw, kw_len) == 0) {
            *out_kw_len = kw_len;
            return kw;
        }
    }
    return NULL;
}

static const char *match_func(const char *p, size_t *out_fn_len) {
    if (!p) return NULL;
    for (int i = 0; s_vintage_funcs[i] != NULL; i++) {
        const char *fn = s_vintage_funcs[i];
        size_t fn_len = runtime_strlen(fn);
        if (runtime_strncasecmp(p, fn, fn_len) == 0) {
            *out_fn_len = fn_len;
            return fn;
        }
    }
    return NULL;
}

// Returns true if the character is an operator or delimiter
static bool is_punct_or_op(char c) {
    return c == ':' || c == ';' || c == ',' || c == '(' || c == ')' ||
           c == '=' || c == '<' || c == '>' || c == '+' || c == '-' ||
           c == '*' || c == '/' || c == '^' || c == '\\' || c == '\0';
}

// Determines if '-' at current point is a unary minus
static bool is_unary_minus(const char *out, size_t out_pos) {
    if (out_pos == 0) return true;
    size_t k = out_pos;
    while (k > 0 && runtime_isspace((unsigned char)out[k - 1])) {
        k--;
    }
    if (k == 0) return true;
    char last_c = out[k - 1];
    if (last_c == '=' || last_c == '<' || last_c == '>' ||
        last_c == '+' || last_c == '-' || last_c == '*' ||
        last_c == '/' || last_c == '^' || last_c == '(' ||
        last_c == ',' || last_c == ':') {
        return true;
    }
    // Check if previous token was a clause keyword like THEN, ELSE, STEP, TO
    size_t start = k;
    while (start > 0 && runtime_isalpha((unsigned char)out[start - 1])) {
        start--;
    }
    size_t wlen = k - start;
    if (wlen == 4 && runtime_strncasecmp(&out[start], "THEN", 4) == 0) return true;
    if (wlen == 4 && runtime_strncasecmp(&out[start], "ELSE", 4) == 0) return true;
    if (wlen == 4 && runtime_strncasecmp(&out[start], "STEP", 4) == 0) return true;
    if (wlen == 2 && runtime_strncasecmp(&out[start], "TO", 2) == 0) return true;
    return false;
}

// Checks if kw is valid in statement grammar context
static bool is_valid_keyword_in_context(const char *kw, const char *line_start, const char *p, const char *after) {
    if (!kw || !p || !after) return false;

    // Find start of current statement (after line number or last ':')
    const char *stmt = p;
    while (stmt > line_start && *(stmt - 1) != ':') {
        stmt--;
    }
    while (*stmt && (runtime_isspace((unsigned char)*stmt) || runtime_isdigit((unsigned char)*stmt))) {
        stmt++;
    }

    bool at_stmt_head = (p == stmt);

    // 1. Clause keyword "TO": ONLY valid inside a FOR statement after '='
    if (runtime_strcasecmp(kw, "TO") == 0) {
        if (at_stmt_head) return false; // A statement never begins with TO
        if (!runtime_strcasestr(stmt, "FOR") && !runtime_strcasestr(stmt, "for")) return false;
        const char *eq = runtime_strchr(stmt, '=');
        if (!eq || eq >= p) return false;
        return true;
    }

    // 2. Clause keyword "STEP": ONLY valid inside a FOR statement after "TO"
    if (runtime_strcasecmp(kw, "STEP") == 0) {
        if (at_stmt_head) return false;
        if (!runtime_strcasestr(stmt, "TO") && !runtime_strcasestr(stmt, "to")) return false;
        return true;
    }

    // 3. Clause keyword "THEN": ONLY valid inside an IF statement
    if (runtime_strcasecmp(kw, "THEN") == 0) {
        if (at_stmt_head) return false;
        if (!runtime_strcasestr(stmt, "IF") && !runtime_strcasestr(stmt, "if")) return false;
        return true;
    }

    // 4. Clause keyword "AS": ONLY valid in OPEN, NAME, or DIM
    if (runtime_strcasecmp(kw, "AS") == 0) {
        if (at_stmt_head) return false;
        if (!runtime_strcasestr(stmt, "OPEN") && !runtime_strcasestr(stmt, "open") &&
            !runtime_strcasestr(stmt, "NAME") && !runtime_strcasestr(stmt, "name") &&
            !runtime_strcasestr(stmt, "DIM")  && !runtime_strcasestr(stmt, "dim")) return false;
        return true;
    }

    // 5. Statement keyword "FOR": MUST be followed by var = ... TO ...
    if (runtime_strcasecmp(kw, "FOR") == 0) {
        const char *end = after;
        bool in_q = false;
        const char *found_to = NULL;
        const char *found_eq = NULL;
        while (*end && (*end != ':' || in_q)) {
            if (*end == '"') in_q = !in_q;
            if (!in_q) {
                if (*end == '=' && !found_eq) found_eq = end;
                if ((runtime_strncasecmp(end, "TO", 2) == 0) &&
                    (runtime_isspace((unsigned char)*(end + 2)) || runtime_isdigit((unsigned char)*(end + 2)) ||
                     *(end + 2) == '-' || *(end + 2) == '+' || *(end + 2) == '(' || runtime_isalpha((unsigned char)*(end + 2)))) {
                    if (found_eq && end > found_eq) found_to = end;
                }
            }
            end++;
        }
        if (!found_eq || !found_to) return false; // Not a FOR loop (e.g. FORMAT$ or FORM = 1)
        return true;
    }

    // 6. Statement keyword "DEF": ONLY for DEF FN (user-defined functions)
    if (runtime_strcasecmp(kw, "DEF") == 0) {
        while (*after && runtime_isspace((unsigned char)*after)) after++;
        if (runtime_strncasecmp(after, "FN", 2) != 0) {
            return false; // DEF is only for DEF FN; DEFINT/DEFSNG/etc. are atomic keywords
        }
        return true;
    }

    // 7. Statement keyword "ON": MUST be followed by GOTO, GOSUB, or ERROR
    if (runtime_strcasecmp(kw, "ON") == 0) {
        const char *end = after;
        bool in_q = false;
        bool has_target = false;
        while (*end && (*end != ':' || in_q)) {
            if (*end == '"') in_q = !in_q;
            if (!in_q) {
                if (runtime_strncasecmp(end, "GOTO", 4) == 0 ||
                    runtime_strncasecmp(end, "GOSUB", 5) == 0 ||
                    runtime_strncasecmp(end, "ERROR", 5) == 0) {
                    has_target = true;
                    break;
                }
            }
            end++;
        }
        if (!has_target) return false; // e.g. ONLY = 1, ONCE = 2
        return true;
    }

    // 8. Operator keywords: AND, OR, XOR, EQV, IMP, MOD
    if (runtime_strcasecmp(kw, "AND") == 0 || runtime_strcasecmp(kw, "OR") == 0 ||
        runtime_strcasecmp(kw, "XOR") == 0 || runtime_strcasecmp(kw, "EQV") == 0 ||
        runtime_strcasecmp(kw, "IMP") == 0 || runtime_strcasecmp(kw, "MOD") == 0) {
        if (at_stmt_head) return false;
        if (p > line_start && runtime_isalpha((unsigned char)*(p - 1))) return false;
        if (runtime_isalpha((unsigned char)*after)) return false;
        const char *chk = after;
        while (*chk && runtime_isspace((unsigned char)*chk)) chk++;
        if (*chk == '=') return false;
        return true;
    }

    return true;
}

// Unpacks a source line into the destination buffer with full Pascal/Algol spacing.
// Returns true on success.
bool reformat_unpack_buffer(const char *src_line, char *out, size_t out_cap) {
    if (!src_line || !out || out_cap < 32) return false;

    size_t out_pos = 0;
    const char *p = src_line;

    // Skip leading whitespace
    while (*p && runtime_isspace((unsigned char)*p)) p++;

    // Copy leading line number if present
    if (runtime_isdigit((unsigned char)*p)) {
        while (*p && runtime_isdigit((unsigned char)*p) && out_pos < out_cap - 2) {
            out[out_pos++] = *p++;
        }
        out[out_pos++] = ' ';
        while (*p && runtime_isspace((unsigned char)*p)) p++;
    }

    // Pragmas (!!) and Directives (::) are 100% byte-for-byte immutable
    if ((p[0] == '!' && p[1] == '!') || (p[0] == ':' && p[1] == ':')) {
        runtime_strncpy(out, src_line, out_cap - 1);
        out[out_cap - 1] = '\0';
        return true;
    }

    bool in_quote = false;

    while (*p && out_pos < out_cap - 16) {
        // 1. String literal protection
        if (*p == '"') {
            in_quote = true;
            out[out_pos++] = *p++;
            while (*p && in_quote && out_pos < out_cap - 2) {
                out[out_pos++] = *p;
                if (*p == '"') {
                    in_quote = false;
                }
                p++;
            }
            continue;
        }

        // 2. Comment protection (REM / ')
        if (*p == '\'' || (runtime_strncasecmp(p, "REM", 3) == 0 &&
                           (runtime_isspace((unsigned char)*(p + 3)) || *(p + 3) == '\0' || *(p + 3) == ':'))) {
            while (*p && out_pos < out_cap - 1) {
                out[out_pos++] = *p++;
            }
            break;
        }

        // 3. DATA statement protection
        if (runtime_strncasecmp(p, "DATA", 4) == 0 &&
            (p == src_line || !runtime_isalnum((unsigned char)*(p - 1))) &&
            (runtime_isspace((unsigned char)*(p + 4)) || *(p + 4) == '\0' || *(p + 4) == ':')) {
            while (*p && *p != ':' && out_pos < out_cap - 1) {
                out[out_pos++] = *p++;
            }
            continue;
        }

        // 4. Statement separator colon (:): formatted as ' : '
        if (*p == ':') {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = ':';
            out[out_pos++] = ' ';
            p++;
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            continue;
        }

        // 5. Comma separator (,): formatted as ', '
        if (*p == ',') {
            while (out_pos > 0 && out[out_pos - 1] == ' ') {
                out_pos--;
            }
            out[out_pos++] = ',';
            out[out_pos++] = ' ';
            p++;
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            continue;
        }

        // 6. Semicolon separator (;): formatted as ' ; '
        if (*p == ';') {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = ';';
            p++;
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            if (*p != '\0' && *p != ';') {
                out[out_pos++] = ' ';
            }
            continue;
        }

        // 7. Parentheses: '(' formatted as ' ( ', ')' formatted as ' ) '
        if (*p == '(') {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = '(';
            p++;
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            if (*p != '\0' && *p != ')') {
                out[out_pos++] = ' ';
            }
            continue;
        }
        if (*p == ')') {
            if (out_pos > 0 && out[out_pos - 1] != ' ' && out[out_pos - 1] != '(') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = ')';
            p++;
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            if (*p != '\0' && !is_punct_or_op(*p)) {
                out[out_pos++] = ' ';
            }
            continue;
        }

        // 8. Multi-character relational operators: <=, <>, >=
        if ((*p == '<' && (*(p + 1) == '=' || *(p + 1) == '>')) ||
            (*p == '>' && *(p + 1) == '=')) {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = *p++;
            out[out_pos++] = *p++;
            out[out_pos++] = ' ';
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            continue;
        }

        // 9. Single-character relational / equality operators: =, <, >
        if (*p == '=' || *p == '<' || *p == '>') {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = *p++;
            out[out_pos++] = ' ';
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            continue;
        }

        // 10. Arithmetic operators: +, *, /, ^
        if (*p == '+' || *p == '*' || *p == '/' || *p == '^') {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            out[out_pos++] = *p++;
            out[out_pos++] = ' ';
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            continue;
        }

        // 11. Minus (-): Unary vs Binary
        if (*p == '-') {
            if (is_unary_minus(out, out_pos)) {
                if (out_pos > 0 && out[out_pos - 1] != ' ' &&
                    out[out_pos - 1] != '(' && out[out_pos - 1] != ',') {
                    out[out_pos++] = ' ';
                }
                out[out_pos++] = *p++;
                while (*p && runtime_isspace((unsigned char)*p)) p++;
            } else {
                if (out_pos > 0 && out[out_pos - 1] != ' ') {
                    out[out_pos++] = ' ';
                }
                out[out_pos++] = *p++;
                out[out_pos++] = ' ';
                while (*p && runtime_isspace((unsigned char)*p)) p++;
            }
            continue;
        }

        // 12. Function calls like MID$(, CHR$(
        size_t fn_len = 0;
        const char *fn = match_func(p, &fn_len);
        if (fn) {
            char prev_c = (out_pos > 0) ? out[out_pos - 1] : ' ';
            if (!runtime_isalnum((unsigned char)prev_c) && prev_c != '_') {
                if (out_pos > 0 && out[out_pos - 1] != ' ' && out[out_pos - 1] != '(') {
                    out[out_pos++] = ' ';
                }
                for (size_t k = 0; k < fn_len; k++) {
                    out[out_pos++] = p[k];
                }
                p += fn_len;
                while (*p && runtime_isspace((unsigned char)*p)) p++;
                if (*p == '(') {
                    out[out_pos++] = ' ';
                }
                continue;
            }
        }

        // 13. Keyword matching and decompression
        size_t kw_len = 0;
        const char *kw = match_keyword(p, &kw_len);
        if (kw && is_valid_keyword_in_context(kw, src_line, p, p + kw_len)) {
            char prev_c = (out_pos > 0) ? out[out_pos - 1] : ' ';
            bool prev_is_digit = runtime_isdigit((unsigned char)prev_c);
            bool prev_is_delim = !runtime_isalnum((unsigned char)prev_c) && prev_c != '_';
            bool is_chained_flow_kw = (runtime_strcasecmp(kw, "THEN") == 0 ||
                                       runtime_strcasecmp(kw, "GOTO") == 0 ||
                                       runtime_strcasecmp(kw, "GOSUB") == 0 ||
                                       runtime_strcasecmp(kw, "TO") == 0 ||
                                       runtime_strcasecmp(kw, "STEP") == 0);

            if (prev_is_delim || prev_is_digit || is_chained_flow_kw) {
                const char *after = p + kw_len;
                bool is_valid_after = false;

                if (*after == '\0' || runtime_isspace((unsigned char)*after) || is_punct_or_op(*after)) {
                    is_valid_after = true;
                } else if (runtime_isdigit((unsigned char)*after)) {
                    is_valid_after = true; // e.g. GOTO9040, THEN100, TO10, STEP2
                } else {
                    // Check if followed by another keyword (e.g. THEN ON) or function (e.g. IF INT)
                    size_t next_kw_len = 0;
                    size_t next_fn_len = 0;
                    if (match_keyword(after, &next_kw_len) || match_func(after, &next_fn_len)) {
                        is_valid_after = true;
                    } else {
                        // Check identifier length: vintage variable name up to 3 chars followed by operator/punct
                        size_t var_len = 0;
                        const char *chk = after;
                        while (chk[var_len] && (runtime_isalnum((unsigned char)chk[var_len]) ||
                               chk[var_len] == '$' || chk[var_len] == '%' || chk[var_len] == '!' ||
                               chk[var_len] == '#' || chk[var_len] == '&')) {
                            var_len++;
                        }
                        if (var_len <= 3 && runtime_isalpha((unsigned char)*after)) {
                            char after_var = chk[var_len];
                            if (after_var == '\0' || runtime_isspace((unsigned char)after_var) || is_punct_or_op(after_var)) {
                                is_valid_after = true; // e.g. IF Z5, FOR I, NEXT I, ON Z4, PRINT O1$
                            }
                        }
                    }
                }

                if (is_valid_after) {
                    if (out_pos > 0 && out[out_pos - 1] != ' ' && out[out_pos - 1] != '(') {
                        out[out_pos++] = ' ';
                    }
                    for (size_t k = 0; k < kw_len; k++) {
                        out[out_pos++] = p[k];
                    }
                    p += kw_len;
                    while (*p && runtime_isspace((unsigned char)*p)) p++;
                    if (*p != '\0' && *p != ':' && *p != ',' && *p != ';') {
                        out[out_pos++] = ' ';
                    }
                    continue;
                }
            }
        }

        // 14. Collapse consecutive spaces in input
        if (runtime_isspace((unsigned char)*p)) {
            if (out_pos > 0 && out[out_pos - 1] != ' ') {
                out[out_pos++] = ' ';
            }
            while (*p && runtime_isspace((unsigned char)*p)) p++;
            continue;
        }

        // 15. Default: copy verbatim
        out[out_pos++] = *p++;
    }

    // Trim trailing whitespace
    while (out_pos > 0 && runtime_isspace((unsigned char)out[out_pos - 1])) {
        out_pos--;
    }
    out[out_pos] = '\0';
    return true;
}

bool reformat_is_packed_line(const char *src_line) {
    if (!src_line) return false;
    char unpacked[4096];
    if (!reformat_unpack_buffer(src_line, unpacked, sizeof(unpacked))) {
        return false;
    }
    return (runtime_strcmp(src_line, unpacked) != 0);
}

char *reformat_unpack_line(const char *src_line, void *mem_ctx) {
    (void)mem_ctx;
    if (!src_line) return NULL;
    char unpacked[4096];
    if (!reformat_unpack_buffer(src_line, unpacked, sizeof(unpacked))) {
        return NULL;
    }
    size_t ulen = runtime_strlen(unpacked);
    char *out = (char *)runtime_calloc(1, ulen + 1);
    if (!out) return NULL;
    runtime_memcpy(out, unpacked, ulen);
    out[ulen] = '\0';
    return out;
}

int reformat_unpack_program(VMContext *vm) {
    if (!vm) return 0;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return 0;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (!lines || line_count == 0) return 0;

    int modified_count = 0;
    char unpacked[4096];

    for (size_t i = 0; i < line_count; i++) {
        BppLineNumber line_num = lines[i].line_number;
        const char *line_text = lines[i].text;
        if (!line_text) continue;

        if (reformat_unpack_buffer(line_text, unpacked, sizeof(unpacked))) {
            if (runtime_strcmp(line_text, unpacked) != 0) {
                mem_program_insert(mem, line_num, unpacked);
                modified_count++;
            }
        }
    }
    return modified_count;
}

bool reformat_expand_open_buffer(const char *src_line, char *out, size_t out_cap) {
    if (!src_line || !out || out_cap == 0) return false;
    out[0] = '\0';

    const char *p = src_line;
    size_t out_pos = 0;
    bool modified = false;

    while (*p && out_pos < out_cap - 1) {
        if (*p == '"') {
            out[out_pos++] = *p++;
            while (*p && *p != '"' && out_pos < out_cap - 1) {
                out[out_pos++] = *p++;
            }
            if (*p == '"' && out_pos < out_cap - 1) {
                out[out_pos++] = *p++;
            }
            continue;
        }

        if (*p == '\'' || (runtime_strncasecmp(p, "REM", 3) == 0 && (p == src_line || runtime_isspace((unsigned char)*(p - 1))))) {
            while (*p && out_pos < out_cap - 1) {
                out[out_pos++] = *p++;
            }
            break;
        }

        if ((p == src_line || runtime_isspace((unsigned char)*(p - 1)) || *(p - 1) == ':') &&
            runtime_strncasecmp(p, "OPEN", 4) == 0 &&
            (runtime_isspace((unsigned char)p[4]) || p[4] == '"')) {

            const char *scan = p + 4;
            while (*scan && runtime_isspace((unsigned char)*scan)) scan++;

            if (*scan == '"' && scan[1] && scan[2] == '"') {
                char m = scan[1];
                const char *mode_kw = NULL;
                if (m == 'I' || m == 'i') mode_kw = "INPUT";
                else if (m == 'O' || m == 'o') mode_kw = "OUTPUT";
                else if (m == 'A' || m == 'a') mode_kw = "APPEND";
                else if (m == 'R' || m == 'r') mode_kw = "RANDOM";
                else if (m == 'B' || m == 'b') mode_kw = "BINARY";

                if (mode_kw) {
                    const char *comma1 = scan + 3;
                    while (*comma1 && runtime_isspace((unsigned char)*comma1)) comma1++;
                    if (*comma1 == ',') {
                        const char *ch_start = comma1 + 1;
                        while (*ch_start && (runtime_isspace((unsigned char)*ch_start) || *ch_start == '#')) ch_start++;
                        const char *comma2 = ch_start;
                        while (*comma2 && *comma2 != ',' && *comma2 != ':' && *comma2 != '\n') comma2++;

                        if (*comma2 == ',') {
                            char chan_str[32] = {0};
                            size_t ch_len = (size_t)(comma2 - ch_start);
                            while (ch_len > 0 && runtime_isspace((unsigned char)ch_start[ch_len - 1])) ch_len--;
                            if (ch_len < sizeof(chan_str)) {
                                runtime_memcpy(chan_str, ch_start, ch_len);
                                chan_str[ch_len] = '\0';
                            }

                            const char *fn_start = comma2 + 1;
                            while (*fn_start && runtime_isspace((unsigned char)*fn_start)) fn_start++;
                            const char *fn_end = fn_start;
                            if (*fn_end == '"') {
                                fn_end++;
                                while (*fn_end && *fn_end != '"') fn_end++;
                                if (*fn_end == '"') fn_end++;
                            } else {
                                while (*fn_end && *fn_end != ',' && *fn_end != ':' && *fn_end != '\n') fn_end++;
                            }

                            char fn_str[256] = {0};
                            size_t fn_len = (size_t)(fn_end - fn_start);
                            while (fn_len > 0 && runtime_isspace((unsigned char)fn_start[fn_len - 1])) fn_len--;
                            if (fn_len < sizeof(fn_str)) {
                                runtime_memcpy(fn_str, fn_start, fn_len);
                                fn_str[fn_len] = '\0';
                            }

                            const char *rec_start = fn_end;
                            while (*rec_start && runtime_isspace((unsigned char)*rec_start)) rec_start++;
                            char rec_str[32] = {0};
                            if (*rec_start == ',') {
                                rec_start++;
                                while (*rec_start && runtime_isspace((unsigned char)*rec_start)) rec_start++;
                                const char *rec_end = rec_start;
                                while (*rec_end && *rec_end != ':' && *rec_end != '\n') rec_end++;
                                size_t r_len = (size_t)(rec_end - rec_start);
                                while (r_len > 0 && runtime_isspace((unsigned char)rec_start[r_len - 1])) r_len--;
                                if (r_len < sizeof(rec_str)) {
                                    runtime_memcpy(rec_str, rec_start, r_len);
                                    rec_str[r_len] = '\0';
                                }
                                fn_end = rec_end;
                            }

                            if (chan_str[0] && fn_str[0]) {
                                char expanded[512];
                                if (rec_str[0]) {
                                    runtime_snprintf(expanded, sizeof(expanded), "OPEN %s FOR %s AS #%s LEN = %s",
                                                     fn_str, mode_kw, chan_str, rec_str);
                                } else {
                                    runtime_snprintf(expanded, sizeof(expanded), "OPEN %s FOR %s AS #%s",
                                                     fn_str, mode_kw, chan_str);
                                }

                                size_t explen = runtime_strlen(expanded);
                                if (out_pos + explen < out_cap - 1) {
                                    runtime_memcpy(out + out_pos, expanded, explen);
                                    out_pos += explen;
                                    p = fn_end;
                                    modified = true;
                                    continue;
                                }
                            }
                        }
                    }
                }
            }
        }

        out[out_pos++] = *p++;
    }

    out[out_pos] = '\0';
    return modified;
}

int reformat_expand_open_program(VMContext *vm) {
    if (!vm) return 0;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return 0;

    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (!lines || line_count == 0) return 0;

    int modified_count = 0;
    char expanded[4096];

    for (size_t i = 0; i < line_count; i++) {
        BppLineNumber line_num = lines[i].line_number;
        const char *line_text = lines[i].text;
        if (!line_text) continue;

        if (reformat_expand_open_buffer(line_text, expanded, sizeof(expanded))) {
            mem_program_insert(mem, line_num, expanded);
            modified_count++;
        }
    }
    return modified_count;
}
