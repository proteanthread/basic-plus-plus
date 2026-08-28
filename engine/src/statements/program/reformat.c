// FILENAME: reformat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (reformat_internal.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, reformat.h, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the REFORMAT statement in BASIC++.
//
// ---- Includes ----

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
                lex_next(lex); // consume '-'
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

    // Pass 1 Analysis
    reformat_pass1_analyze(vm, &plan);

    // Handle CHECK mode
    if (mod == MOD_CHECK) {
        reformat_render_check_report(vm, &plan, true);
        return err;
    }

    // Handle STRICT mode enforcement
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

    // Pass 2 Blank REM Separators Insertion
    reformat_pass2_blank_lines(vm, &plan);

    // Pass SPLIT (Selective compound statement expansion)
    if (mod == MOD_SPLIT) {
        reformat_pass_split(vm, &plan);
    }

    // Pass 3 Indentation and cosmetic formatting
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
