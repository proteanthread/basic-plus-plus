/**
 * @file if.c
 * @brief IF/THEN/ELSE conditional branching statement handler for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Evaluates conditional expressions and controls statement branch execution for single-line
 * IF/THEN/ELSE constructs, line number jumps (IF x THEN 100), and trailing postfix modifiers.
 *
 * 2. WHY IT EXISTS:
 * Serves as the core conditional branching micro-library across desktop, lite REPL, and script runners.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Evaluates the conditional expression via eval_expression(). If non-zero (true), executes the THEN
 * clause (or performs a GOTO line jump if followed by a line number integer). If zero (false), skips
 * to the optional ELSE clause and executes or jumps accordingly.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_if'. Directly includes "statements/core/if.h",
 * "eval/eval.h", "vm/vm.h", and "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * To support multi-line block IF/ELSEIF/END IF constructs, extend line token parsing to push block IF
 * stack frames onto the interpreter stack.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Trailing postfix condition evaluation order (multiple trailing IF modifiers evaluate right-to-left).
 * Non-recursive VM stack invariants.
 *
 * 8. WHAT TO EXPECT:
 * Returns ERR_NONE on successful branch execution, ERR_SYNTAX on missing THEN/GOTO, or ERR_TYPE_MISMATCH.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check expression evaluation truthiness rules. Verify lexer token cursor after THEN and ELSE keywords.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Lexer cursor is positioned immediately past the IF token. Valid VMContext.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Floating-point non-zero truth check (val != 0.0).
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/eval.c
 * - engine/src/statements/core/goto.c
 * Prerequisite Header Files:
 * - engine/include/statements/core/if.h
 * - engine/include/eval/eval.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#include "statements/core/if.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "eval/eval.h"
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

static BppError dispatch_branch(VMContext *vm, LexerContext *lex) {
    extern BppError execute_single_statement(VMContext *vm, LexerContext *lex);
    return execute_single_statement(vm, lex);
}

BppError stmt_if_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Evaluate condition */
    BValue cond = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    if (cond.type == VAL_STRING) {
        err.code = 13; /* Type mismatch */
        err.message = "IF condition cannot be a string";
        return err;
    }

    /* Expect THEN */
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_KEYWORD || tok.as.keyword != KW_THEN) {
        err.code = 2; /* Syntax error */
        err.message = "Expected THEN after IF condition";
        return err;
    }

    bool is_true = (cond.as.number != 0.0);

    if (is_true) {
        /* Parse and execute THEN branch statement */
        BppToken next = lex_peek(lex);
        if (next.type == TOK_NUMBER) {
            /* Implicit GOTO */
            lex_next(lex);
            BppLineNumber target = next.as.number;
            if (!mem_program_get(vm_get_mem(vm), target)) {
                err.code = 8;
                err.message = "Undefined line number in IF branch";
                return err;
            }
            vm_jump(vm, target, NULL);
            return err;
        }

        /* Execute all statements in the THEN branch (separated by colons) until ELSE or EOL */
        while (true) {
            err = dispatch_branch(vm, lex);
            if (err.code != 0 || vm_is_jump_active(vm)) {
                return err;
            }
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                lex_next(lex); /* Consume colon */
                BppToken after_colon = lex_peek(lex);
                if (after_colon.type == TOK_KEYWORD && after_colon.as.keyword == KW_ELSE) {
                    break;
                }
            } else {
                break;
            }
        }

        /* Skip trailing ELSE branch if it exists */
        BppToken skip = lex_peek(lex);
        if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
            while (skip.type != TOK_EOF && skip.type != TOK_EOL) {
                lex_next(lex);
                skip = lex_peek(lex);
            }
        }
    } else {
        /* Condition is false: skip to ELSE or EOL */
        BppToken skip = lex_peek(lex);
        while (skip.type != TOK_EOF && (skip.type != TOK_EOL || *skip.start == ':')) {
            if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
                break;
            }
            lex_next(lex);
            skip = lex_peek(lex);
        }

        if (skip.type == TOK_KEYWORD && skip.as.keyword == KW_ELSE) {
            lex_next(lex); /* Consume ELSE */

            /* Check if statement following ELSE is a line number (implicit GOTO) */
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
            }

            /* Execute all statements in the ELSE branch (separated by colons) */
            while (true) {
                err = dispatch_branch(vm, lex);
                if (err.code != 0 || vm_is_jump_active(vm)) {
                    return err;
                }
                BppToken next_tok = lex_peek(lex);
                if (next_tok.type == TOK_EOL && *next_tok.start == ':') {
                    lex_next(lex); /* Consume colon */
                } else {
                    break;
                }
            }
        }
    }

    return err;
}
