// FILENAME: renum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (snprintf.h, snprintf.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, renum.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the RENUM statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/renum.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "device/vdev.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "runtime/format/snprintf.h"


BppError stmt_renum_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    BppLineNumber new_start = 10;
    BppLineNumber old_start = 0;
    BppLineNumber step = 10;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_NUMBER) {
        tok = lex_next(lex);
        new_start = (BppLineNumber)tok.as.number;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            tok = lex_peek(lex);
            if (tok.type == TOK_NUMBER) {
                tok = lex_next(lex);
                old_start = (BppLineNumber)tok.as.number;

                tok = lex_peek(lex);
                if (tok.type == TOK_COMMA) {
                    lex_next(lex);
                    tok = lex_peek(lex);
                    if (tok.type == TOK_NUMBER) {
                        tok = lex_next(lex);
                        step = (BppLineNumber)tok.as.number;
                    }
                }
            }
        }
    }

    if (step <= 0) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (count == 0) return err;

    // Build map of old line numbers to new line numbers
    typedef struct {
        BppLineNumber old_num;
        BppLineNumber new_num;
        char *text;
    } LineRemap;

    LineRemap *remaps = (LineRemap *)calloc(count, sizeof(LineRemap));
    if (!remaps) {
        err.code = ERR_OUT_OF_MEMORY;
        return err;
    }

    BppLineNumber current_new = new_start;
    for (size_t i = 0; i < count; i++) {
        remaps[i].old_num = lines[i].line_number;
        remaps[i].text = strdup(lines[i].text ? lines[i].text : "");
        if (lines[i].line_number >= old_start) {
            remaps[i].new_num = current_new;
            current_new += step;
        } else {
            remaps[i].new_num = lines[i].line_number;
        }
    }

    // Perform line reference rewriting pass across statement text
    for (size_t i = 0; i < count; i++) {
        char *text = remaps[i].text;
        if (!text || text[0] == '\0') continue;

        char new_text[2048] = "";
        size_t src_idx = 0;
        size_t dst_idx = 0;
        size_t len = strlen(text);

        static const char *kw_targets[] = {
            "GOTO", "GOSUB", "THEN", "ELSE", "RESTORE", "RESUME", "RUN", NULL
        };

        while (src_idx < len) {
            bool kw_found = false;
            size_t kw_len = 0;

            for (size_t k = 0; kw_targets[k] != NULL; k++) {
                size_t kw_match_len = strlen(kw_targets[k]);
                if (src_idx + kw_match_len <= len &&
                    strncasecmp(&text[src_idx], kw_targets[k], kw_match_len) == 0) {
                    // Ensure word boundary before and after keyword
                    bool left_boundary = (src_idx == 0 || !isalnum((unsigned char)text[src_idx - 1]));
                    bool right_boundary = (src_idx + kw_match_len >= len || !isalnum((unsigned char)text[src_idx + kw_match_len]));
                    if (left_boundary && right_boundary) {
                        kw_found = true;
                        kw_len = kw_match_len;
                        break;
                    }
                }
            }

            if (kw_found) {
                // Copy keyword
                memcpy(&new_text[dst_idx], &text[src_idx], kw_len);
                dst_idx += kw_len;
                src_idx += kw_len;

                // Copy following spaces
                while (src_idx < len && (text[src_idx] == ' ' || text[src_idx] == '\t')) {
                    new_text[dst_idx++] = text[src_idx++];
                }

                // Check if next token is a line number
                if (src_idx < len && isdigit((unsigned char)text[src_idx])) {
                    BppLineNumber target_old = 0;
                    while (src_idx < len && isdigit((unsigned char)text[src_idx])) {
                        target_old = target_old * 10 + (text[src_idx] - '0');
                        src_idx++;
                    }

                    // Look up target_old in remaps
                    bool mapped = false;
                    BppLineNumber target_new = 0;
                    for (size_t r = 0; r < count; r++) {
                        if (remaps[r].old_num == target_old) {
                            mapped = true;
                            target_new = remaps[r].new_num;
                            break;
                        }
                    }

                    if (mapped) {
                        int written = runtime_snprintf(&new_text[dst_idx], sizeof(new_text) - dst_idx, "%lld", (long long)target_new);
                        if (written > 0) dst_idx += written;
                    } else {
                        // Target line number missing - issue warning and retain reference
                        char warning_buf[256];
                        runtime_snprintf(warning_buf, sizeof(warning_buf), "Undefined line %lld in %lld\n",
                                 (long long)target_old, (long long)remaps[i].old_num);
                        VDevContext *vd = vm_get_vdev(vm);
                        if (vd) vdev_puts(vd, warning_buf);

                        int written = runtime_snprintf(&new_text[dst_idx], sizeof(new_text) - dst_idx, "%lld", (long long)target_old);
                        if (written > 0) dst_idx += written;
                    }

                }
            } else {
                new_text[dst_idx++] = text[src_idx++];
            }
        }
        new_text[dst_idx] = '\0';

        free(remaps[i].text);
        remaps[i].text = strdup(new_text);
    }

    // Clear old program lines and store renumbered lines
    mem_program_clear(mem);
    for (size_t i = 0; i < count; i++) {
        mem_program_store(mem, remaps[i].new_num, remaps[i].text);
        if (remaps[i].text) free(remaps[i].text);
    }
    free(remaps);

    return err;
}

void stmt_renum_register(void) {
    static const MicroLibMetadata meta = {
        .name = "RENUM",
        .category = "Program Mgmt & Editing",
        .syntax = "RENUM [new_start] [,[old_start] [,step]]",
        .help_text = "Renumbers program lines in memory starting from old_start with new_start and step interval.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
