// FILENAME: pick_locate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libkernel (config.h), libengine (lexer.h, vm.h)
// Interface definitions for MultiValue dynamic array locate and search operations.
//
// ---- Includes ----

#ifndef BPP_PICK_LOCATE_H
#define BPP_PICK_LOCATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "types/config.h"
#include "types/errors.h"
#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

// Core MultiValue dynamic array search algorithm
// Searches for target in dynarray using AM/VM/SVM delimiters and optional sort order.
// Returns true if found (out_pos is 1-based index).
// Returns false if not found (out_pos is 1-based insertion index).
bool pick_locate_search(const char *target, const char *dynarray, int *out_pos, const char *order);

// Unified statement executor for LOCATE ... IN, FIND ... IN, and ARRAY FIND ... IN
// Called when parser encounters: [LOCATE | FIND | ARRAY FIND] target IN dynarray ...
BppError stmt_pick_locate_execute(VMContext *vm, LexerContext *lex, BValue target);

// Scans ahead from current lexer position to detect if an unparenthesized IN clause follows
bool pick_locate_has_in_clause(LexerContext *lex, const char **out_in_pos);

#endif // BPP_PICK_LOCATE_H
