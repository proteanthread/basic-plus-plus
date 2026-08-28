// FILENAME: check.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (analyzer.h, analyzer.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (check.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the CHECK statement in BASIC++.
//
// ---- Includes ----

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

#include "statements/debug/diagnostics/check.h"
#include "debug/analyzer.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"

typedef enum {
    CHECK_FMT_SUMMARY,
    CHECK_FMT_DETAILED,
    CHECK_FMT_JSON
} CheckFormat;

BppError stmt_check_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_next(lex);
    CheckFormat fmt = CHECK_FMT_SUMMARY;

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char mod[32] = {0};
        snprintf(mod, sizeof(mod), "%.*s", (int)tok.length, tok.start);

        if (strcasecmp(mod, "DETAILED") == 0) {
            fmt = CHECK_FMT_DETAILED;
        } else if (strcasecmp(mod, "JSON") == 0) {
            fmt = CHECK_FMT_JSON;
        } else if (strcasecmp(mod, "SUMMARY") == 0) {
            fmt = CHECK_FMT_SUMMARY;
        }
    }

    MemoryContext *mem = vm_get_mem(vm);
    VDevContext *vdev = vm_get_vdev(vm);

    StaticAnalysisReport report;
    analyzer_run(mem, &report);

    switch (fmt) {
        case CHECK_FMT_DETAILED:
            analyzer_render_detailed(vdev, mem, &report);
            break;
        case CHECK_FMT_JSON:
            analyzer_render_json(vdev, &report);
            break;
        case CHECK_FMT_SUMMARY:
        default:
            analyzer_render_summary(vdev, &report);
            break;
    }

    return err;
}

void stmt_check_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CHECK",
        .category = "Debug & Testing",
        .syntax = "CHECK [SUMMARY|DETAILED|JSON]",
        .help_text = "Executes static analysis on active program in memory and outputs summary, detailed, or JSON metrics.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta);
}
