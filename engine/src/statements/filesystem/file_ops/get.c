// FILENAME: get.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (bgi.h, bgi.c, eval.h, eval.c, get.h, lexer.h, lexer.c)
// NEEDS: libengine (map.h, map.c, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the GET statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/get.h"
#include "statements/core/program/map.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "runtime/arrays.h"
#include "device/bgi.h"
#include "runtime/micro_lib_metadata.h"
#include "platform/platform.h"
#include <string.h>

void stmt_get_register(void) {
    static const MicroLibMetadata meta = {
        .name = "GET",
        .category = "Filesystem I/O & Graphics",
        .syntax = "GET [#]file_num [, record_number] | GET (x1, y1)-(x2, y2), array_name",
        .help_text = "Reads a record from a random-access file or captures a screen rectangle into a memory array.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number"
    };
    microlib_register(&meta);
}

BppError stmt_get_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        // Graphics GET (x1, y1)-(x2, y2), array
        lex_next(lex); // Consume '('
        BValue x1_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in GET";
            return err;
        }
        BValue y1_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in GET";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_MINUS) {
            err.code = 2; err.message = "Expected '-' in GET";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_LPAREN) {
            err.code = 2; err.message = "Expected '(' in GET";
            return err;
        }
        BValue x2_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in GET";
            return err;
        }
        BValue y2_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in GET";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' before array name in GET";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected array identifier in GET";
            return err;
        }

        char arr_name[64];
        if (tok.length >= sizeof(arr_name)) tok.length = sizeof(arr_name) - 1;
        memcpy(arr_name, tok.start, tok.length);
        arr_name[tok.length] = '\0';

        BppToken peek_paren = lex_peek(lex);
        if (peek_paren.type == TOK_LPAREN) {
            lex_next(lex);
            BValue off_val = eval_expression(vm, lex, &err);
            (void)off_val;
            if (err.code != 0) return err;
            if (lex_next(lex).type != TOK_RPAREN) {
                err.code = 2; err.message = "Expected ')' after array index";
                return err;
            }
        }

        int x1 = (int)x1_val.as.number;
        int y1 = (int)y1_val.as.number;
        int x2 = (int)x2_val.as.number;
        int y2 = (int)y2_val.as.number;
        if (x2 < x1) { int tmp = x1; x1 = x2; x2 = tmp; }
        if (y2 < y1) { int tmp = y1; y1 = y2; y2 = tmp; }
        int w = x2 - x1 + 1;
        int h = y2 - y1 + 1;

        ArrayContext *arr_ctx = vm_get_arr(vm);
        int total_needed = 2 + w * h;
        BValue *elems = arr_ensure_capacity(arr_ctx, arr_name, total_needed);
        if (!elems) {
            int bounds[1] = { total_needed };
            arr_dim(arr_ctx, arr_name, 1, bounds);
            elems = arr_ensure_capacity(arr_ctx, arr_name, total_needed);
        }

        if (elems) {
            elems[0].type = VAL_NUMBER;
            elems[0].as.number = (double)w;
            elems[1].type = VAL_NUMBER;
            elems[1].as.number = (double)h;

            BGI_Context *bgi = BGI_get_global_context();
            int idx = 2;
            for (int y = y1; y <= y2; ++y) {
                for (int x = x1; x <= x2; ++x) {
                    int c = 0;
                    if (bgi && bgi->initialized && bgi->framebuffer) {
                        c = BGI_getpixel(bgi, x, y);
                    }
                    elems[idx].type = VAL_NUMBER;
                    elems[idx].as.number = (double)c;
                    idx++;
                }
            }
        }
        return err;
    }

    BppToken hash = lex_peek(lex);
    if (hash.type == TOK_HASH) {
        lex_next(lex);
    }

    BValue ch_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    int channel = (int)ch_val.as.number;
    FileContext *fc = vm_get_file(vm);
    if (!file_is_open(fc, channel)) {
        err.code = 52;
        err.message = "Bad File Number";
        return err;
    }

    long record_num = -1;
    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) {
        lex_next(lex);
        BppToken rec_kw = lex_peek(lex);
        if ((rec_kw.type == TOK_KEYWORD && rec_kw.as.keyword == KW_RECORD) ||
            (rec_kw.type == TOK_IDENT && rec_kw.length == 6 && platform_strncasecmp(rec_kw.start, "RECORD", 6) == 0)) {
            lex_next(lex); // Consume RECORD
        }
        BValue rec_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (rec_val.type == VAL_NUMBER || rec_val.type == VAL_INTEGER) {
            record_num = (long)rec_val.as.number;
        }
    }

    int rec_len = file_get_record_len(fc, channel);
    if (rec_len <= 0) rec_len = 128;

    if (record_num > 0) {
        file_seek(fc, channel, (record_num - 1) * rec_len);
    }

    unsigned char *rec_buf = file_get_record_buffer(fc, channel);
    if (rec_buf) {
        file_read(fc, channel, rec_buf, rec_len);
        const char *map_name = map_get_channel_map(vm, channel);
        if (map_name) {
            map_sync_to_variables(vm, map_name, rec_buf, rec_len);
        }
    }

    return err;
}
