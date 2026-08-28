// FILENAME: analyzer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (check.c, verify.c)
// NEEDS: libcore (analyzer.h, ctype.h, ctype.c, memops.h, memops.c)
// NEEDS: libcore (memory.h, memory.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (lexer.h, lexer.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides core logic and interface definitions for analyzer within BASIC++.
//
// ---- Includes ----

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "debug/analyzer.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/ctype/ctype.h"

static void add_diag(StaticAnalysisReport *rep, BppLineNumber line, const char *severity, const char *code, const char *msg) {
    if (!rep || rep->diagnostic_count >= ANALYZER_MAX_DIAGNOSTICS) return;
    AnalyzerDiagnostic *d = &rep->diagnostics[rep->diagnostic_count++];
    d->line = line;
    runtime_snprintf(d->severity, sizeof(d->severity), "%s", severity ? severity : "");
    runtime_snprintf(d->code, sizeof(d->code), "%s", code ? code : "");
    runtime_snprintf(d->message, sizeof(d->message), "%s", msg ? msg : "");
}

static bool str_contains_kw(const char *text, const char *kw) {
    if (!text || !kw) return false;
    char buf[512];
    runtime_strncpy(buf, text, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    for (char *p = buf; *p; p++) *p = (char)runtime_toupper((unsigned char)*p);
    return runtime_strstr(buf, kw) != NULL;
}

void analyzer_run(MemoryContext *mem, StaticAnalysisReport *out_report) {
    if (!out_report) return;
    runtime_memset(out_report, 0, sizeof(*out_report));
    if (!mem) return;

    const char *ver_tag = mem_program_get_version(mem);
    if (ver_tag && ver_tag[0]) {
        runtime_strncpy(out_report->program_version, ver_tag, sizeof(out_report->program_version) - 1);
    } else {
        runtime_strncpy(out_report->program_version, "UNTAGGED", sizeof(out_report->program_version) - 1);
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    out_report->total_lines = count;

    bool after_unconditional = false;

    for (size_t i = 0; i < count; i++) {
        BppLineNumber lnum = lines[i].line_number;
        const char *txt = lines[i].text ? lines[i].text : "";

        // Skip leading line number
        const char *p = txt;
        while (*p && (runtime_isdigit((unsigned char)*p) || runtime_isspace((unsigned char)*p))) p++;

        if (*p == '\0') continue;

        // Comment check
        if (*p == '\'' || runtime_strncasecmp(p, "REM", 3) == 0) {
            out_report->comment_lines++;
            continue;
        }

        out_report->code_lines++;

        // Check unreachable code
        if (after_unconditional) {
            if (*p != ':' && !str_contains_kw(p, "SUB") && !str_contains_kw(p, "FUNCTION")) {
                out_report->unreachable_lines++;
                add_diag(out_report, lnum, "WARNING", "WARN_DEAD_CODE", "Unreachable code following unconditional control statement");
            }
            after_unconditional = false;
        }

        // Keyword counts
        if (str_contains_kw(p, "FOR"))  out_report->for_count++;
        if (str_contains_kw(p, "NEXT")) out_report->next_count++;
        if (str_contains_kw(p, "WHILE")) out_report->while_count++;
        if (str_contains_kw(p, "WEND")) out_report->wend_count++;
        if (str_contains_kw(p, "DO"))    out_report->do_count++;
        if (str_contains_kw(p, "LOOP"))  out_report->loop_count++;
        if (str_contains_kw(p, "IF"))    out_report->if_count++;
        if (str_contains_kw(p, "SUB"))   out_report->sub_count++;
        if (str_contains_kw(p, "FUNCTION")) out_report->func_count++;
        if (str_contains_kw(p, "GOSUB")) out_report->gosub_calls++;
        if (str_contains_kw(p, "DATA"))  out_report->data_items++;
        if (str_contains_kw(p, "READ"))  out_report->read_vars++;

        if (str_contains_kw(p, "GOTO") || str_contains_kw(p, "RETURN") || str_contains_kw(p, "END")) {
            after_unconditional = true;
        }
    }

    // Mismatch diagnostic checks
    if (out_report->for_count != out_report->next_count) {
        add_diag(out_report, 0, "WARNING", "WARN_MISMATCH", "Mismatched FOR/NEXT loop structures");
    }
    if (out_report->while_count != out_report->wend_count) {
        add_diag(out_report, 0, "WARNING", "WARN_MISMATCH", "Mismatched WHILE/WEND loop structures");
    }
    if (out_report->do_count != out_report->loop_count) {
        add_diag(out_report, 0, "WARNING", "WARN_MISMATCH", "Mismatched DO/LOOP loop structures");
    }
    if (out_report->read_vars > out_report->data_items) {
        add_diag(out_report, 0, "WARNING", "WARN_DATA_PARITY", "READ variable count exceeds DATA items");
    }

    // Cyclomatic Complexity metric
    out_report->cyclomatic_complexity = 1 + out_report->if_count + out_report->for_count + out_report->while_count + out_report->do_count;
}

void analyzer_render_summary(VDevContext *vdev, const StaticAnalysisReport *report) {
    if (!report) return;

    vdev_printf(vdev, "=== BASIC++ STATIC ANALYSIS REPORT ===\n");
    vdev_printf(vdev, "Program Version Tag: %s\n\n", report->program_version);

    vdev_printf(vdev, "--- Program Statistics ---\n");
    vdev_printf(vdev, "  Total Lines:      %zu (%zu code, %zu comments)\n",
                report->total_lines, report->code_lines, report->comment_lines);
    vdev_printf(vdev, "  Control Structures: FOR=%d NEXT=%d WHILE=%d WEND=%d DO=%d LOOP=%d IF=%d\n",
                report->for_count, report->next_count, report->while_count,
                report->wend_count, report->do_count, report->loop_count, report->if_count);
    vdev_printf(vdev, "  Procedures:       %d SUBs, %d FUNCTIONs, %d GOSUB calls\n",
                report->sub_count, report->func_count, report->gosub_calls);
    vdev_printf(vdev, "  DATA / READ:      %d DATA items, %d READ variables\n",
                report->data_items, report->read_vars);

    vdev_printf(vdev, "\n--- Complexity Metrics ---\n");
    const char *complexity_desc = "simple";
    if (report->cyclomatic_complexity > 50) complexity_desc = "extremely complex";
    else if (report->cyclomatic_complexity > 20) complexity_desc = "complex";
    else if (report->cyclomatic_complexity > 10) complexity_desc = "moderate";
    vdev_printf(vdev, "  Cyclomatic Complexity: %d (%s)\n", report->cyclomatic_complexity, complexity_desc);

    vdev_printf(vdev, "\n--- Diagnostics (%zu issues) ---\n", report->diagnostic_count);
    if (report->diagnostic_count == 0) {
        vdev_printf(vdev, "  [PASS] No structural errors or quality warnings detected.\n");
    } else {
        for (size_t i = 0; i < report->diagnostic_count; i++) {
            const AnalyzerDiagnostic *d = &report->diagnostics[i];
            if (d->line > 0) {
                vdev_printf(vdev, "  %s (Line %lld) [%s]: %s\n",
                            d->severity, (long long)d->line, d->code, d->message);
            } else {
                vdev_printf(vdev, "  %s [%s]: %s\n",
                            d->severity, d->code, d->message);
            }
        }
    }
    vdev_printf(vdev, "\n");
}

void analyzer_render_detailed(VDevContext *vdev, MemoryContext *mem, const StaticAnalysisReport *report) {
    analyzer_render_summary(vdev, report);

    if (!mem) return;
    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);

    vdev_printf(vdev, "=== DETAILED LINE LISTING ===\n");
    int indent = 0;
    for (size_t i = 0; i < count; i++) {
        BppLineNumber lnum = lines[i].line_number;
        const char *txt = lines[i].text ? lines[i].text : "";

        if (str_contains_kw(txt, "NEXT") || str_contains_kw(txt, "WEND") || str_contains_kw(txt, "LOOP") || str_contains_kw(txt, "END SUB") || str_contains_kw(txt, "END FUNCTION")) {
            if (indent > 0) indent--;
        }

        vdev_printf(vdev, "%5lld | %*s%s\n", (long long)lnum, indent * 2, "", txt);

        if (str_contains_kw(txt, "FOR") || str_contains_kw(txt, "WHILE") || str_contains_kw(txt, "DO") || str_contains_kw(txt, "SUB") || str_contains_kw(txt, "FUNCTION")) {
            indent++;
        }
    }
    vdev_printf(vdev, "\n");
}

void analyzer_render_json(VDevContext *vdev, const StaticAnalysisReport *report) {
    if (!report) return;

    vdev_printf(vdev, "{\n");
    vdev_printf(vdev, "  \"program_version\": \"%s\",\n", report->program_version);
    vdev_printf(vdev, "  \"statistics\": {\n");
    vdev_printf(vdev, "    \"total_lines\": %zu,\n", report->total_lines);
    vdev_printf(vdev, "    \"code_lines\": %zu,\n", report->code_lines);
    vdev_printf(vdev, "    \"comment_lines\": %zu,\n", report->comment_lines);
    vdev_printf(vdev, "    \"cyclomatic_complexity\": %d\n", report->cyclomatic_complexity);
    vdev_printf(vdev, "  },\n");
    vdev_printf(vdev, "  \"diagnostics_count\": %zu,\n", report->diagnostic_count);
    vdev_printf(vdev, "  \"diagnostics\": [\n");
    for (size_t i = 0; i < report->diagnostic_count; i++) {
        const AnalyzerDiagnostic *d = &report->diagnostics[i];
        vdev_printf(vdev, "    {\"line\": %lld, \"severity\": \"%s\", \"code\": \"%s\", \"message\": \"%s\"}%s\n",
                    (long long)d->line, d->severity, d->code, d->message,
                    (i + 1 < report->diagnostic_count) ? "," : "");
    }
    vdev_printf(vdev, "  ]\n");
    vdev_printf(vdev, "}\n");
}
