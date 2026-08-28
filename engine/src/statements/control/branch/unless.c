// FILENAME: unless.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, map.h, map.c, string.c, unless.h, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the UNLESS statement in BASIC++.
//
// ---- Includes ----

#include "statements/control/branch/unless.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/map.h"
#include "memory/memory.h"
#include "types/errors.h"
#include <string.h>
#include <stdbool.h>

void stmt_unless_register(void) {
    static const MicroLibMetadata meta = {
        .name = "UNLESS",
        .category = "Control Flow",
        .syntax = "UNLESS expr [THEN] stmt/line [ELSE stmt/line]",
        .help_text = "Evaluates boolean expression and executes THEN clause if false, ELSE clause if true (DEC / Tymshare).",
        .error_codes = "Error 2: Syntax error, Error 8: Undefined line number"
    };
    microlib_register(&meta);
}

static BppError dispatch_branch(VMContext *vm, LexerContext *lex) {
    extern BppError execute_single_statement(VMContext *vm, LexerContext *lex);
    return execute_single_statement(vm, lex);
}

BppError stmt_unless_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Evaluate condition
    BValue cond = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    if (cond.type == VAL_STRING) {
        if (cond.as.string) str_release(vm_get_str(vm), cond.as.string);
        err.code = ERR_TYPE_MISMATCH;
        err.message = "UNLESS condition cannot be a string";
        return err;
    } else if (cond.type == VAL_MAP) {
        if (cond.as.map) map_release(vm_get_str(vm), cond.as.map);
        err.code = ERR_TYPE_MISMATCH;
        err.message = "UNLESS condition cannot be an object";
        return err;
    }

    // Optional THEN
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_THEN) {
        lex_next(lex);
    } else if (tok.type == TOK_IDENT && tok.length == 4 && strncasecmp(tok.start, "THEN", 4) == 0) {
        lex_next(lex);
    }

    bool is_false = (cond.as.number == 0.0);

    if (is_false) {
        // Parse and execute THEN branch statement(s)
        BppToken next = lex_peek(lex);
        if (next.type == TOK_NUMBER) {
            // Implicit GOTO
            lex_next(lex);
            BppLineNumber target = next.as.number;
            if (!mem_program_get(vm_get_mem(vm), target)) {
                err.code = ERR_UNDEFINED_LINE;
                err.message = "Undefined line number in UNLESS branch";
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
            if (next_tok.type == TOK_EOL && next_tok.start && *next_tok.start == ':') {
                lex_next(lex); // Consume colon
                BppToken after_colon = lex_peek(lex);
                if (after_colon.type == TOK_KEYWORD && after_colon.as.keyword == KW_ELSE) {
                    break;
                }
            } else {
                break;
            }
        }

        // Skip trailing ELSE branch if it exists
        BppToken skip = lex_peek(lex);
        if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
            while (skip.type != TOK_EOF && (skip.type != TOK_EOL || (skip.start && *skip.start == ':'))) {
                lex_next(lex);
                skip = lex_peek(lex);
            }
        }
    } else {
        // Condition is true: skip to ELSE or EOL
        BppToken skip = lex_peek(lex);
        while (skip.type != TOK_EOF && (skip.type != TOK_EOL || (skip.start && *skip.start == ':'))) {
            if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
                break;
            }
            lex_next(lex);
            skip = lex_peek(lex);
        }

        if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
            lex_next(lex); // Consume ELSE

            // Check if statement following ELSE is a line number (implicit GOTO)
            BppToken next = lex_peek(lex);
            if (next.type == TOK_NUMBER) {
                lex_next(lex);
                BppLineNumber target = next.as.number;
                if (!mem_program_get(vm_get_mem(vm), target)) {
                    err.code = ERR_UNDEFINED_LINE;
                    err.message = "Undefined line number in UNLESS ELSE branch";
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
                if (next_tok.type == TOK_EOL && next_tok.start && *next_tok.start == ':') {
                    lex_next(lex); // Consume colon
                } else {
                    break;
                }
            }
        }
    }

    return err;
}
