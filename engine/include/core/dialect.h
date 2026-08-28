// FILENAME: dialect.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (dialect.c)
// NEEDED BY: libengine (context.c, print.c, try.c)
// NEEDS: libengine (map.h, map.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for dialect within BASIC++.
//
// ---- Includes ----

#ifndef DIALECT_H
#define DIALECT_H

#include <stdbool.h>
#include <stddef.h>

#include "types/types.h"
#include "runtime/map.h"
#include "vm/vm.h"

typedef enum {
    PRECEDENCE_STANDARD = 0,     // Standard operator precedence (PEMDAS)
    PRECEDENCE_LEFT_TO_RIGHT     // Left-to-right calculation evaluation
} MathPrecedence;

typedef struct {
    char         name[64];
    BppKeywordId id;             // Standard core keyword ID or dynamic custom ID
    char         mapped_to[64];   // Optional alias translation target keyword
} BppDialectKeyword;

typedef struct BppDialect {
    char             name[64];
    char             comment_char;         // Custom comment starter, e.g. '#' or ';' or '\''
    char             stmt_separator;       // Custom statement separator, e.g. ';' or ':'
    int              default_array_base;    // Array lower bound (0 or 1)
    bool             case_sensitive;       // Case-sensitivity for identifiers and keywords
    MathPrecedence   math_precedence;      // Precedence mode for expression evaluation
    BppDialectKeyword keywords[256];       // Mapped keywords list
    int              keyword_count;        // Active mappings count
    char             preprocessor_hook[64];// Optional BASIC++ preprocessor function name
    char             member_access_char;   // Custom member access dot character, e.g. '.' or 0 if none
} BppDialect;

// Core management functions
BppDialect *dialect_create(void);
void        dialect_free(BppDialect *d);

// Conversion & loading
bool        dialect_load_from_map(VMContext *vm, BppMap *map, BppDialect *d, char *err_buf, size_t err_len);
bool        dialect_validate_map(VMContext *vm, BppMap *map, char *err_buf, size_t err_len);
char       *dialect_generate_docs(VMContext *vm, BppDialect *d);

#endif // DIALECT_H
