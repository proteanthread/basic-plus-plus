/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file reformat.c
 * @brief REFORMAT statement handler and micro-library registration for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements `stmt_reformat_handler()` statement handler and `stmt_reformat_register()` metadata registration.
 *
 * 2. WHY IT EXISTS:
 * Serves as the primary entry point for the REFORMAT command, parsing optional spacing parameters,
 * modifiers (CHECK, STRICT, UPPER, LOWER, PRESERVE, SPACES, SPLIT), and dispatching to the engine.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Ephemerally tokenizes statement parameters using LexerContext, executes pass 1 static analysis,
 * handles CHECK report rendering or STRICT mode enforcement, and executes pass 3 indentation.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'stmt_reformat'. Includes "statements/program/reformat.h",
 * "vm/vm.h", "lexer/lexer.h", "device/vdev.h", "runtime/micro_lib_metadata.h", <stdio.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in desktop ('baspp') and REPL ('bpp') editions per Rule #1.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add modifier keyword parsing for Phase 2/3 extensions (SPLIT, UPPER, LOWER, SPACES).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard BppError (*)(VMContext*, LexerContext*) signature and MicroLibMetadata registration call.
 *
 * 8. WHAT TO EXPECT:
 * Returns BPP_ERR_NONE on successful reformatting or CHECK execution; returns BPP_ERR_SYNTAX on STRICT abort.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check argument parsing logic and verify error return paths.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Bounded string operations per Rule #1.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/program/reformat_engine.c
 * - engine/src/device/vdev.c
 * Prerequisite Header Files:
 * - engine/include/statements/program/reformat.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "statements/program/reformat.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#define strncasecmp _strnicmp
#endif

BppError stmt_reformat_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) return err;

    int spaces = 2;
    bool spaces_set = false;
    ReformatModifier mod = MOD_NONE;
    double target_start = 0.0;
    double target_end = 0.0;

    while (1) {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_EOF || tok.type == TOK_EOL) break;

        if (tok.type == TOK_NUMBER) {
            double num = tok.as.number;
            BppToken next_tok = lex_peek(lex);
            if (next_tok.type == TOK_MINUS) {
                lex_next(lex); /* consume '-' */
                BppToken end_tok = lex_next(lex);
                if (end_tok.type == TOK_NUMBER) {
                    target_start = num;
                    target_end = end_tok.as.number;
                }
            } else if (spaces_set) {
                target_start = num;
                BppToken end_tok = lex_next(lex);
                if (end_tok.type == TOK_NUMBER) {
                    target_end = end_tok.as.number;
                }
            } else {
                spaces = (int)num;
                if (spaces < 0) spaces = 0;
                if (spaces > 16) spaces = 16;
                spaces_set = true;
            }
        } else if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
            const char *txt = tok.start;
            size_t len = tok.length;

            if (len == 5 && strncasecmp(txt, "CHECK", 5) == 0) {
                mod = MOD_CHECK;
            } else if (len == 6 && strncasecmp(txt, "STRICT", 6) == 0) {
                mod = MOD_STRICT;
            } else if (len == 5 && strncasecmp(txt, "SPLIT", 5) == 0) {
                mod = MOD_SPLIT;
            } else if (len == 5 && strncasecmp(txt, "UPPER", 5) == 0) {
                mod = MOD_UPPER;
            } else if (len == 5 && strncasecmp(txt, "LOWER", 5) == 0) {
                mod = MOD_LOWER;
            } else if (len == 8 && strncasecmp(txt, "PRESERVE", 8) == 0) {
                mod = MOD_PRESERVE;
            } else if (len == 6 && strncasecmp(txt, "SPACES", 6) == 0) {
                mod = MOD_SPACES;
            } else {
                char sub_name[128];
                snprintf(sub_name, sizeof(sub_name), "%.*s", (int)len, txt);
                reformat_find_sub_range(vm, sub_name, &target_start, &target_end);
            }
        }
    }

    ReformatPlan plan;
    reformat_plan_init(&plan, spaces);
    plan.target_start = target_start;
    plan.target_end = target_end;

    /* Pass 1 Analysis */
    reformat_pass1_analyze(vm, &plan);

    /* Handle CHECK mode */
    if (mod == MOD_CHECK) {
        reformat_render_check_report(vm, &plan, true);
        return err;
    }

    /* Handle STRICT mode enforcement */
    if (mod == MOD_STRICT) {
        if (plan.error_count > 0 || plan.warning_count > 0) {
            char buf[256];
            snprintf(buf, sizeof(buf),
                     "REFORMAT STRICT: Aborted due to %d error(s) and %d warning(s).\n"
                     "Run 'REFORMAT CHECK' to view detailed diagnostic report.\n",
                     plan.error_count, plan.warning_count);
            vdev_puts(vm_get_vdev(vm), buf);
            err.code = 2;
            err.category = ERR_CAT_SYNTAX;
            err.message = "REFORMAT STRICT blocked by diagnostics";
            return err;
        }
    }

    /* Pass 2 Blank REM Separators Insertion */
    reformat_pass2_blank_lines(vm, &plan);

    /* Pass SPLIT (Selective compound statement expansion) */
    if (mod == MOD_SPLIT) {
        reformat_pass_split(vm, &plan);
    }

    /* Pass 3 Indentation and cosmetic formatting */
    err = reformat_pass3_indent(vm, &plan, mod);
    if (err.code != 0) return err;

    char msg[256];
    snprintf(msg, sizeof(msg), "Reformatted %d line(s) (%d changed, %d unchanged).\n",
             plan.total_lines, plan.changed_lines, plan.unchanged_lines);
    vdev_puts(vm_get_vdev(vm), msg);

    if (mod == MOD_SPLIT || plan.suggestion_count > 0) {
        reformat_render_suggestions_summary(vm, &plan);
    }

    return err;
}

void stmt_reformat_register(void) {
    static const MicroLibMetadata meta = {
        .name = "REFORMAT",
        .category = "Program Mgmt & Editing",
        .syntax = "REFORMAT [spaces_per_indent] [target] [CHECK|STRICT|SPLIT|UPPER|LOWER|PRESERVE|SPACES]",
        .help_text = "Reformats BASIC source code with configurable indentation and static structural analysis.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
