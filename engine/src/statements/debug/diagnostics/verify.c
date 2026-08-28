// FILENAME: verify.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (analyzer.h, analyzer.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, string.c, verify.h, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the VERIFY statement in BASIC++.
//
// ---- Includes ----

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#include "statements/debug/diagnostics/verify.h"
#include "debug/analyzer.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"

typedef enum {
    VERIFY_FMT_SUMMARY,
    VERIFY_FMT_DETAILED,
    VERIFY_FMT_JSON
} VerifyFormat;

BppError stmt_verify_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    char filepath[260] = {0};
    VerifyFormat fmt = VERIFY_FMT_SUMMARY;

    BppToken tok = lex_next(lex);
    if (tok.type == TOK_STRING) {
        snprintf(filepath, sizeof(filepath), "%.*s", (int)tok.length, tok.start + 1);
        if (filepath[0] && filepath[strlen(filepath) - 1] == '"') {
            filepath[strlen(filepath) - 1] = '\0';
        }
        tok = lex_next(lex);
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char mod[32] = {0};
        snprintf(mod, sizeof(mod), "%.*s", (int)tok.length, tok.start);
        if (strcasecmp(mod, "DETAILED") == 0) fmt = VERIFY_FMT_DETAILED;
        else if (strcasecmp(mod, "JSON") == 0) fmt = VERIFY_FMT_JSON;
        else if (strcasecmp(mod, "SUMMARY") == 0) fmt = VERIFY_FMT_SUMMARY;
    }

    MemoryContext *target_mem = NULL;
    VDevContext *vdev = vm_get_vdev(vm);

    if (filepath[0] != '\0') {
        FILE *fp = fopen(filepath, "r");
        if (!fp) {
            err.code = 53; // File Not Found
            err.message = "VERIFY target file not found";
            return err;
        }

        target_mem = mem_init(1024 * 1024, 64 * 1024, 64 * 1024, 64 * 1024);
        if (!target_mem) {
            fclose(fp);
            err.code = 7; // Out of memory
            err.message = "Out of memory allocating VERIFY context";
            return err;
        }

        char line_buf[512];
        BppLineNumber line_seq = 10;
        while (fgets(line_buf, sizeof(line_buf), fp)) {
            char *nl = strchr(line_buf, '\n');
            if (nl) *nl = '\0';
            char *cr = strchr(line_buf, '\r');
            if (cr) *cr = '\0';

            // Parse optional leading line number
            BppLineNumber lnum = 0;
            if (sscanf(line_buf, "%lld", (long long *)&lnum) == 1 && lnum > 0) {
                mem_program_insert(target_mem, lnum, line_buf);
            } else {
                char fmt_line[600];
                snprintf(fmt_line, sizeof(fmt_line), "%lld %s", (long long)line_seq, line_buf);
                mem_program_insert(target_mem, line_seq, fmt_line);
                line_seq += 10;
            }
        }
        fclose(fp);

        vdev_printf(vdev, "=== VERIFY: %s ===\n", filepath);
    } else {
        target_mem = vm_get_mem(vm);
        vdev_printf(vdev, "=== VERIFY: Active Memory Program ===\n");
    }

    StaticAnalysisReport report;
    analyzer_run(target_mem, &report);

    switch (fmt) {
        case VERIFY_FMT_DETAILED:
            analyzer_render_detailed(vdev, target_mem, &report);
            break;
        case VERIFY_FMT_JSON:
            analyzer_render_json(vdev, &report);
            break;
        case VERIFY_FMT_SUMMARY:
        default:
            analyzer_render_summary(vdev, &report);
            break;
    }

    if (filepath[0] != '\0' && target_mem) {
        mem_shutdown(target_mem);
    }

    return err;
}

void stmt_verify_register(void) {
    static const MicroLibMetadata meta = {
        .name = "VERIFY",
        .category = "Debug & Testing",
        .syntax = "VERIFY [\"filename.bas\"] [SUMMARY|DETAILED|JSON]",
        .help_text = "Executes static analysis on external disk file or current program in isolated temporary memory.",
        .error_codes = "Error 2: Syntax Error, Error 53: File Not Found"
    };
    microlib_register(&meta);
}
