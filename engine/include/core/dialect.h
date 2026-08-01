/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_dialect.h
 * @brief Dialect Metaprogramming Engine configuration and API declarations.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Defines the data structures and management APIs for dynamic BASIC++
 *   and non-BASIC dialects.
 * - Why it exists: Allows users to configure comment prefix characters, statement separators,
 *   array bases, precedence rules, custom keyword mappings, and preprocessor hooks dynamically.
 * - Why it works this way: It reads options from structured maps (JSON/YAML/INI/XML parsed via BppMap)
 *   and configures the VM context's active dialect configuration dynamically.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Additional properties inside BppDialect, maximum keyword map size limits.
 * - What cannot be changed: Memory lifetime dependencies on the VMContext state.
 * - What to expect: Activating a dialect dynamically shifts the Lexer and Parser scanning patterns.
 * - What to do if something breaks: Check active_dialect pointer inside VMContext and validation returns.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Dialect settings are applied thread-safely per VMContext.
 * - Portability concerns: ANSI/ISO C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional lexer operators and brackets config.
 * - How to write external extensions: Register C-level preprocessor callbacks or bind directly.
 */

#ifndef BPP_DIALECT_H
#define BPP_DIALECT_H

#include <stdbool.h>
#include <stddef.h>

#include "types/types.h"
#include "runtime/map.h"
#include "vm/vm.h"

typedef enum {
    PRECEDENCE_STANDARD = 0,     /* Standard operator precedence (PEMDAS) */
    PRECEDENCE_LEFT_TO_RIGHT     /* Left-to-right calculation evaluation */
} MathPrecedence;

typedef struct {
    char         name[64];
    BppKeywordId id;             /* Standard core keyword ID or dynamic custom ID */
    char         mapped_to[64];   /* Optional alias translation target keyword */
} BppDialectKeyword;

typedef struct BppDialect {
    char             name[64];
    char             comment_char;         /* Custom comment starter, e.g. '#' or ';' or '\'' */
    char             stmt_separator;       /* Custom statement separator, e.g. ';' or ':' */
    int              default_array_base;    /* Array lower bound (0 or 1) */
    bool             case_sensitive;       /* Case-sensitivity for identifiers and keywords */
    MathPrecedence   math_precedence;      /* Precedence mode for expression evaluation */
    BppDialectKeyword keywords[256];       /* Mapped keywords list */
    int              keyword_count;        /* Active mappings count */
    char             preprocessor_hook[64];/* Optional BASIC++ preprocessor function name */
    char             member_access_char;   /* Custom member access dot character, e.g. '.' or 0 if none */
} BppDialect;

/* Core management functions */
BppDialect *dialect_create(void);
void        dialect_free(BppDialect *d);

/* Conversion & loading */
bool        dialect_load_from_map(VMContext *vm, BppMap *map, BppDialect *d, char *err_buf, size_t err_len);
bool        dialect_validate_map(VMContext *vm, BppMap *map, char *err_buf, size_t err_len);
char       *dialect_generate_docs(VMContext *vm, BppDialect *d);

#endif /* BPP_DIALECT_H */
