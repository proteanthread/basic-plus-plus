// FILENAME: list.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, list.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the LIST statement in BASIC++.
//
// ---- Includes ----

#include "statements/program/list.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/memory.h"
#include "runtime/strings.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>
#include <stdio.h>

BppError stmt_list_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (!vm || !lex) {
        err.code = ERR_ILLEGAL_FUNCTION_CALL;
        return err;
    }

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    BppLineNumber start_line = 0;
    BppLineNumber end_line = 999999999;
    char out_filename[256] = "";
    char search_pattern[128] = "";

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_STRING) {
        tok = lex_next(lex);
        if (tok.as.string && tok.length > 0) {
            snprintf(search_pattern, sizeof(search_pattern), "%.*s", (int)tok.length, tok.as.string);
        }
    } else if (tok.type == TOK_MINUS) {
        lex_next(lex);
        tok = lex_peek(lex);
        if (tok.type == TOK_NUMBER) {
            tok = lex_next(lex);
            end_line = (BppLineNumber)tok.as.number;
        }
    } else if (tok.type == TOK_NUMBER) {
        tok = lex_next(lex);
        start_line = (BppLineNumber)tok.as.number;
        end_line = start_line;

        tok = lex_peek(lex);
        if (tok.type == TOK_MINUS) {
            lex_next(lex);
            end_line = 999999999;
            tok = lex_peek(lex);
            if (tok.type == TOK_NUMBER) {
                tok = lex_next(lex);
                end_line = (BppLineNumber)tok.as.number;
            }
        }
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BValue fn_val = eval_expression(vm, lex, &err);
        if (err.code != 0) {
            return err;
        }
        if (fn_val.type != VAL_STRING || !fn_val.as.string) {
            if (fn_val.type == VAL_STRING && fn_val.as.string) {
                str_release(vm_get_str(vm), fn_val.as.string);
            }
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        const char *val_str = str_data(fn_val.as.string);
        if (search_pattern[0] == '\0' && (strstr(val_str, ".") != NULL || strchr(val_str, '/') != NULL || strchr(val_str, '\\') != NULL)) {
            snprintf(out_filename, sizeof(out_filename), "%s", val_str);
        } else {
            snprintf(search_pattern, sizeof(search_pattern), "%s", val_str);
        }
        str_release(vm_get_str(vm), fn_val.as.string);
    }

    FILE *fp = NULL;
    if (out_filename[0] != '\0') {
        fp = fopen(out_filename, "w");
        if (!fp) {
            err.code = ERR_FILE_NOT_FOUND;
            return err;
        }
    }

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    VDevContext *vd = vm_get_vdev(vm);

    for (size_t i = 0; i < count; i++) {
        if (lines[i].line_number >= start_line && lines[i].line_number <= end_line) {
            const char *text = lines[i].text ? lines[i].text : "";
            if (search_pattern[0] != '\0') {
                // Case-insensitive search
                bool found = false;
                size_t text_len = strlen(text);
                size_t pat_len = strlen(search_pattern);
                if (pat_len <= text_len) {
                    for (size_t k = 0; k <= text_len - pat_len; k++) {
                        if (strncasecmp(&text[k], search_pattern, pat_len) == 0) {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) continue;
            }
            char buf[1024];
            snprintf(buf, sizeof(buf), "%lld %s\n", (long long)lines[i].line_number, text);
            if (fp) {
                fputs(buf, fp);
            } else {
                vdev_puts(vd, buf);
            }
        }
    }

    if (fp) {
        fclose(fp);
    }
    return err;
}

void stmt_list_register(void) {
    static const MicroLibMetadata meta = {
        .name = "LIST",
        .category = "Program Mgmt & Editing",
        .syntax = "LIST [start_line] [- [end_line]] [, filename_expr]",
        .help_text = "Displays specified program lines or entire source in memory, optionally writing to destination file.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
