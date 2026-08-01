/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_using.h
 * @brief Unified bidirectional USING formatting & validation engine declarations.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares structures for parsing USING format masks and validation rules.
 * - Why it exists: Provides a single master engine for formatted output (PRINT/LPRINT/TYPE/DISPLAY)
 *   and formatted interactive/batch validation (INPUT/GET/READ).
 * - Why it works this way: Decouples lexing of USING patterns from output devices/file streams,
 *   supporting inline conditions, attributes, and auto-skip character matching.
 */

#ifndef BPP_USING_H
#define BPP_USING_H

#include <stdbool.h>
#include <stddef.h>

#include "vm/vm.h"
#include "lexer/lexer.h"

#define MAX_USING_TOKENS 128

typedef enum {
    USING_TOK_LITERAL,
    USING_TOK_NUMERIC,      /* e.g. ###.## */
    USING_TOK_STRING_FIELD, /* e.g. \    \, &, !, '' */
    USING_TOK_FORM_FEED,    /* F or Fn */
    USING_TOK_LINE_FEED,    /* L or Ln */
    USING_TOK_BELL,         /* G */
    USING_TOK_ATTR,         /* \A{code} */
    USING_TOK_COLUMN,       /* C */
} UsingTokenType;

typedef struct {
    UsingTokenType type;
    char text[128];         /* Raw pattern/literal text */
    int code;               /* Attribute code or lines count */
    char cond_op[8];        /* Conditional operator (e.g. "==", ">>", "<>") */
    double cond_val1;       /* First conditional value */
    double cond_val2;       /* Second conditional value (for range between) */
    bool has_condition;     /* True if conditional block present */
} UsingToken;

typedef struct {
    UsingToken tokens[MAX_USING_TOKENS];
    int token_count;
} UsingMask;

/* API Functions */
void using_parse_mask(const char *fmt_str, UsingMask *mask);
bool using_eval_condition(double val, const UsingToken *tok);
void using_format_output(VMContext *vm, const UsingMask *mask, int *mask_idx, BValue val, char *out_buf, size_t out_max);

#include <stdio.h>

/* Input Validation API */
bool using_validate_char(char c, char mask_char, bool *uppercase, bool *lowercase);
bool using_validate_input_string(const char *input, const char *mask_str, char *err_msg, size_t err_max);

/* Print USING internals */
void print_using_internal_ex(VMContext *vm, LexerContext *lex, int channel, FILE *stream);
void print_using_internal(VMContext *vm, LexerContext *lex, int channel);

#endif /* BPP_USING_H */
