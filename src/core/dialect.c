/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file dialect.c
 * @brief Dialect Metaprogramming Engine configuration parser and lifecycle management.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements the validation, parsing, registration, and documentation
 *   generation for custom language specifications in BASIC++.
 * - Why it exists: Fulfills the Phase 25 requirement for a dynamic Dialect Engine,
 *   supporting custom tokenizing, parser option settings, and keyword translation layers.
 * - Why it works this way: It traverses the BppMap structure to extract configuration keys,
 *   performs semantic validation against a schema, and populates the active BppDialect config structure.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Add validation rules or parser parameters, customize documentation layouts.
 * - What cannot be changed: Memory-safe lookup mappings and C17 compliance rules.
 * - What to expect: Changes to keyword maps require standard string copy checks to avoid buffer overflows.
 * - What to do if something breaks: Check schema trace printouts or verify map key existence.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCEREN
 * - Assumptions: Dialect specifications are bounded to 256 mapped keywords maximum.
 * - Portability concerns: ANSI/ISO C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add plugin bindings or dynamic C-level callbacks.
 * - How to write external extensions: Custom VM drivers can query this state to check active dialect parameters.
 */

#include "bpp_dialect.h"
#include "bpp_strings.h"
#include "bpp_vm.h"
#include "bpp_arrays.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

BppDialect *dialect_create(void) {
    BppDialect *d = (BppDialect *)calloc(1, sizeof(BppDialect));
    if (!d) return NULL;
    memset(d, 0, sizeof(BppDialect));
    
    /* Set default configuration options */
    snprintf(d->name, sizeof(d->name), "CUSTOM");
    d->comment_char = '\'';
    d->stmt_separator = ':';
    d->default_array_base = 0;
    d->case_sensitive = false;
    d->math_precedence = PRECEDENCE_STANDARD;
    d->keyword_count = 0;
    d->preprocessor_hook[0] = '\0';
    d->member_access_char = '.';
    return d;
}

void dialect_free(BppDialect *d) {
    if (d) {
        free(d);
    }
}

static char parse_char_value(const char *s) {
    if (strlen(s) == 2 && s[0] == '\\') {
        if (s[1] == 'n') return '\n';
        if (s[1] == 'r') return '\r';
        if (s[1] == 't') return '\t';
        if (s[1] == '\'') return '\'';
        if (s[1] == '\\') return '\\';
    }
    return s[0];
}

bool dialect_validate_map(VMContext *vm, BppMap *map, char *err_buf, size_t err_len) {
    if (!map) {
        snprintf(err_buf, err_len, "Null map pointer");
        return false;
    }

    /* 1. Name Check */
    if (bpp_map_has(map, "name")) {
        BValue val;
        bpp_map_get(map, "name", &val);
        if (val.type != VAL_STRING) {
            snprintf(err_buf, err_len, "Dialect 'name' must be a string");
            return false;
        }
    }

    /* 2. comment_char Check */
    if (bpp_map_has(map, "comment_char")) {
        BValue val;
        bpp_map_get(map, "comment_char", &val);
        if (val.type != VAL_STRING) {
            snprintf(err_buf, err_len, "Dialect 'comment_char' must be a string");
            return false;
        }
        const char *s = str_data(val.as.string);
        size_t len = strlen(s);
        if (len == 2 && s[0] == '\\') {
            if (s[1] != 'n' && s[1] != 'r' && s[1] != 't' && s[1] != '\\' && s[1] != '\'') {
                snprintf(err_buf, err_len, "Unsupported escape sequence in 'comment_char'");
                return false;
            }
        } else if (len > 1) {
            snprintf(err_buf, err_len, "Dialect 'comment_char' must be a single character");
            return false;
        }
    }

    /* 3. stmt_separator Check */
    if (bpp_map_has(map, "stmt_separator")) {
        BValue val;
        bpp_map_get(map, "stmt_separator", &val);
        if (val.type != VAL_STRING) {
            snprintf(err_buf, err_len, "Dialect 'stmt_separator' must be a string");
            return false;
        }
        const char *s = str_data(val.as.string);
        size_t len = strlen(s);
        if (len == 2 && s[0] == '\\') {
            if (s[1] != 'n' && s[1] != 'r' && s[1] != 't' && s[1] != '\\' && s[1] != '\'') {
                snprintf(err_buf, err_len, "Unsupported escape sequence in 'stmt_separator'");
                return false;
            }
        } else if (len > 1) {
            snprintf(err_buf, err_len, "Dialect 'stmt_separator' must be a single character");
            return false;
        }
    }

    /* 4. default_array_base Check */
    if (bpp_map_has(map, "default_array_base")) {
        BValue val;
        bpp_map_get(map, "default_array_base", &val);
        if (val.type != VAL_NUMBER) {
            snprintf(err_buf, err_len, "Dialect 'default_array_base' must be a number");
            return false;
        }
        double base = val.as.number;
        if (base != 0.0 && base != 1.0) {
            snprintf(err_buf, err_len, "Dialect 'default_array_base' must be 0 or 1");
            return false;
        }
    }

    /* 5. case_sensitive Check */
    if (bpp_map_has(map, "case_sensitive")) {
        BValue val;
        bpp_map_get(map, "case_sensitive", &val);
        if (val.type != VAL_NUMBER) {
            snprintf(err_buf, err_len, "Dialect 'case_sensitive' must be a number/boolean");
            return false;
        }
    }

    /* 6. math_precedence Check */
    if (bpp_map_has(map, "math_precedence")) {
        BValue val;
        bpp_map_get(map, "math_precedence", &val);
        if (val.type != VAL_STRING) {
            snprintf(err_buf, err_len, "Dialect 'math_precedence' must be a string ('STANDARD' or 'LEFT_TO_RIGHT')");
            return false;
        }
        const char *s = str_data(val.as.string);
        if (strcasecmp(s, "STANDARD") != 0 && strcasecmp(s, "LEFT_TO_RIGHT") != 0) {
            snprintf(err_buf, err_len, "Dialect 'math_precedence' must be 'STANDARD' or 'LEFT_TO_RIGHT'");
            return false;
        }
    }

    /* 7. Keywords Map Check */
    if (bpp_map_has(map, "keywords")) {
        BValue val;
        bpp_map_get(map, "keywords", &val);
        if (val.type != VAL_MAP) {
            snprintf(err_buf, err_len, "Dialect 'keywords' must be a nested map");
            return false;
        }
        BppMap *kw_map = val.as.map;
        for (int i = 0; i < bpp_map_count(kw_map); ++i) {
            const char *new_key = bpp_map_key(kw_map, i);
            BValue target_val;
            bpp_map_get(kw_map, new_key, &target_val);
            if (target_val.type != VAL_STRING) {
                snprintf(err_buf, err_len, "Dialect keyword mapping target for '%s' must be a string", new_key);
                return false;
            }
        }
    }

    /* 8. preprocessor_hook Check */
    if (bpp_map_has(map, "preprocessor_hook")) {
        BValue val;
        bpp_map_get(map, "preprocessor_hook", &val);
        if (val.type != VAL_STRING) {
            snprintf(err_buf, err_len, "Dialect 'preprocessor_hook' must be a string");
            return false;
        }
    }

    /* 9. member_access_char Check */
    if (bpp_map_has(map, "member_access_char")) {
        BValue val;
        bpp_map_get(map, "member_access_char", &val);
        if (val.type != VAL_STRING) {
            snprintf(err_buf, err_len, "Dialect 'member_access_char' must be a string");
            return false;
        }
        const char *s = str_data(val.as.string);
        if (strlen(s) > 1) {
            snprintf(err_buf, err_len, "Dialect 'member_access_char' must be a single character");
            return false;
        }
    }

    return true;
}

bool dialect_load_from_map(VMContext *vm, BppMap *map, BppDialect *d, char *err_buf, size_t err_len) {
    if (!dialect_validate_map(vm, map, err_buf, err_len)) {
        return false;
    }

    /* Read name */
    if (bpp_map_has(map, "name")) {
        BValue val;
        bpp_map_get(map, "name", &val);
        strncpy(d->name, str_data(val.as.string), sizeof(d->name) - 1);
        d->name[sizeof(d->name) - 1] = '\0';
    }

    /* Read comment character */
    if (bpp_map_has(map, "comment_char")) {
        BValue val;
        bpp_map_get(map, "comment_char", &val);
        const char *s = str_data(val.as.string);
        if (strlen(s) > 0) {
            d->comment_char = parse_char_value(s);
        } else {
            d->comment_char = 0;
        }
    }

    /* Read statement separator */
    if (bpp_map_has(map, "stmt_separator")) {
        BValue val;
        bpp_map_get(map, "stmt_separator", &val);
        const char *s = str_data(val.as.string);
        if (strlen(s) > 0) {
            d->stmt_separator = parse_char_value(s);
        } else {
            d->stmt_separator = 0;
        }
    }

    /* Read array base */
    if (bpp_map_has(map, "default_array_base")) {
        BValue val;
        bpp_map_get(map, "default_array_base", &val);
        d->default_array_base = (int)val.as.number;
        arr_set_option_base(vm_get_arr(vm), d->default_array_base);
    }

    /* Read case sensitive */
    if (bpp_map_has(map, "case_sensitive")) {
        BValue val;
        bpp_map_get(map, "case_sensitive", &val);
        d->case_sensitive = (val.as.number != 0.0);
    }

    /* Read precedence option */
    if (bpp_map_has(map, "math_precedence")) {
        BValue val;
        bpp_map_get(map, "math_precedence", &val);
        const char *s = str_data(val.as.string);
        if (strcasecmp(s, "LEFT_TO_RIGHT") == 0) {
            d->math_precedence = PRECEDENCE_LEFT_TO_RIGHT;
        } else {
            d->math_precedence = PRECEDENCE_STANDARD;
        }
    }

    /* Read keywords mapping list */
    d->keyword_count = 0;
    if (bpp_map_has(map, "keywords")) {
        BValue val;
        bpp_map_get(map, "keywords", &val);
        BppMap *kw_map = val.as.map;
        for (int i = 0; i < bpp_map_count(kw_map); ++i) {
            const char *new_key = bpp_map_key(kw_map, i);
            BValue target_val;
            bpp_map_get(kw_map, new_key, &target_val);
            if (d->keyword_count < 256) {
                BppDialectKeyword *dk = &d->keywords[d->keyword_count++];
                strncpy(dk->name, new_key, sizeof(dk->name) - 1);
                dk->name[sizeof(dk->name) - 1] = '\0';
                
                strncpy(dk->mapped_to, str_data(target_val.as.string), sizeof(dk->mapped_to) - 1);
                dk->mapped_to[sizeof(dk->mapped_to) - 1] = '\0';

                /* Map standard keyword ID */
                dk->id = lex_find_keyword_by_name(dk->mapped_to);
                if (dk->id == KW_NONE) {
                    /* If not standard, register as custom dynamic keyword */
                    dk->id = keyword_register_custom(dk->mapped_to);
                }
            }
        }
    }

    /* Read preprocessor hook name */
    if (bpp_map_has(map, "preprocessor_hook")) {
        BValue val;
        bpp_map_get(map, "preprocessor_hook", &val);
        strncpy(d->preprocessor_hook, str_data(val.as.string), sizeof(d->preprocessor_hook) - 1);
        d->preprocessor_hook[sizeof(d->preprocessor_hook) - 1] = '\0';
    }

    /* Read member_access_char */
    if (bpp_map_has(map, "member_access_char")) {
        BValue val;
        bpp_map_get(map, "member_access_char", &val);
        const char *s = str_data(val.as.string);
        if (strlen(s) > 0) {
            d->member_access_char = s[0];
        } else {
            d->member_access_char = 0;
        }
    } else {
        d->member_access_char = '.';
    }

    return true;
}

char *dialect_generate_docs(VMContext *vm, BppDialect *d) {
    if (!d) return NULL;
    char temp[8192];
    size_t offset = 0;

    /* Clamp offset to prevent unsigned underflow in (sizeof(temp) - offset)
     * if snprintf returns more than the remaining buffer space.
     * snprintf may return the number of chars that WOULD have been written
     * on truncation, so offset can exceed sizeof(temp). This macro prevents
     * subsequent snprintf calls from writing past the buffer and prevents
     * the final memcpy from reading past the stack buffer. */
#define CLAMP_OFFSET() do { if (offset >= sizeof(temp) - 1) offset = sizeof(temp) - 1; } while(0)

    offset += snprintf(temp + offset, sizeof(temp) - offset, "# Dialect: %s Reference Documentation\n\n", d->name);
    CLAMP_OFFSET();
    offset += snprintf(temp + offset, sizeof(temp) - offset, "## Lexer & Tokenizer Settings\n");
    CLAMP_OFFSET();
    offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Comment Prefix character**: '%c'\n", d->comment_char ? d->comment_char : ' ');
    CLAMP_OFFSET();
    offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Statement Separator character**: '%c'\n", d->stmt_separator ? d->stmt_separator : ' ');
    CLAMP_OFFSET();
    offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Case Sensitivity**: %s\n", d->case_sensitive ? "Yes (Case-Sensitive)" : "No (Case-Insensitive)");
    CLAMP_OFFSET();
    if (d->member_access_char) {
        offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Member Access character**: '%c'\n", d->member_access_char);
        CLAMP_OFFSET();
    }
    
    offset += snprintf(temp + offset, sizeof(temp) - offset, "\n## Execution & Parser Semantics\n");
    CLAMP_OFFSET();
    offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Default Array Base Index**: %d\n", d->default_array_base);
    CLAMP_OFFSET();
    offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Math Evaluation Order**: %s\n", (d->math_precedence == PRECEDENCE_LEFT_TO_RIGHT) ? "Left-to-Right" : "Standard PEMDAS");
    CLAMP_OFFSET();
    
    if (d->preprocessor_hook[0]) {
        offset += snprintf(temp + offset, sizeof(temp) - offset, "- **Preprocessor Hook function**: `%s`\n", d->preprocessor_hook);
        CLAMP_OFFSET();
    }

    offset += snprintf(temp + offset, sizeof(temp) - offset, "\n## Mapped Keywords & Syntax Aliases\n");
    CLAMP_OFFSET();
    if (d->keyword_count == 0) {
        offset += snprintf(temp + offset, sizeof(temp) - offset, "No custom keywords registered.\n");
        CLAMP_OFFSET();
    } else {
        offset += snprintf(temp + offset, sizeof(temp) - offset, "| Custom Keyword | Mapped Target / Core Action |\n");
        CLAMP_OFFSET();
        offset += snprintf(temp + offset, sizeof(temp) - offset, "| --- | --- |\n");
        CLAMP_OFFSET();
        for (int i = 0; i < d->keyword_count; ++i) {
            offset += snprintf(temp + offset, sizeof(temp) - offset, "| %s | %s |\n", d->keywords[i].name, d->keywords[i].mapped_to);
            CLAMP_OFFSET();
        }
    }

#undef CLAMP_OFFSET

    char *result = (char *)calloc(1, offset + 1);
    if (result) {
        memcpy(result, temp, offset);
        result[offset] = '\0';
    }
    return result;
}
