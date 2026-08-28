// FILENAME: if.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (eval.h, eval.c, if.h, string.c, vm.h)
// Provides runtime implementation for the IF statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/if.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "memory/memory.h"
#include <string.h>
#include <stdbool.h>

void stmt_if_register(void) {
    MicroLibMetadata meta = {
        .name = "IF",
        .category = "Control Flow",
        .syntax = "IF expr THEN stmt/line [ELSE stmt/line]",
        .help_text = "Evaluates boolean expression expr and executes THEN clause if true, ELSE clause if false.",
        .error_codes = "Error 2: Syntax error (missing THEN or malformed expression)"
    };
    microlib_register(&meta);
}

static inline bool is_tok_then(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_THEN) ||
           (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "THEN", 4) == 0);
}

static inline bool is_tok_else(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE) ||
           (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "ELSE", 4) == 0);
}

static inline bool is_tok_goto(BppToken tok) {
    return (tok.type == TOK_KEYWORD && tok.as.keyword == KW_GOTO) ||
           (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "GOTO", 4) == 0);
}

static BppError dispatch_branch(VMContext *vm, LexerContext *lex) {
    extern BppError execute_single_statement(VMContext *vm, LexerContext *lex);
    return execute_single_statement(vm, lex);
}

BppError stmt_if_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Check for HP TSB: IF END #channel THEN line/stmt
    BppToken peek = lex_peek(lex);
    bool is_end = (peek.type == TOK_KEYWORD && peek.as.keyword == KW_END) ||
                  (peek.type == TOK_IDENT && peek.length == 3 && strncasecmp(peek.start, "END", 3) == 0);
    if (is_end) {
        LexerContext *look = lex_init(vm_get_mem(vm), lex_get_pos(lex));
        lex_next(look); // consume END
        BppToken after = lex_peek(look);
        bool has_hash = (after.type == TOK_HASH);
        lex_shutdown(look);

        if (has_hash) {
            lex_next(lex); // consume END
            lex_next(lex); // consume #
            BValue ch_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;
            int ch = (int)ch_val.as.number;
            BppToken tok_then = lex_next(lex);
            if (!is_tok_then(tok_then)) {
                err.code = 2; err.message = "Expected THEN after IF END #channel";
                return err;
            }
            bool is_eof = file_eof(vm_get_file(vm), ch);
            if (is_eof) {
                BppToken next = lex_peek(lex);
                if (next.type == TOK_NUMBER) {
                    lex_next(lex);
                    BppLineNumber target = next.as.number;
                    if (!mem_program_get(vm_get_mem(vm), target)) {
                        err.code = 8; err.message = "Undefined line number in IF branch";
                        return err;
                    }
                    vm_jump(vm, target, NULL);
                    return err;
                }
                return dispatch_branch(vm, lex);
            } else {
                return err;
            }
        }
    }

    // Evaluate condition
    BValue cond = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    if (cond.type == VAL_STRING) {
        err.code = 13; // Type mismatch
        err.message = "IF condition cannot be a string";
        return err;
    }

    // Expect THEN or vintage GOTO or direct line number (GW-BASIC / BASICA)
    BppToken tok = lex_peek(lex);
    if (is_tok_then(tok)) {
        lex_next(lex); // Consume THEN
    } else if (is_tok_goto(tok) || tok.type == TOK_NUMBER) {
        // Vintage IF cond GOTO line or IF cond line: allowed
    } else {
        err.code = 2; // Syntax error
        err.message = "Expected THEN after IF condition";
        return err;
    }

    bool is_true = (cond.as.number != 0.0);

    if (is_true) {
        // Parse and execute THEN branch statement
        BppToken next = lex_peek(lex);
        if (next.type == TOK_NUMBER) {
            // Implicit GOTO
            lex_next(lex);
            BppLineNumber target = next.as.number;
            if (!mem_program_get(vm_get_mem(vm), target)) {
                err.code = 8;
                err.message = "Undefined line number in IF branch";
                return err;
            }
            vm_jump(vm, target, NULL);
            return err;
        } else if (is_tok_goto(next)) {
            lex_next(lex); // Consume GOTO
            BppToken target_tok = lex_next(lex);
            if (target_tok.type != TOK_NUMBER) {
                err.code = 2;
                err.message = "Expected line number after GOTO";
                return err;
            }
            BppLineNumber target = target_tok.as.number;
            if (!mem_program_get(vm_get_mem(vm), target)) {
                err.code = 8;
                err.message = "Undefined line number in IF branch";
                return err;
            }
            vm_jump(vm, target, NULL);
            return err;
        }

        // Execute all statements in the THEN branch (separated by colons) until ELSE or EOL
        while (true) {
            err = dispatch_branch(vm, lex);
            if (err.code != 0 || vm_is_jump_active(vm)) {
                return err;
            }
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                lex_next(lex); // Consume colon
                BppToken after_colon = lex_peek(lex);
                if (is_tok_else(after_colon)) {
                    break;
                }
            } else {
                break;
            }
        }

        // Skip trailing ELSE branch if it exists
        BppToken skip = lex_peek(lex);
        if (is_tok_else(skip)) {
            while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
                lex_next(lex);
                skip = lex_peek(lex);
            }
        }
    } else {
        // Condition is false: skip to ELSE or EOL
        BppToken skip = lex_peek(lex);
        while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
            if (is_tok_else(skip)) {
                break;
            }
            lex_next(lex);
            skip = lex_peek(lex);
        }

        if (is_tok_else(skip)) {
            lex_next(lex); // Consume ELSE

            // Check if statement following ELSE is a line number (implicit GOTO)
            BppToken next = lex_peek(lex);
            if (next.type == TOK_NUMBER) {
                lex_next(lex);
                BppLineNumber target = next.as.number;
                if (!mem_program_get(vm_get_mem(vm), target)) {
                    err.code = 8;
                    err.message = "Undefined line number in ELSE branch";
                    return err;
                }
                vm_jump(vm, target, NULL);
                return err;
            } else if (is_tok_goto(next)) {
                lex_next(lex); // Consume GOTO
                BppToken target_tok = lex_next(lex);
                if (target_tok.type != TOK_NUMBER) {
                    err.code = 2;
                    err.message = "Expected line number after GOTO";
                    return err;
                }
                BppLineNumber target = target_tok.as.number;
                if (!mem_program_get(vm_get_mem(vm), target)) {
                    err.code = 8;
                    err.message = "Undefined line number in ELSE branch";
                    return err;
                }
                vm_jump(vm, target, NULL);
                return err;
            }

            // Execute all statements in the ELSE branch (separated by colons)
            while (true) {
                err = dispatch_branch(vm, lex);
                if (err.code != 0 || vm_is_jump_active(vm)) {
                    return err;
                }
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                    lex_next(lex); // Consume colon
                } else {
                    break;
                }
            }
        }
    }

    return err;
}
