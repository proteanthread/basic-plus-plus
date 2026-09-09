// FILENAME: stmt_rename.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_rename.c)
// NEEDS: libengine (lexer.h, vm.h)
// NEEDS: libkernel (types.h)
// Provides interface definitions for the RENAME statement and function in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_STMT_RENAME_H
#define STATEMENTS_PROGRAM_STMT_RENAME_H

#include <stdbool.h>
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

// Casing standard choices for variable modernization
typedef enum {
    RENAME_CASE_MIXED = 0,    // PascalCase / MixedCase (Default: CharCode, TotalSum)
    RENAME_CASE_SNAKE = 1,    // snake_case (C17 standard: char_code, total_sum)
    RENAME_CASE_CAMEL = 2,    // camelCase (charCode, totalSum)
    RENAME_CASE_CONST = 3,    // SCREAMING_SNAKE_CASE (CHAR_CODE, TOTAL_SUM)
    RENAME_CASE_HUNGARIAN = 4,// Hungarian (intCharCode, strText)
    RENAME_CASE_UPPER = 5     // UPPERCASE (CHARCODE, TOTALSUM)
} RenameCasingStyle;

BppError stmt_rename_handler(VMContext *vm, LexerContext *lex);
void stmt_rename_register(void);
BValue func_rename_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

// Programmatic rename utility for batch and semantic renamers
int rename_symbol_in_program(VMContext *vm, const char *old_name, const char *new_name);
int rename_symbol_in_program_ex(VMContext *vm, const char *old_name, const char *new_name,
                                BppLineNumber *out_lines, size_t max_lines, size_t *out_line_count);

#endif // STATEMENTS_PROGRAM_STMT_RENAME_H
