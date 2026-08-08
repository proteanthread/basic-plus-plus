/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file print_using.c
 * @brief Runtime component implementation and public API surface for print_using.c.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for print_using.c within the runtime subsystem.
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
 * @file print_using.c
 * @brief PRINT USING formatted output engine.
 */

#include "stmt/stmt.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "runtime/file.h"
#include "runtime/using.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_using_internal_ex(VMContext *vm, LexerContext *lex, int channel, FILE *stream) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* 1. Format string expression */
    BValue fmt_val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        vm_set_error(vm, err.code, err.message);
        return;
    }
    if (fmt_val.type != VAL_STRING) {
        vm_set_error(vm, 13, "Type mismatch: PRINT USING expects format string");
        return;
    }
    const char *fmt_str = str_data(fmt_val.as.string);

    /* 2. Semicolon separator */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_SEMICOLON) {
        vm_set_error(vm, 2, "Expected ';' in PRINT USING statement");
        str_release(vm_get_str(vm), fmt_val.as.string);
        return;
    }

    /* Parse USING format mask */
    UsingMask mask;
    using_parse_mask(fmt_str, &mask);

    int mask_idx = 0;
    bool last_was_sep = false;
    BValue val_none;
    val_none.type = VAL_NONE;
    val_none.as.number = 0.0;

    while (true) {
        /* Print leading literals */
        while (mask_idx < mask.token_count) {
            UsingTokenType t_type = mask.tokens[mask_idx].type;
            if (t_type == USING_TOK_LITERAL || t_type == USING_TOK_ATTR || 
                t_type == USING_TOK_FORM_FEED || t_type == USING_TOK_LINE_FEED || 
                t_type == USING_TOK_BELL) {
                
                char out_buf[256];
                using_format_output(vm, &mask, &mask_idx, val_none, out_buf, sizeof(out_buf));
                if (stream) {
                    fprintf(stream, "%s", out_buf);
                } else if (channel == -1) {
                    vdev_puts(vm_get_vdev(vm), out_buf);
                } else {
                    file_puts(vm_get_file(vm), channel, out_buf);
                }
            } else {
                break;
            }
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
            break;
        }

        /* Evaluate next expression to format */
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            vm_set_error(vm, err.code, err.message);
            str_release(vm_get_str(vm), fmt_val.as.string);
            return;
        }

        /* Recycle mask index if we reached the end of format tokens */
        if (mask_idx >= mask.token_count) {
            mask_idx = 0;
            while (mask_idx < mask.token_count) {
                UsingTokenType t_type = mask.tokens[mask_idx].type;
                if (t_type == USING_TOK_LITERAL || t_type == USING_TOK_ATTR || 
                    t_type == USING_TOK_FORM_FEED || t_type == USING_TOK_LINE_FEED || 
                    t_type == USING_TOK_BELL) {
                    
                    char out_buf[256];
                    using_format_output(vm, &mask, &mask_idx, val_none, out_buf, sizeof(out_buf));
                    if (stream) {
                        fprintf(stream, "%s", out_buf);
                    } else if (channel == -1) {
                        vdev_puts(vm_get_vdev(vm), out_buf);
                    } else {
                        file_puts(vm_get_file(vm), channel, out_buf);
                    }
                } else {
                    break;
                }
            }
        }

        /* Format variable value using the specifier token */
        char out_buf[256];
        out_buf[0] = '\0';
        using_format_output(vm, &mask, &mask_idx, val, out_buf, sizeof(out_buf));

        if (stream) {
            fprintf(stream, "%s", out_buf);
        } else if (channel == -1) {
            vdev_puts(vm_get_vdev(vm), out_buf);
        } else {
            file_puts(vm_get_file(vm), channel, out_buf);
        }

        if (val.type == VAL_STRING && val.as.string) {
            str_release(vm_get_str(vm), val.as.string);
        }

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA || tok.type == TOK_SEMICOLON) {
            lex_next(lex);
            last_was_sep = true;
        } else {
            last_was_sep = false;
        }
    }

    /* Print trailing literals */
    while (mask_idx < mask.token_count) {
        UsingTokenType t_type = mask.tokens[mask_idx].type;
        if (t_type == USING_TOK_LITERAL || t_type == USING_TOK_ATTR || 
            t_type == USING_TOK_FORM_FEED || t_type == USING_TOK_LINE_FEED || 
            t_type == USING_TOK_BELL) {
            
            char out_buf[256];
            using_format_output(vm, &mask, &mask_idx, val_none, out_buf, sizeof(out_buf));
            if (stream) {
                fprintf(stream, "%s", out_buf);
            } else if (channel == -1) {
                vdev_puts(vm_get_vdev(vm), out_buf);
            } else {
                file_puts(vm_get_file(vm), channel, out_buf);
            }
        } else {
            break;
        }
    }

    if (!last_was_sep) {
        if (stream) {
            fprintf(stream, "\n");
        } else if (channel == -1) {
            vdev_putc(vm_get_vdev(vm), '\n');
        } else {
            file_putc(vm_get_file(vm), channel, '\n');
        }
    }
    if (stream) {
        fflush(stream);
    } else if (channel != -1) {
        file_flush(vm_get_file(vm), channel);
    }
    str_release(vm_get_str(vm), fmt_val.as.string);
}

void print_using_internal(VMContext *vm, LexerContext *lex, int channel) {
    print_using_internal_ex(vm, lex, channel, NULL);
}

BppError stmt_print_using_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    print_using_internal(vm, lex, -1);
    return err;
}
