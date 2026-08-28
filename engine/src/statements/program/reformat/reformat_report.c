// FILENAME: reformat_report.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (reformat_internal.h)
// Provides runtime implementation for the REFORMAT_REPORT statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/reformat_internal.h"

//
// ---- Check Report and Diff Preview Renderer ----

void reformat_render_check_report(VMContext *vm, const ReformatPlan *plan, bool save_requested) {
    (void)save_requested;
    if (!vm || !plan) return;

    VDevContext *vd = vm_get_vdev(vm);
    vdev_puts(vd, "REFORMAT CHECK: Analyzing program...\n\n");

    vdev_puts(vd, "=== DIFF PREVIEW ===\n");
    MemoryContext *mem = vm_get_mem(vm);
    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);

    int current_indent = 0;
    int spaces_per = plan->spaces_per_indent > 0 ? plan->spaces_per_indent : 2;
    int diff_shown = 0;

    for (size_t i = 0; i < line_count; i++) {
        double line_num = lines[i].line_number;
        const char *orig_text = lines[i].text ? lines[i].text : "";

        FastToken tokens[MAX_LINE_TOKENS];
        int tok_cnt = tokenize_line_fast(orig_text, tokens, MAX_LINE_TOKENS);

        int pre_adjust = 0;
        int post_adjust = 0;
        bool is_bounce = false;

        if (tok_cnt > 0) {
            BppKeywordId kw = tokens[0].kw;
            if (kw == KW_FOR || kw == KW_WHILE || kw == KW_DO || kw == KW_SELECT ||
                kw == KW_SUB || kw == KW_FUNCTION || kw == KW_TRY) {
                if (kw != KW_IF || is_block_if(tokens, tok_cnt)) post_adjust = 1;
            } else if (kw == KW_NEXT || kw == KW_WEND || kw == KW_LOOP) {
                pre_adjust = -1;
            } else if (kw == KW_END) {
                ReformatBlockType end_t = get_end_block_type(tokens, tok_cnt, 0);
                if (end_t != BLOCK_NONE) pre_adjust = -1;
            } else if (kw == KW_ELSE || kw == KW_CASE || kw == KW_CATCH) {
                is_bounce = true;
            }
        }

        current_indent += pre_adjust;
        if (current_indent < 0) current_indent = 0;

        int line_indent = is_bounce ? (current_indent > 0 ? current_indent - 1 : 0) : current_indent;
        if (tok_cnt > 0 && (is_label_line(tokens, tok_cnt) || ((tokens[0].kw == KW_REM || tokens[0].type == TOK_DOCSTRING) && current_indent == 0))) {
            line_indent = 0;
        }

        const char *body_text = skip_leading_ws(orig_text);
        char new_buf[2048];
        int is = line_indent * spaces_per;
        if (is < 0) is = 0;
        if (is > 128) is = 128;
        snprintf(new_buf, sizeof(new_buf), "%*s%.1024s", is, "", body_text);

        if (strcmp(orig_text, new_buf) != 0) {
            char buf[4200];
            snprintf(buf, sizeof(buf), "  Line %lld: [BEFORE] %lld %.1024s\n           [AFTER ] %lld %.1024s\n",
                     (long long)line_num, (long long)line_num, orig_text, (long long)line_num, new_buf);
            vdev_puts(vd, buf);
            diff_shown++;
        }

        current_indent += post_adjust;
        if (current_indent < 0) current_indent = 0;
    }

    if (diff_shown == 0) {
        vdev_puts(vd, "  (No formatting changes required)\n");
    }
    vdev_puts(vd, "\n");

    vdev_puts(vd, "=== DIAGNOSTICS ===\n");
    if (plan->diag_count == 0) {
        vdev_puts(vd, "  No structural errors or warnings found.\n");
    } else {
        for (int d = 0; d < plan->diag_count; d++) {
            const ReformatDiagnostic *diag = &plan->diagnostics[d];
            const char *sev_str = (diag->severity == DIAG_ERROR) ? "ERROR" : "WARNING";
            char header[512];
            snprintf(header, sizeof(header), "  [%-7s] Line %lld: %s\n", sev_str, (long long)diag->line, diag->what);
            vdev_puts(vd, header);

            char detail[1024];
            snprintf(detail, sizeof(detail), "            WHAT: %s\n            WHY:  %s\n            HOW:  %s\n\n",
                     diag->what, diag->why, diag->how);
            vdev_puts(vd, detail);
        }
        char count_buf[128];
        snprintf(count_buf, sizeof(count_buf), "  %d ERROR(s), %d WARNING(s)\n", plan->error_count, plan->warning_count);
        vdev_puts(vd, count_buf);
    }
    vdev_puts(vd, "\n");

    vdev_puts(vd, "=== SUMMARY ===\n");
    char sum_buf[512];
    snprintf(sum_buf, sizeof(sum_buf),
             "  Total lines:              %d\n"
             "  Lines to change:           %d\n"
             "  Lines unchanged:           %d\n"
             "  Indent width:               %d spaces\n\n"
             "  Diagnostics:\n"
             "    Errors:     %d\n"
             "    Warnings:   %d\n\n",
             plan->total_lines, diff_shown, plan->total_lines - diff_shown,
             plan->spaces_per_indent, plan->error_count, plan->warning_count);
    vdev_puts(vd, sum_buf);

    if (plan->error_count == 0 && plan->warning_count == 0) {
        vdev_puts(vd, "  STRICT mode ready:  YES\n  Recommend: REFORMAT ");
        char rec[32];
        snprintf(rec, sizeof(rec), "%d\n\n", plan->spaces_per_indent);
        vdev_puts(vd, rec);
    } else {
        char strict_buf[256];
        snprintf(strict_buf, sizeof(strict_buf),
                 "  STRICT mode ready:  NO\n"
                 "    %d error(s) and %d warning(s) must be resolved before STRICT will proceed.\n"
                 "    Tip: Fix the issues listed in DIAGNOSTICS, then run REFORMAT STRICT.\n\n",
                 plan->error_count, plan->warning_count);
        vdev_puts(vd, strict_buf);
    }

    if (plan->has_filename && strlen(plan->filename) > 0) {
        char chk_name[260];
        snprintf(chk_name, sizeof(chk_name), "%s", plan->filename);
        char *dot = strrchr(chk_name, '.');
        if (dot) *dot = '\0';
        strncat(chk_name, ".CHK", sizeof(chk_name) - strlen(chk_name) - 1);

        FILE *f = fopen(chk_name, "w");
        if (f) {
            fprintf(f, "REFORMAT CHECK Report for %s\n\n", plan->filename);
            fprintf(f, "Total Lines: %d, Diagnostics: %d Errors, %d Warnings\n",
                    plan->total_lines, plan->error_count, plan->warning_count);
            fprintf(f, "STRICT Ready: %s\n\n", (plan->error_count == 0 && plan->warning_count == 0) ? "YES" : "NO");
            for (int d = 0; d < plan->diag_count; d++) {
                fprintf(f, "[%s] Line %lld: %s\n  WHAT: %s\n  WHY:  %s\n  HOW:  %s\n\n",
                        plan->diagnostics[d].severity == DIAG_ERROR ? "ERROR" : "WARNING",
                        (long long)plan->diagnostics[d].line,
                        plan->diagnostics[d].what, plan->diagnostics[d].what,
                        plan->diagnostics[d].why, plan->diagnostics[d].how);
            }
            fclose(f);
            char report_msg[300];
            snprintf(report_msg, sizeof(report_msg), "  Report saved to: %s\n", chk_name);
            vdev_puts(vd, report_msg);
        } else {
            vdev_puts(vd, "  Report NOT saved (failed to open .CHK file for writing).\n");
        }
    } else {
        vdev_puts(vd, "  Report NOT saved (program has no filename — use SAVE first).\n");
    }
}

//
// ---- Suggestions Renderer and Metadata Registration ----

void reformat_render_suggestions_summary(VMContext *vm, const ReformatPlan *plan) {
    if (!vm || !plan) return;
    VDevContext *vd = vm_get_vdev(vm);
    if (plan->suggestion_count == 0) return;

    vdev_puts(vd, "=== SUGGESTIONS ===\n");
    for (int s = 0; s < plan->suggestion_count; s++) {
        const ReformatSuggestion *sug = &plan->suggestions[s];
        char buf[1024];
        snprintf(buf, sizeof(buf), "  %d. Line %lld: %s\n     -> %s\n",
                 s + 1, (long long)sug->line, sug->text, sug->reason);
        vdev_puts(vd, buf);
    }
    char count_buf[128];
    snprintf(count_buf, sizeof(count_buf), "\n  %d suggestion(s) for manual review.\n\n", plan->suggestion_count);
    vdev_puts(vd, count_buf);
}

void stmt_reformat_engine_register(void) {
    MicroLibMetadata meta = {
        .name = "REFORMAT ENGINE",
        .category = "Code Formatting",
        .syntax = "REFORMAT [line_start[-line_end]]",
        .help_text = "Reformats source code lines according to BASIC++ style and layout rules.",
        .error_codes = "Error 8: Undefined Line Number"
    };
    microlib_register(&meta);
}
