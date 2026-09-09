// FILENAME: print_pos.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (print.c)
// NEEDS: libengine (vm.h, lexer.h)
// NEEDS: libkernel (types.h)
// Provides cursor positioning modifiers (AT, @) for the PRINT statement.
//
// ---- Includes ----

#ifndef STMT_PRINT_POS_H
#define STMT_PRINT_POS_H

#include <stddef.h>
#include <stdbool.h>
#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

BppError parse_print_at_modifier(VMContext *vm, LexerContext *lex, size_t *col, bool *last_was_sep);
BppError parse_print_at_sign_modifier(VMContext *vm, LexerContext *lex, size_t *col, bool *last_was_sep);
void stmt_print_at_register(void);

#endif // STMT_PRINT_POS_H
