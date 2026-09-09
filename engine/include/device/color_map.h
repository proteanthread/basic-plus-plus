// FILENAME: color_map.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cls.c, stmt_home.c)
// NEEDS: libkernel (types.h), libengine (lexer.h, vm.h)
// Maps color names, hex codes, and numeric values to IBM PC 16-color palette.
//
// ---- Includes ----

#ifndef DEVICE_COLOR_MAP_H
#define DEVICE_COLOR_MAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

// Translates a color name string to an IBM PC 16-color palette index (0-15), or -1 if unrecognized
int color_name_to_index(const char *name);

// Parses a color argument from lexer (unquoted name, string, hex, or numeric expression)
// Returns color index (0-15 or 24-bit RGB) on success, or -1 if omitted/error
int color_parse_token_or_expr(VMContext *vm, LexerContext *lex, BppError *err);

#endif // DEVICE_COLOR_MAP_H
