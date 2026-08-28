// FILENAME: rem.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, rem.h, string.c, vm.h)
// Provides runtime implementation for the REM statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/rem.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdio.h>
#include <string.h>

void stmt_rem_register(void) {
    MicroLibMetadata meta = {
        .name = "REM",
        .category = "Control Flow",
        .syntax = "REM [comment text] or ' [comment text]",
        .help_text = "Defines a remark/comment line ignored by the BASIC engine during execution.",
        .error_codes = "None (comments never generate errors)"
    };
    microlib_register(&meta);
}

BppError stmt_rem_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    const char *pos = lex_get_pos(lex);
    if (!pos) return err;

    while (*pos == ' ' || *pos == '\t') pos++;

    if (strncasecmp(pos, "$INCLUDE:", 9) == 0 || strncasecmp(pos, "$INCLUDE", 8) == 0) {
        pos += (strncasecmp(pos, "$INCLUDE:", 9) == 0) ? 9 : 8;
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
            FILE *f = fopen(inc_path, "r");
            if (!f) {
                char alt_path[512];
                snprintf(alt_path, sizeof(alt_path), "engine/include/%s", inc_path);
                f = fopen(alt_path, "r");
            }
            if (!f) {
                char alt_path[512];
                snprintf(alt_path, sizeof(alt_path), "include/%s", inc_path);
                f = fopen(alt_path, "r");
            }
            if (f) {
                char line_buf[1024];
                while (fgets(line_buf, sizeof(line_buf), f)) {
                    size_t llen = strlen(line_buf);
                    while (llen > 0 && (line_buf[llen - 1] == '\r' || line_buf[llen - 1] == '\n')) {
                        line_buf[--llen] = '\0';
                    }
                    if (llen == 0) continue;
                    BppError line_err = vm_execute_line(vm, line_buf);
                    if (line_err.code != 0) {
                        fclose(f);
                        return line_err;
                    }
                }
                fclose(f);
            }
        }
    }

    return err;
}
