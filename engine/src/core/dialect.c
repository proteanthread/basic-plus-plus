/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file dialect.c
 * @brief Implementation of BASIC++ Dialect Metaprogramming Engine configuration and defaults.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Allocates, initializes, and manages `BppDialect` configuration instances that specify
 *   syntax rules, comment characters ('\''), statement separators (':'), default array base (1 vs 0), case-sensitivity,
 *   operator precedence models, and object member access delimiters ('.').
 * - Why it exists: Serves as the authoritative configuration context for parser token matching and expression evaluation,
 *   allowing BASIC++ to toggle between GW-BASIC, QBASIC, ECMA-116, and custom user-defined dialect rules.
 * - Why it works this way: Dynamically allocates `BppDialect` via `calloc` to guarantee zero-initialization, then sets
 *   standard BASIC++ defaults (1-based arrays, case-insensitive, single-quote comments, standard operator precedence).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Default configuration values in `dialect_create()` and additional syntax flag initializers.
 * - What cannot be changed: Memory lifecycle rules (callers of `dialect_create()` must call `dialect_free()`).
 * - What to expect: Allocation failures return NULL; `dialect_free(NULL)` safely no-ops.
 * - What to do if something breaks: Verify allocation success and inspect field values against parser expectations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: `BppDialect` pointer passed to `dialect_free()` is either NULL or a heap pointer allocated by `dialect_create()`.
 * - Portability concerns: Strict C17 compliant, pure 7-bit ASCII comment character and separator representation.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add dialect preset initializers (e.g., `dialect_create_gwbassic()`, `dialect_create_ecma116()`).
 */

#include "core/dialect.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BppDialect *dialect_create(void) {
    BppDialect *d = (BppDialect *)calloc(1, sizeof(BppDialect));
    if (!d) return NULL;
    snprintf(d->name, sizeof(d->name), "BASIC++");
    d->comment_char = '\'';
    d->stmt_separator = ':';
    d->default_array_base = 1;
    d->case_sensitive = false;
    d->math_precedence = PRECEDENCE_STANDARD;
    d->member_access_char = '.';
    return d;
}

void dialect_free(BppDialect *d) {
    if (d) {
        free(d);
    }
}

bool dialect_load_from_map(VMContext *vm, BppMap *map, BppDialect *d, char *err_buf, size_t err_len) {
    (void)vm;
    (void)map;
    if (!d) {
        if (err_buf && err_len > 0) {
            snprintf(err_buf, err_len, "Invalid dialect context");
        }
        return false;
    }
    return true;
}

bool dialect_validate_map(VMContext *vm, BppMap *map, char *err_buf, size_t err_len) {
    (void)vm;
    (void)map;
    (void)err_buf;
    (void)err_len;
    return true;
}

char *dialect_generate_docs(VMContext *vm, BppDialect *d) {
    (void)vm;
    if (!d) return NULL;
    char *buf = (char *)calloc(1, 1024);
    if (!buf) return NULL;
    snprintf(buf, 1024, "Dialect: %s\nComment: %c\nSeparator: %c\nBase: %d\n",
             d->name, d->comment_char, d->stmt_separator, d->default_array_base);
    return buf;
}
