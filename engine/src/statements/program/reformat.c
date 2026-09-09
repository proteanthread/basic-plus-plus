// FILENAME: reformat.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (reformat_internal.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, reformat.h, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the REFORMAT statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/reformat.h"
#include "statements/program/reformat_unpack.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_reformat_desc = {
    .name = "REFORMAT",
    .category = "Program Mgmt & Editing",
    .syntax = "REFORMAT [spaces_per_indent] [target] [CHECK|STRICT|SPLIT|UPPER|LOWER|PRESERVE|SPACES|UNPACK]",
    .description = "Reformats BASIC source code with configurable indentation, unpacking, and static structural analysis.",
    .error_summary = "Error 2: Syntax Error",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

#ifdef _MSC_VER
#define runtime_strncasecmp runtime_strncasecmp
#endif

#include "statements/program/modernize/semantic_harvester.h"

BppError stmt_reformat_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    if (!vm || !lex) return err;
    VDevContext *vd = vm_get_vdev(vm);

    // Handle dictionary map form: REFORMAT { unpack: true, ... } or REFORMAT { undo: true }
    if (lex_peek(lex).type == TOK_RPN_LITERAL) {
        BppToken r = lex_next(lex);
        if (runtime_strcasestr(r.start, "undo") || runtime_strcasestr(r.start, "revert")) {
            if (rambank_snapshot_pop(vm, "REFORMAT") || rambank_snapshot_pop(vm, NULL)) {
                vdev_puts(vd, "REFORMAT: Successfully undone previous reformat operations.\n");
            } else {
                vdev_puts(vd, "REFORMAT: No undo snapshot available in RAMbank.\n");
            }
            return err;
        }
        if (runtime_strcasestr(r.start, "expand_open") || runtime_strcasestr(r.start, "expandopen")) {
            rambank_snapshot_push(vm, "REFORMAT");
            int exp_cnt = reformat_expand_open_program(vm);
            vdev_printf(vd, "REFORMAT: Decompressed %d short-form OPEN statement(s) to canonical syntax.\n", exp_cnt);
            return err;
        }
        if (runtime_strcasestr(r.start, "unpack")) {
            rambank_snapshot_push(vm, "REFORMAT");
            int total_u = 0;
            int p = 1;
            for (; p <= 10; p++) {
                int u = reformat_unpack_program(vm);
                total_u += u;
                if (u == 0) break;
            }
            vdev_printf(vd, "REFORMAT: Unpacked %d packed line(s) across %d pass(es) (Converged).\n", total_u, p <= 10 ? p : 10);
            return err;
        }
    } else if (lex_peek(lex).type == TOK_LBRACE) {
        lex_next(lex);
        bool do_unpack = false;
        bool do_expand_open = false;
        bool do_undo = false;
        while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
            BppToken k = lex_next(lex);
            if (k.type == TOK_IDENT || k.type == TOK_KEYWORD || k.type == TOK_STRING) {
                if (runtime_strncasecmp(k.start, "undo", 4) == 0 || runtime_strncasecmp(k.start, "\"undo\"", 6) == 0) {
                    do_undo = true;
                } else if (runtime_strncasecmp(k.start, "unpack", 6) == 0 || runtime_strncasecmp(k.start, "\"unpack\"", 8) == 0) {
                    do_unpack = true;
                } else if (runtime_strcasestr(k.start, "expand_open") || runtime_strcasestr(k.start, "expandopen")) {
                    do_expand_open = true;
                }
            }
            if (lex_peek(lex).start && *lex_peek(lex).start == ':') lex_next(lex);
            if (lex_peek(lex).type == TOK_EOL) lex_next(lex);
            lex_next(lex); // Skip value
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
        if (do_undo) {
            if (rambank_snapshot_pop(vm, "REFORMAT") || rambank_snapshot_pop(vm, NULL)) {
                vdev_puts(vd, "REFORMAT: Successfully undone previous reformat operations.\n");
            } else {
                vdev_puts(vd, "REFORMAT: No undo snapshot available in RAMbank.\n");
            }
            return err;
        }
        if (do_expand_open) {
            rambank_snapshot_push(vm, "REFORMAT");
            int exp_cnt = reformat_expand_open_program(vm);
            vdev_printf(vd, "REFORMAT: Decompressed %d short-form OPEN statement(s) to canonical syntax.\n", exp_cnt);
            if (!do_unpack) return err;
        }
        if (do_unpack) {
            rambank_snapshot_push(vm, "REFORMAT");
            int total_u = 0;
            int p = 1;
            for (; p <= 10; p++) {
                int u = reformat_unpack_program(vm);
                total_u += u;
                if (u == 0) break;
            }
            vdev_printf(vd, "REFORMAT: Unpacked %d packed line(s) across %d pass(es) (Converged).\n", total_u, p <= 10 ? p : 10);
            return err;
        }
    }

    // Handle bracket list form: REFORMAT [ UNPACK ] or REFORMAT [ UNDO ] or REFORMAT [ EXPAND_OPEN ]
    if (lex_peek(lex).type == TOK_LBRACKET) {
        lex_next(lex);
        bool do_expand_open = false;
        bool do_unpack = false;
        while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) {
            BppToken btok = lex_next(lex);
            if (btok.type == TOK_IDENT || btok.type == TOK_KEYWORD) {
                if (runtime_strncasecmp(btok.start, "UNDO", 4) == 0 || runtime_strncasecmp(btok.start, "REVERT", 6) == 0) {
                    while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
                    if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
                    if (rambank_snapshot_pop(vm, "REFORMAT") || rambank_snapshot_pop(vm, NULL)) {
                        vdev_puts(vd, "REFORMAT: Successfully undone previous reformat operations.\n");
                    } else {
                        vdev_puts(vd, "REFORMAT: No undo snapshot available in RAMbank.\n");
                    }
                    return err;
                }
                if (runtime_strncasecmp(btok.start, "UNPACK", 6) == 0) {
                    do_unpack = true;
                } else if (runtime_strncasecmp(btok.start, "EXPAND_OPEN", 11) == 0 || runtime_strncasecmp(btok.start, "EXPANDOPEN", 10) == 0) {
                    do_expand_open = true;
                }
            }
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);

        if (do_expand_open) {
            rambank_snapshot_push(vm, "REFORMAT");
            int exp_cnt = reformat_expand_open_program(vm);
            vdev_printf(vd, "REFORMAT: Decompressed %d short-form OPEN statement(s) to canonical syntax.\n", exp_cnt);
            if (!do_unpack) return err;
        }
        if (do_unpack) {
            rambank_snapshot_push(vm, "REFORMAT");
            int total_u = 0;
            int p = 1;
            for (; p <= 10; p++) {
                int u = reformat_unpack_program(vm);
                total_u += u;
                if (u == 0) break;
            }
            vdev_printf(vd, "REFORMAT: Unpacked %d packed line(s) across %d pass(es) (Converged).\n", total_u, p <= 10 ? p : 10);
            return err;
        }
    }

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

            if (len == 5 && runtime_strncasecmp(txt, "CHECK", 5) == 0) {
                mod = MOD_CHECK;
            } else if (len == 6 && runtime_strncasecmp(txt, "STRICT", 6) == 0) {
                mod = MOD_STRICT;
            } else if (len == 5 && runtime_strncasecmp(txt, "SPLIT", 5) == 0) {
                mod = MOD_SPLIT;
            } else if (len == 5 && runtime_strncasecmp(txt, "UPPER", 5) == 0) {
                mod = MOD_UPPER;
            } else if (len == 5 && runtime_strncasecmp(txt, "LOWER", 5) == 0) {
                mod = MOD_LOWER;
            } else if (len == 8 && runtime_strncasecmp(txt, "PRESERVE", 8) == 0) {
                mod = MOD_PRESERVE;
            } else if ((len == 6 && runtime_strncasecmp(txt, "SPACES", 6) == 0) ||
                       (len == 5 && runtime_strncasecmp(txt, "SPACE", 5) == 0)) {
                mod = MOD_SPACES;
            } else if (len == 4 && runtime_strncasecmp(txt, "UNDO", 4) == 0) {
                if (rambank_snapshot_pop(vm, "REFORMAT") || rambank_snapshot_pop(vm, NULL)) {
                    vdev_puts(vd, "REFORMAT: Successfully undone previous reformat operations.\n");
                } else {
                    vdev_puts(vd, "REFORMAT: No undo snapshot available in RAMbank.\n");
                }
                return err;
            } else if (len == 6 && runtime_strncasecmp(txt, "UNPACK", 6) == 0) {
                mod = MOD_UNPACK;
            } else {
                char sub_name[128];
                runtime_snprintf(sub_name, sizeof(sub_name), "%.*s", (int)len, txt);
                reformat_find_sub_range(vm, sub_name, &target_start, &target_end);
            }
        }
    }

    int max_passes = 10;

    if (mod == MOD_UNPACK) {
        rambank_snapshot_push(vm, "REFORMAT");
        int total_u = 0;
        int p = 1;
        for (; p <= max_passes; p++) {
            int u = reformat_unpack_program(vm);
            total_u += u;
            if (u == 0) break;
        }

        // Post-pass syntax validation for UNPACK
        bool syntax_ok = true;
        BppLineNumber err_line = 0;
        MemoryContext *mem_chk = vm_get_mem(vm);
        size_t chk_count = 0;
        BppProgramLine *chk_lines = mem_program_get_all(mem_chk, &chk_count);
        for (size_t l = 0; l < chk_count; l++) {
            const char *lt = chk_lines[l].text;
            if (!lt) continue;
            bool in_q = false;
            int parens = 0;
            for (const char *cp = lt; *cp; cp++) {
                if (*cp == '"') in_q = !in_q;
                else if (!in_q) {
                    if (*cp == '\'' || (runtime_strncasecmp(cp, "REM", 3) == 0 && (cp == lt || runtime_isspace((unsigned char)*(cp - 1))))) break;
                    if (*cp == '(') parens++;
                    else if (*cp == ')') { parens--; if (parens < 0) break; }
                }
            }
            if (in_q || parens != 0) { syntax_ok = false; err_line = chk_lines[l].line_number; break; }
        }
        if (!syntax_ok) {
            vdev_printf(vm_get_vdev(vm), "REFORMAT UNPACK: Syntax integrity failed on line %lld - automatically rolled back to snapshot.\n", (long long)err_line);
            rambank_snapshot_pop(vm, "REFORMAT");
            err.code = 2;
            err.message = "REFORMAT UNPACK syntax integrity violation; restored from RAMbank";
            return err;
        }

        char ubuf[128];
        runtime_snprintf(ubuf, sizeof(ubuf), "REFORMAT UNPACK: Decompressed %d packed line(s) across %d pass(es) (Converged).\n", total_u, p <= max_passes ? p : max_passes);
        vdev_puts(vm_get_vdev(vm), ubuf);
        return err;
    }

    rambank_snapshot_push(vm, "REFORMAT");

    // Pass 0: Decompress / Unpack vintage space-less lines by default unless PRESERVE or CHECK
    int total_unpacked = 0;
    if (mod != MOD_PRESERVE && mod != MOD_CHECK) {
        for (int p = 1; p <= max_passes; p++) {
            int u = reformat_unpack_program(vm);
            total_unpacked += u;
            if (u == 0) break;
        }
        if (mod == MOD_SPACES) {
            char ubuf[128];
            runtime_snprintf(ubuf, sizeof(ubuf), "REFORMAT SPACES: Formatted %d packed line(s) with canonical spacing.\n", total_unpacked);
            vdev_puts(vm_get_vdev(vm), ubuf);
        }
    }

    int format_passes = 0;
    int cumulative_changed = total_unpacked;
    int last_total_lines = 0;
    int last_suggestions = 0;

    for (int p = 1; p <= max_passes; p++) {
        format_passes = p;
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
                runtime_snprintf(buf, sizeof(buf),
                         "REFORMAT STRICT: Aborted due to %d error(s) and %d warning(s).\n"
                         "Run 'REFORMAT CHECK' to view detailed diagnostic report.\n",
                         plan.error_count, plan.warning_count);
                vdev_puts(vm_get_vdev(vm), buf);
                rambank_snapshot_pop(vm, "REFORMAT");
                err.code = 2;
                err.category = ERR_CAT_SYNTAX;
                err.message = "REFORMAT STRICT blocked by diagnostics";
                return err;
            }
        }

        // Pass 2 Blank REM Separators Insertion
        if (p == 1) {
            reformat_pass2_blank_lines(vm, &plan);
            if (mod == MOD_SPLIT) {
                reformat_pass_split(vm, &plan);
            }
        }

        // Pass 3 Indentation and cosmetic formatting
        err = reformat_pass3_indent(vm, &plan, mod);
        if (err.code != 0) {
            rambank_snapshot_pop(vm, "REFORMAT");
            return err;
        }

        cumulative_changed += plan.changed_lines;
        last_total_lines = plan.total_lines;
        last_suggestions = plan.suggestion_count;

        if (plan.changed_lines == 0) {
            break; // Converged
        }
    }

    // Post-pass syntax & paren/quote balance audit
    bool syntax_ok = true;
    BppLineNumber err_line = 0;
    MemoryContext *mem_chk = vm_get_mem(vm);
    size_t chk_count = 0;
    BppProgramLine *chk_lines = mem_program_get_all(mem_chk, &chk_count);
    for (size_t l = 0; l < chk_count; l++) {
        const char *lt = chk_lines[l].text;
        if (!lt) continue;
        bool in_q = false;
        int parens = 0;
        for (const char *cp = lt; *cp; cp++) {
            if (*cp == '"') in_q = !in_q;
            else if (!in_q) {
                if (*cp == '\'' || (runtime_strncasecmp(cp, "REM", 3) == 0 && (cp == lt || runtime_isspace((unsigned char)*(cp - 1))))) break;
                if (*cp == '(') parens++;
                else if (*cp == ')') { parens--; if (parens < 0) break; }
            }
        }
        if (in_q || parens != 0) { syntax_ok = false; err_line = chk_lines[l].line_number; break; }
    }
    if (!syntax_ok) {
        vdev_printf(vm_get_vdev(vm), "REFORMAT: Syntax integrity validation failed on line %lld - automatically rolled back to snapshot.\n", (long long)err_line);
        rambank_snapshot_pop(vm, "REFORMAT");
        err.code = 2;
        err.message = "REFORMAT syntax integrity violation; restored from RAMbank";
        return err;
    }

    int total_changed = cumulative_changed;
    if (total_changed > last_total_lines) total_changed = last_total_lines;
    int total_unchanged = (last_total_lines > total_changed) ? (last_total_lines - total_changed) : 0;

    char msg[256];
    runtime_snprintf(msg, sizeof(msg), "Reformatted %d line(s) (%d changed, %d unchanged) across %d pass(es) (Converged).\n",
             last_total_lines, total_changed, total_unchanged, format_passes);
    vdev_puts(vm_get_vdev(vm), msg);

    if (mod == MOD_SPLIT || last_suggestions > 0) {
        ReformatPlan dummy_plan;
        reformat_plan_init(&dummy_plan, spaces);
        dummy_plan.suggestion_count = last_suggestions;
        reformat_render_suggestions_summary(vm, &dummy_plan);
    }

    return err;
}

BValue func_reformat_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    ReformatPlan plan;
    reformat_plan_init(&plan, 2);
    reformat_pass1_analyze(vm, &plan);
    reformat_pass2_blank_lines(vm, &plan);
    *err = reformat_pass3_indent(vm, &plan, MOD_NONE);
    res.as.number = (double)plan.changed_lines;
    return res;
}

void stmt_reformat_register(void) {
    lang_desc_register(&g_reformat_desc);
}
