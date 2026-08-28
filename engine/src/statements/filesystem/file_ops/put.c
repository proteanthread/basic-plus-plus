// FILENAME: put.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (bgi.h, bgi.c, eval.h, eval.c, lexer.h, lexer.c)
// NEEDS: libengine (map.h, map.c, put.h, string.c, vm.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the PUT statement in BASIC++.
//
// ---- Includes ----

#include "statements/filesystem/file_ops/put.h"
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

void stmt_put_register(void) {
    static const MicroLibMetadata meta = {
        .name = "PUT",
        .category = "Filesystem I/O & Graphics",
        .syntax = "PUT [#]file_num [, record_number] | PUT (x, y), array_name [, action]",
        .help_text = "Writes a record from the FIELD buffer into a random-access file or draws a memory array onto the screen.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number, Error 63: Bad Record Number"
    };
    microlib_register(&meta);
}

BppError stmt_put_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_LPAREN) {
        // Graphics PUT (x, y), array [, action]
        lex_next(lex); // Consume '('
        BValue x_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' in PUT";
            return err;
        }
        BValue y_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        tok = lex_next(lex);
        if (tok.type != TOK_RPAREN) {
            err.code = 2; err.message = "Expected ')' in PUT";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_COMMA) {
            err.code = 2; err.message = "Expected ',' before array name in PUT";
            return err;
        }

        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
            err.code = 2; err.message = "Expected array identifier in PUT";
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

        int action = 0; // 0=XOR, 1=PSET, 2=PRESET, 3=AND, 4=OR
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex); // Consume ','
            tok = lex_next(lex);
            char act_str[32] = "";
            if (tok.length < sizeof(act_str)) {
                memcpy(act_str, tok.start, tok.length);
                act_str[tok.length] = '\0';
            }
            if (platform_strcasecmp(act_str, "PSET") == 0) action = 1;
            else if (platform_strcasecmp(act_str, "PRESET") == 0) action = 2;
            else if (platform_strcasecmp(act_str, "AND") == 0) action = 3;
            else if (platform_strcasecmp(act_str, "OR") == 0) action = 4;
            else if (platform_strcasecmp(act_str, "XOR") == 0) action = 0;
        }

        int x0 = (int)x_val.as.number;
        int y0 = (int)y_val.as.number;

        ArrayContext *arr_ctx = vm_get_arr(vm);
        int total_size = 0;
        BValue *elems = arr_get_flat_elements(arr_ctx, arr_name, &total_size);
        if (elems && total_size >= 2) {
            int w = (int)elems[0].as.number;
            int h = (int)elems[1].as.number;
            if (w > 0 && h > 0 && total_size >= 2 + w * h) {
                BGI_Context *bgi = BGI_get_global_context();
                int idx = 2;
                for (int dy = 0; dy < h; ++dy) {
                    for (int dx = 0; dx < w; ++dx) {
                        int src_c = (int)elems[idx++].as.number;
                        int px = x0 + dx;
                        int py = y0 + dy;
                        if (bgi && bgi->initialized && bgi->framebuffer) {
                            int dst_c = BGI_getpixel(bgi, px, py);
                            int final_c = src_c;
                            switch (action) {
                                case 0: final_c = dst_c ^ src_c; break; // XOR
                                case 1: final_c = src_c; break;         // PSET
                                case 2: final_c = ~src_c & 0x0F; break; // PRESET
                                case 3: final_c = dst_c & src_c; break; // AND
                                case 4: final_c = dst_c | src_c; break; // OR
                            }
                            BGI_putpixel(bgi, px, py, final_c);
                        }
                    }
                }
                bgi_sync();
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
        const char *map_name = map_get_channel_map(vm, channel);
        if (map_name) {
            map_sync_from_variables(vm, map_name, rec_buf, rec_len);
        }
        file_write(fc, channel, rec_buf, rec_len);
    }

    return err;
}
