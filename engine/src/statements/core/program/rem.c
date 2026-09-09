// FILENAME: rem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (lexer.h, lexer.c, rem.h, string.c, vm.h)
// Provides runtime implementation for the REM statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/rem.h"
#include "runtime/language_descriptor.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "platform/platform.h"

static const LangDesc g_rem_desc = {
    .name = "REM",
    .category = "Control Flow",
    .syntax = "REM [comment text] or ' [comment text]",
    .description = "Defines a remark/comment line ignored by the BASIC engine during execution.",
    .error_summary = "None (comments never generate errors)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SAFE,
    .type = FEATURE_STATEMENT
};

void stmt_rem_register(void) {
    lang_desc_register(&g_rem_desc);
}

BppError stmt_rem_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    const char *pos = lex_get_pos(lex);
    if (!pos) return err;

    while (*pos == ' ' || *pos == '\t') pos++;

    if (runtime_strncasecmp(pos, "$INCLUDE:", 9) == 0 || runtime_strncasecmp(pos, "$INCLUDE", 8) == 0) {
        pos += (runtime_strncasecmp(pos, "$INCLUDE:", 9) == 0) ? 9 : 8;
        while (*pos == ' ' || *pos == '\t') pos++;

        char inc_path[512] = {0};
        size_t idx = 0;
        char quote = 0;
        if (*pos == '\'' || *pos == '"') {
            quote = *pos++;
            while (*pos && *pos != quote && idx < sizeof(inc_path) - 1) {
                inc_path[idx++] = *pos++;
            }
            if (*pos == quote) pos++;
        } else {
            while (*pos && *pos != ' ' && *pos != '\t' && *pos != '\r' && *pos != '\n' && idx < sizeof(inc_path) - 1) {
                inc_path[idx++] = *pos++;
            }
        }
        inc_path[idx] = '\0';

        if (idx > 0) {
            void *f = platform_file_open(inc_path, "r");
            if (!f) {
                char alt_path[512];
                runtime_snprintf(alt_path, sizeof(alt_path), "engine/include/%s", inc_path);
                f = platform_file_open(alt_path, "r");
            }
            if (!f) {
                char alt_path[512];
                runtime_snprintf(alt_path, sizeof(alt_path), "include/%s", inc_path);
                f = platform_file_open(alt_path, "r");
            }
            if (f) {
                char line_buf[1024];
                while (platform_file_gets(line_buf, sizeof(line_buf), f)) {
                    size_t llen = runtime_strlen(line_buf);
                    while (llen > 0 && (line_buf[llen - 1] == '\r' || line_buf[llen - 1] == '\n')) {
                        line_buf[--llen] = '\0';
                    }
                    if (llen == 0) continue;
                    BppError line_err = vm_execute_line(vm, line_buf);
                    if (line_err.code != 0) {
                        platform_file_close(f);
                        return line_err;
                    }
                }
                platform_file_close(f);
            }
        }
    }

    return err;
}
