/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file using.h
 * @brief Runtime component implementation and public API surface for using.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for using.h within the runtime subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file using.h
 * @brief Unified bidirectional USING formatting & validation engine declarations.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares structures for parsing USING format masks and validation rules.
 * - Why it exists: Provides a single master engine for formatted output (PRINT/LPRINT/TYPE/DISPLAY)
 *   and formatted interactive/batch validation (INPUT/GET/READ).
 * - Why it works this way: Decouples lexing of USING patterns from output devices/file streams,
 *   supporting inline conditions, attributes, and auto-skip character matching.
 */

#ifndef RUNTIME_USING_H
#define RUNTIME_USING_H

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

#endif /* RUNTIME_USING_H */
