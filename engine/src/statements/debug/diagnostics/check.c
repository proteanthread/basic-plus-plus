// FILENAME: check.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (analyzer.h, analyzer.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (check.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the CHECK statement in BASIC++.

#include "statements/debug/diagnostics/check.h"
#include "debug/analyzer.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/language_descriptor.h"
#include "runtime/funcreg.h"
#include "runtime/format/snprintf.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"
#include "runtime/conv/num_parse.h"
#include "statements/program/modernize/semantic_harvester.h"

static const LangDesc g_check_desc = {
    .name = "CHECK",
    .category = "Debug & Testing",
    .syntax = "CHECK [file$] [SUMMARY|DETAILED|JSON|CONFLICTS|SMART]",
    .description = "Executes 7-stage code health audit, keyword-variable conflict detection, and semantic inference.",
    .error_summary = "Error 2: Syntax Error, Error 53: File Not Found",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

static const char * const s_check_conflicts[] = {
    "LOOP", "COUNT", "DATE", "TIME", "POS", "TYPE", "SPEED", "BAUD",
    "SOUND", "SCREEN", "PLAY", "PORT", "MIN", "MAX", "CLAMP", "ROUND",
    "LABEL", "BUTTON", "TITLE", "COLOR", "LINE", "CIRCLE", "PAINT",
    "DRAW", "PSET", "POINT", "KEY", "ATTR", "DIR", "WEEK", "SECOND",
    NULL
};

typedef enum {
    CHECK_FMT_SUMMARY,
    CHECK_FMT_DETAILED,
    CHECK_FMT_JSON,
    CHECK_FMT_CONFLICTS,
    CHECK_FMT_SMART,
    CHECK_FMT_FOLKLORE
} CheckFormat;

static void check_audit_folklore(VDevContext *vd, MemoryContext *mem, bool deep_mode) {
    if (!vd || !mem) return;
    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    if (!lines || line_count == 0) return;

    if (deep_mode) {
        vdev_puts(vd, "=== CHECK FOLKLORE: Vintage Performance & Modernization Audit ===\n");
    }

    int notices = 0;

    for (size_t l = 0; l < line_count; l++) {
        const char *t = lines[l].text;
        if (!t) continue;
        int lnum = (int)lines[l].line_number;

        // 1. Short-form OPEN check
        if (runtime_strcasestr(t, "OPEN")) {
            const char *op = runtime_strcasestr(t, "OPEN");
            const char *sp = op + 4;
            while (*sp == ' ') sp++;
            if (*sp == '"' && sp[1] && sp[2] == '"' && (sp[1] == 'I' || sp[1] == 'i' || sp[1] == 'O' || sp[1] == 'o' || sp[1] == 'A' || sp[1] == 'a' || sp[1] == 'R' || sp[1] == 'r')) {
                if (deep_mode) {
                    vdev_printf(vd, "  Line %d: Vintage short-form OPEN \"%c\" detected. Supported natively in BASIC++, modernizing to 'OPEN filename FOR mode AS #n' improves portability -> Recommend: REFORMAT [expand_open]\n", lnum, sp[1]);
                } else {
                    vdev_printf(vd, "  [INFO] Line %d: Short-form OPEN detected -> Optional: REFORMAT [expand_open]\n", lnum);
                }
                notices++;
            }
        }

        // 2. Bare NEXT loop check
        if (runtime_strcasestr(t, "NEXT")) {
            const char *np = runtime_strcasestr(t, "NEXT");
            const char *after = np + 4;
            while (*after == ' ') after++;
            if (*after == '\0' || *after == ':' || *after == '\'') {
                if (deep_mode) {
                    vdev_printf(vd, "  Line %d: Bare NEXT loop detected. Vintage interpreters skipped variable scanning; modern code recommends explicit NEXT (e.g. NEXT I) to avoid loop stack mismatch.\n", lnum);
                } else {
                    vdev_printf(vd, "  [INFO] Line %d: Bare NEXT loop detected -> Explicit variable name recommended for clarity\n", lnum);
                }
                notices++;
            }
        }

        // 3. Unquoted DATA keyword collision check
        if (runtime_strcasestr(t, "DATA")) {
            const char *dp = runtime_strcasestr(t, "DATA") + 4;
            while (*dp == ' ') dp++;
            while (*dp && *dp != ':' && *dp != '\'') {
                while (*dp == ' ' || *dp == ',') dp++;
                if (*dp == '"') {
                    dp++;
                    while (*dp && *dp != '"') dp++;
                    if (*dp == '"') dp++;
                } else if (*dp && *dp != ':' && *dp != '\'') {
                    char item[64] = {0};
                    size_t ilen = 0;
                    while (*dp && *dp != ',' && *dp != ':' && *dp != '\'' && ilen < sizeof(item) - 1) {
                        item[ilen++] = *dp++;
                    }
                    while (ilen > 0 && item[ilen - 1] == ' ') ilen--;
                    item[ilen] = '\0';
                    for (int i = 0; s_check_conflicts[i] != NULL; i++) {
                        if (runtime_strcasecmp(item, s_check_conflicts[i]) == 0) {
                            if (deep_mode) {
                                vdev_printf(vd, "  Line %d: DATA contains unquoted keyword item '%s' -> Recommend quoting to prevent parsing ambiguities.\n", lnum, item);
                            } else {
                                vdev_printf(vd, "  [INFO] Line %d: DATA statement contains unquoted keyword '%s'\n", lnum, item);
                            }
                            notices++;
                            break;
                        }
                    }
                }
            }
        }

        // 4. Distant GOTO/GOSUB check
        const char *gp = runtime_strcasestr(t, "GOTO");
        if (!gp) gp = runtime_strcasestr(t, "GOSUB");
        if (gp) {
            const char *num_p = gp + 4;
            if (runtime_strncasecmp(gp, "GOSUB", 5) == 0) num_p = gp + 5;
            while (*num_p == ' ') num_p++;
            if (*num_p >= '0' && *num_p <= '9') {
                int target = runtime_atoi(num_p);
                if (target > lnum + 250) {
                    if (deep_mode) {
                        vdev_printf(vd, "  Line %d: Distant jump to line %d. Vintage GW-BASIC scans line numbers from line 1; keeping hot subroutines near top of program reduces scan overhead.\n", lnum, target);
                    } else {
                        vdev_printf(vd, "  [INFO] Line %d: Distant jump to line %d detected\n", lnum, target);
                    }
                    notices++;
                }
            }
        }

        // 5. FIELD statement check
        if (runtime_strcasestr(t, "FIELD")) {
            if (deep_mode) {
                vdev_printf(vd, "  Line %d: Vintage FIELD buffer detected -> Consider modernizing to strongly-typed 'TYPE struct_name ... END TYPE' records.\n", lnum);
            } else {
                vdev_printf(vd, "  [INFO] Line %d: FIELD buffer record detected -> Consider TYPE struct modernization\n", lnum);
            }
            notices++;
        }
    }

    if (deep_mode) {
        vdev_printf(vd, "Folklore audit complete: %d folklore pattern(s) identified.\n", notices);
    }
}

BppError stmt_check_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    char filepath[260] = {0};
    CheckFormat fmt = CHECK_FMT_SUMMARY;

    // Multi-delimiter handling: CHECK { ... } or CHECK [ ... ]
    if (lex_peek(lex).type == TOK_RPN_LITERAL) {
        BppToken r = lex_next(lex);
        char buf[256];
        runtime_snprintf(buf, sizeof(buf), "%.*s", (int)r.length, r.start);
        char *fstart = runtime_strcasestr(buf, "file");
        if (fstart) {
            char *col = runtime_strchr(fstart, ':');
            if (col) {
                char *p = col + 1;
                while (*p && (*p == ' ' || *p == '"')) p++;
                char *pend = p;
                while (*pend && *pend != '"' && *pend != ',' && *pend != '}' && *pend != '\n') pend++;
                runtime_snprintf(filepath, sizeof(filepath), "%.*s", (int)(pend - p), p);
            }
        }
        if (runtime_strcasestr(buf, "conflicts")) fmt = CHECK_FMT_CONFLICTS;
        else if (runtime_strcasestr(buf, "smart")) fmt = CHECK_FMT_SMART;
        else if (runtime_strcasestr(buf, "folklore")) fmt = CHECK_FMT_FOLKLORE;
        else if (runtime_strcasestr(buf, "json")) fmt = CHECK_FMT_JSON;
        else if (runtime_strcasestr(buf, "detailed")) fmt = CHECK_FMT_DETAILED;
        else fmt = CHECK_FMT_SUMMARY;
    } else if (lex_peek(lex).type == TOK_LBRACE) {
        lex_next(lex);
        while (lex_peek(lex).type != TOK_RBRACE && lex_peek(lex).type != TOK_EOF) {
            BppToken k = lex_next(lex);
            if (lex_peek(lex).start && *lex_peek(lex).start == ':') lex_next(lex);
            if (lex_peek(lex).type == TOK_EOL) lex_next(lex);
            BppToken v = lex_next(lex);
            char k_str[64] = {0};
            char v_str[64] = {0};
            runtime_snprintf(k_str, sizeof(k_str), "%.*s", (int)k.length, k.start);
            runtime_snprintf(v_str, sizeof(v_str), "%.*s", (int)v.length, v.start);

            if (runtime_strcasestr(k_str, "file")) {
                runtime_strncpy(filepath, v_str, sizeof(filepath) - 1);
            }
            if (runtime_strcasestr(k_str, "conflicts") || runtime_strcasestr(v_str, "conflicts")) {
                fmt = CHECK_FMT_CONFLICTS;
            } else if (runtime_strcasestr(k_str, "smart") || runtime_strcasestr(v_str, "smart")) {
                fmt = CHECK_FMT_SMART;
            } else if (runtime_strcasestr(k_str, "folklore") || runtime_strcasestr(v_str, "folklore")) {
                fmt = CHECK_FMT_FOLKLORE;
            } else if (runtime_strcasestr(k_str, "json") || runtime_strcasestr(v_str, "json")) {
                fmt = CHECK_FMT_JSON;
            } else if (runtime_strcasestr(k_str, "detailed") || runtime_strcasestr(v_str, "detailed")) {
                fmt = CHECK_FMT_DETAILED;
            } else if (runtime_strcasestr(k_str, "summary") || runtime_strcasestr(v_str, "summary")) {
                fmt = CHECK_FMT_SUMMARY;
            }
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
        }
        if (lex_peek(lex).type == TOK_RBRACE) lex_next(lex);
    } else if (lex_peek(lex).type == TOK_LBRACKET) {
        lex_next(lex);
        BppToken b = lex_next(lex);
        if (b.type == TOK_STRING) {
            runtime_snprintf(filepath, sizeof(filepath), "%.*s", (int)b.length, b.start);
            if (lex_peek(lex).type == TOK_COMMA) lex_next(lex);
            b = lex_next(lex);
        }
        if (runtime_strncasecmp(b.start, "CONFLICTS", 9) == 0) fmt = CHECK_FMT_CONFLICTS;
        else if (runtime_strncasecmp(b.start, "SMART", 5) == 0) fmt = CHECK_FMT_SMART;
        else if (runtime_strncasecmp(b.start, "FOLKLORE", 8) == 0) fmt = CHECK_FMT_FOLKLORE;
        else if (runtime_strncasecmp(b.start, "JSON", 4) == 0) fmt = CHECK_FMT_JSON;
        else if (runtime_strncasecmp(b.start, "DETAILED", 8) == 0) fmt = CHECK_FMT_DETAILED;
        while (lex_peek(lex).type != TOK_RBRACKET && lex_peek(lex).type != TOK_EOF) lex_next(lex);
        if (lex_peek(lex).type == TOK_RBRACKET) lex_next(lex);
    } else {
        BppToken tok = lex_next(lex);
        if (tok.type == TOK_STRING) {
            runtime_snprintf(filepath, sizeof(filepath), "%.*s", (int)tok.length, tok.start);
            tok = lex_next(lex);
        }
        if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
            char mod[32] = {0};
            runtime_snprintf(mod, sizeof(mod), "%.*s", (int)tok.length, tok.start);

            if (runtime_strcasecmp(mod, "DETAILED") == 0) fmt = CHECK_FMT_DETAILED;
            else if (runtime_strcasecmp(mod, "JSON") == 0) fmt = CHECK_FMT_JSON;
            else if (runtime_strcasecmp(mod, "SUMMARY") == 0) fmt = CHECK_FMT_SUMMARY;
            else if (runtime_strcasecmp(mod, "CONFLICTS") == 0) fmt = CHECK_FMT_CONFLICTS;
            else if (runtime_strcasecmp(mod, "SMART") == 0) fmt = CHECK_FMT_SMART;
            else if (runtime_strcasecmp(mod, "FOLKLORE") == 0) fmt = CHECK_FMT_FOLKLORE;
        }
    }

    VDevContext *vd = vm_get_vdev(vm);
    MemoryContext *mem = NULL;

    if (filepath[0] != '\0') {
        void *fp = platform_file_open(filepath, "r");
        if (!fp) {
            err.code = 53; // File Not Found
            err.message = "CHECK target file not found";
            return err;
        }
        mem = mem_init(1024 * 1024 * 4, 64 * 1024, 64 * 1024, 64 * 1024);
        if (!mem) {
            platform_file_close(fp);
            err.code = 7;
            err.message = "Out of memory allocating isolated CHECK sandbox";
            return err;
        }
        char line_buf[512];
        BppLineNumber line_seq = 10;
        while (platform_file_gets(line_buf, sizeof(line_buf), fp)) {
            char *nl = runtime_strchr(line_buf, '\n');
            if (nl) *nl = '\0';
            char *cr = runtime_strchr(line_buf, '\r');
            if (cr) *cr = '\0';

            BppLineNumber lnum = 0;
            const char *p = line_buf;
            while (*p == ' ' || *p == '\t') p++;
            if (*p >= '0' && *p <= '9') {
                while (*p >= '0' && *p <= '9') {
                    lnum = lnum * 10 + (*p - '0');
                    p++;
                }
            }
            if (lnum > 0) {
                mem_program_insert(mem, lnum, line_buf);
            } else {
                char fmt_line[600];
                runtime_snprintf(fmt_line, sizeof(fmt_line), "%lld %s", (long long)line_seq, line_buf);
                mem_program_insert(mem, line_seq, fmt_line);
                line_seq += 10;
            }
        }
        platform_file_close(fp);
        vdev_printf(vd, "=== CHECK: %s (Isolated Sandbox) ===\n", filepath);
    } else {
        mem = vm_get_mem(vm);
    }

    if (!mem) return err;

    if (fmt == CHECK_FMT_CONFLICTS) {
        vdev_puts(vd, "=== CHECK CONFLICTS: Keyword vs Variable Analysis ===\n");
        int count = 0;
        size_t line_count = 0;
        BppProgramLine *lines = mem_program_get_all(mem, &line_count);
        for (size_t l = 0; l < line_count; l++) {
            const char *text = lines[l].text;
            if (!text) continue;
            for (int i = 0; s_check_conflicts[i] != NULL; i++) {
                const char *kw = s_check_conflicts[i];
                if (runtime_strcasestr(text, kw)) {
                    vdev_printf(vd, "  Line %d: Potential variable '%s' conflicts with reserved keyword -> Recommend: RENAME %s TO v_%s\n",
                                (int)lines[l].line_number, kw, kw, kw);
                    count++;
                }
            }
        }
        vdev_printf(vd, "Found %d keyword conflict(s). Run 'RENAME CONFLICTS' or 'RENAME SMART' to modernize.\n", count);
        if (filepath[0] != '\0') mem_shutdown(mem);
        return err;
    }

    if (fmt == CHECK_FMT_SMART) {
        vdev_puts(vd, "=== CHECK SMART: Semantic Context Inference Report ===\n");
        ContextLexicon h_lex;
        semantic_harvest_program(vm, &h_lex, MODERNIZE_CASE_MIXED);
        for (size_t c = 0; c < h_lex.candidate_count; c++) {
            vdev_printf(vd, "  Line %d: Variable '%s' inferred as %s [Confidence: %d%%] via %s\n",
                        h_lex.candidates[c].line_number,
                        h_lex.candidates[c].old_name,
                        h_lex.candidates[c].modern_name,
                        h_lex.candidates[c].confidence,
                        h_lex.candidates[c].source_rule);
        }
        vdev_printf(vd, "Found %d candidate variable(s) for smart modernization. Run 'RENAME SMART' to apply.\n", (int)h_lex.candidate_count);
        if (filepath[0] != '\0') mem_shutdown(mem);
        return err;
    }

    if (fmt == CHECK_FMT_FOLKLORE) {
        check_audit_folklore(vd, mem, true);
        if (filepath[0] != '\0') mem_shutdown(mem);
        return err;
    }

    // Full 7-Stage Code Health & Modernization Readiness Engine
    StaticAnalysisReport report;
    analyzer_run(mem, &report);

    // Harvest keyword collisions
    int conflict_count = 0;
    size_t line_count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &line_count);
    for (size_t l = 0; l < line_count; l++) {
        const char *text = lines[l].text;
        if (!text) continue;
        for (int i = 0; s_check_conflicts[i] != NULL; i++) {
            if (runtime_strcasestr(text, s_check_conflicts[i])) conflict_count++;
        }
    }

    // Harvest context candidates
    ContextLexicon h_lex;
    semantic_harvest_program(vm, &h_lex, MODERNIZE_CASE_MIXED);

    // Calculate Modernization Readiness Score (0..100)
    int readiness_score = 100;
    if (report.for_count != report.next_count) readiness_score -= 15;
    if (report.do_count != report.loop_count) readiness_score -= 15;
    if (report.while_count != report.wend_count) readiness_score -= 15;
    readiness_score -= (conflict_count * 2);
    readiness_score -= (report.unreachable_lines * 3);
    if (report.cyclomatic_complexity > 20) readiness_score -= 10;
    if (readiness_score < 10) readiness_score = 10;
    if (readiness_score > 100) readiness_score = 100;

    if (fmt == CHECK_FMT_DETAILED) {
        vdev_puts(vd, "=== CHECK: 7-Stage Code Health & Modernization Audit ===\n");
        vdev_printf(vd, "  Stage 1: Lexical & Line Scan     - %zu Total Line(s) (%zu Code, %zu Comments)\n",
                    report.total_lines, report.code_lines, report.comment_lines);
        vdev_printf(vd, "  Stage 2: Control Flow Structures - FOR:%d/%d, DO:%d/%d, WHILE:%d/%d\n",
                    report.for_count, report.next_count, report.do_count, report.loop_count,
                    report.while_count, report.wend_count);
        vdev_printf(vd, "  Stage 3: Keyword Collision Scan  - %d Potential Conflict(s) Detected\n", conflict_count);
        vdev_printf(vd, "  Stage 4: Semantic Context Clues  - %zu Candidate Variable(s) Harvested\n", h_lex.candidate_count);
        vdev_printf(vd, "  Stage 5: Flow Reachability Audit - %d Unreachable Dead Code Line(s)\n", report.unreachable_lines);
        vdev_printf(vd, "  Stage 6: Complexity & Health     - Cyclomatic Complexity: %d\n", report.cyclomatic_complexity);
        vdev_printf(vd, "  Stage 7: Recommendations Plan    - Modernization Readiness Score: %d%%\n", readiness_score);
        if (conflict_count > 0) vdev_puts(vd, "    -> Recommendation: Execute 'RENAME CONFLICTS' to resolve reserved keywords.\n");
        if (h_lex.candidate_count > 0) vdev_puts(vd, "    -> Recommendation: Execute 'RENAME SMART' to modernize variable names.\n");
        vdev_puts(vd, "    -> Recommendation: Execute 'REFORMAT UNPACK' to decompress packed colons.\n");
        vdev_puts(vd, "    -> Recommendation: Execute 'RENUM 10, 10' to normalize sequential line numbering.\n");
        check_audit_folklore(vd, mem, false);
    } else if (fmt == CHECK_FMT_JSON) {
        analyzer_render_json(vd, &report);
    } else {
        analyzer_render_summary(vd, &report);
        check_audit_folklore(vd, mem, false);
    }

    if (filepath[0] != '\0') {
        mem_shutdown(mem);
    }

    return err;
}

BValue func_check_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname; (void)arg_count; (void)args; (void)err;
    BValue res;
    runtime_memset(&res, 0, sizeof(res));
    res.type = VAL_NUMBER;
    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return res;

    StaticAnalysisReport report;
    analyzer_run(mem, &report);
    res.as.number = (double)report.diagnostic_count;
    return res;
}

void stmt_check_register(void) {
    lang_desc_register(&g_check_desc);

    FunctionEntry entry = {
        .name = "CHECK",
        .keyword = KW_NONE,
        .category = FCAT_UTIL,
        .ret_type = FRET_INT,
        .min_args = 0,
        .max_args = 1,
        .safety = FSAFE_STATE,
        .overridable = 0,
        .handler = NULL,
        .help_text = "Static code health check: CHECK([file$])",
        .module_name = "Debug"
    };
    funcreg_register(&entry);
}
