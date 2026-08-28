// FILENAME: using.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (print_using.c, using_engine.c)
// NEEDED BY: libengine (input.c, lprint.c, print.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides core logic and interface definitions for using within BASIC++.
//
// ---- Includes ----

#ifndef RUNTIME_USING_H
#define RUNTIME_USING_H

#include <stdbool.h>
#include <stddef.h>

#include "vm/vm.h"
#include "lexer/lexer.h"

#define MAX_USING_TOKENS 128

typedef enum {
    USING_TOK_LITERAL,
    USING_TOK_NUMERIC,      // e.g. ###.##
    USING_TOK_STRING_FIELD, // e.g. \    \, &, !, ''
    USING_TOK_FORM_FEED,    // F or Fn
    USING_TOK_LINE_FEED,    // L or Ln
    USING_TOK_BELL,         // G
    USING_TOK_ATTR,         // \A{code}
    USING_TOK_COLUMN,       // C
} UsingTokenType;

typedef struct {
    UsingTokenType type;
    char text[128];         // Raw pattern/literal text
    int code;               // Attribute code or lines count
    char cond_op[8];        // Conditional operator (e.g. "==", ">>", "<>")
    double cond_val1;       // First conditional value
    double cond_val2;       // Second conditional value (for range between)
    bool has_condition;     // True if conditional block present
} UsingToken;

typedef struct {
    UsingToken tokens[MAX_USING_TOKENS];
    int token_count;
} UsingMask;

// API Functions
void using_parse_mask(const char *fmt_str, UsingMask *mask);
bool using_eval_condition(double val, const UsingToken *tok);
void using_format_output(VMContext *vm, const UsingMask *mask, int *mask_idx, BValue val, char *out_buf, size_t out_max);

#include <stdio.h>

// Input Validation API
bool using_validate_char(char c, char mask_char, bool *uppercase, bool *lowercase);
bool using_validate_input_string(const char *input, const char *mask_str, char *err_msg, size_t err_max);

#define PRINT_CHANNEL_CONSOLE -1
#define PRINT_CHANNEL_PRINTER -2

// Print USING internals
void print_using_internal_ex(VMContext *vm, LexerContext *lex, int channel, FILE *stream);
void print_using_internal(VMContext *vm, LexerContext *lex, int channel);

#endif // RUNTIME_USING_H
