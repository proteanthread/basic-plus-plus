// FILENAME: map.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine
// NEEDS: libcore (ctype.h, ctype.c, micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, map.h, string.c)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the MAP statement in BASIC++.
//
// ---- Includes ----

#include "statements/core/program/map.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "types/errors.h"
#include "platform/platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static MapBuffer g_map_buffers[BASIC_MAX_MAP_BUFFERS];
static int       g_map_count = 0;
static char      g_channel_maps[32][64];

void stmt_map_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAP",
        .category = "Variables & Memory",
        .syntax = "MAP (map_name) var1 [= len] [, var2 [= len] ...] | MAP (map_name) ... MAPEND",
        .help_text = "DEC RSTS/E BASIC-PLUS-2 statement declaring a static named buffer overlay across variables and RMS-11 file records.",
        .error_codes = "Error 2: Syntax Error, Error 50: Field Overflow"
    };
    microlib_register(&meta);

    static const MicroLibMetadata meta_mapend = {
        .name = "MAPEND",
        .category = "Variables & Memory",
        .syntax = "MAPEND",
        .help_text = "Terminates a multi-line MAP block definition in DEC BASIC-PLUS-2.",
        .error_codes = "Error 2: Syntax Error"
    };
    microlib_register(&meta_mapend);
}

void map_registry_clear_all(VMContext *vm) {
    (void)vm;
    memset(g_map_buffers, 0, sizeof(g_map_buffers));
    g_map_count = 0;
    memset(g_channel_maps, 0, sizeof(g_channel_maps));
}

MapBuffer *map_get_buffer(VMContext *vm, const char *map_name) {
    (void)vm;
    if (!map_name || !map_name[0]) return NULL;
    for (int i = 0; i < g_map_count; i++) {
        if (platform_strcasecmp(g_map_buffers[i].map_name, map_name) == 0) {
            return &g_map_buffers[i];
        }
    }
    return NULL;
}

bool map_registry_add_field(VMContext *vm, const char *map_name, const char *var_name, MapFieldType type, int length) {
    (void)vm;
    if (!map_name || !var_name || !map_name[0] || !var_name[0]) return false;

    MapBuffer *mb = map_get_buffer(vm, map_name);
    if (!mb) {
        if (g_map_count >= BASIC_MAX_MAP_BUFFERS) return false;
        mb = &g_map_buffers[g_map_count++];
        memset(mb, 0, sizeof(MapBuffer));
        strncpy(mb->map_name, map_name, sizeof(mb->map_name) - 1);
    }

    if (mb->field_count >= BASIC_MAX_MAP_FIELDS) return false;

    // Check if field already exists
    for (int i = 0; i < mb->field_count; i++) {
        if (platform_strcasecmp(mb->fields[i].var_name, var_name) == 0) {
            return true;
        }
    }

    MapField *mf = &mb->fields[mb->field_count++];
    strncpy(mf->var_name, var_name, sizeof(mf->var_name) - 1);
    mf->type = type;
    mf->offset = mb->total_length;
    mf->length = length;
    mb->total_length += length;

    return true;
}

bool map_bind_channel(VMContext *vm, int channel, const char *map_name) {
    (void)vm;
    if (channel < 1 || channel >= 32 || !map_name) return false;
    strncpy(g_channel_maps[channel], map_name, sizeof(g_channel_maps[channel]) - 1);
    return true;
}

const char *map_get_channel_map(VMContext *vm, int channel) {
    (void)vm;
    if (channel < 1 || channel >= 32) return NULL;
    if (g_channel_maps[channel][0] != '\0') return g_channel_maps[channel];
    return NULL;
}

void map_sync_to_variables(VMContext *vm, const char *map_name, const unsigned char *raw_buf, int buf_len) {
    if (!vm || !map_name || !raw_buf || buf_len <= 0) return;
    MapBuffer *mb = map_get_buffer(vm, map_name);
    if (!mb) return;

    VariableContext *vc = vm_get_var(vm);
    for (int i = 0; i < mb->field_count; i++) {
        MapField *mf = &mb->fields[i];
        if (mf->offset >= buf_len) break;

        int flen = mf->length;
        if (mf->offset + flen > buf_len) flen = buf_len - mf->offset;
        const unsigned char *src = raw_buf + mf->offset;

        if (mf->type == MAP_TYPE_STRING) {
            char temp_str[1024];
            int copy_len = (flen < (int)sizeof(temp_str) - 1) ? flen : (int)sizeof(temp_str) - 1;
            memcpy(temp_str, src, copy_len);
            temp_str[copy_len] = '\0';
            // Trim trailing spaces if needed, or create exact slice
            BValue val = { .type = VAL_STRING, .as.string = str_create(vm_get_str(vm), temp_str, copy_len) };
            var_assign(vc, mf->var_name, val);
        } else if (mf->type == MAP_TYPE_DOUBLE || mf->type == MAP_TYPE_SINGLE) {
            double dval = 0.0;
            if (flen >= (int)sizeof(double)) {
                memcpy(&dval, src, sizeof(double));
            } else if (flen >= (int)sizeof(float)) {
                float fval = 0.0f;
                memcpy(&fval, src, sizeof(float));
                dval = (double)fval;
            }
            BValue val = { .type = VAL_NUMBER, .as.number = dval };
            var_assign(vc, mf->var_name, val);
        } else if (mf->type == MAP_TYPE_INTEGER) {
            int ival = 0;
            if (flen >= (int)sizeof(int)) {
                memcpy(&ival, src, sizeof(int));
            } else if (flen >= 2) {
                short sval = 0;
                memcpy(&sval, src, sizeof(short));
                ival = (int)sval;
            }
            BValue val = { .type = VAL_INTEGER, .as.number = (double)ival };
            var_assign(vc, mf->var_name, val);
        }
    }
}

void map_sync_from_variables(VMContext *vm, const char *map_name, unsigned char *raw_buf, int buf_len) {
    if (!vm || !map_name || !raw_buf || buf_len <= 0) return;
    MapBuffer *mb = map_get_buffer(vm, map_name);
    if (!mb) return;

    VariableContext *vc = vm_get_var(vm);
    for (int i = 0; i < mb->field_count; i++) {
        MapField *mf = &mb->fields[i];
        if (mf->offset >= buf_len) break;

        int flen = mf->length;
        if (mf->offset + flen > buf_len) flen = buf_len - mf->offset;
        unsigned char *dst = raw_buf + mf->offset;

        BValue *val = var_lookup(vc, mf->var_name, false);
        if (mf->type == MAP_TYPE_STRING) {
            memset(dst, ' ', flen);
            if (val && val->type == VAL_STRING && val->as.string) {
                const char *data = str_data(val->as.string);
                size_t slen = str_len(val->as.string);
                size_t clen = (slen < (size_t)flen) ? slen : (size_t)flen;
                memcpy(dst, data, clen);
            }
        } else if (mf->type == MAP_TYPE_DOUBLE || mf->type == MAP_TYPE_SINGLE) {
            double dval = (val && (val->type == VAL_NUMBER || val->type == VAL_INTEGER)) ? val->as.number : 0.0;
            if (flen >= (int)sizeof(double)) {
                memcpy(dst, &dval, sizeof(double));
            } else if (flen >= (int)sizeof(float)) {
                float fval = (float)dval;
                memcpy(dst, &fval, sizeof(float));
            }
        } else if (mf->type == MAP_TYPE_INTEGER) {
            int ival = (val && (val->type == VAL_NUMBER || val->type == VAL_INTEGER)) ? (int)val->as.number : 0;
            if (flen >= (int)sizeof(int)) {
                memcpy(dst, &ival, sizeof(int));
            } else if (flen >= 2) {
                short sval = (short)ival;
                memcpy(dst, &sval, sizeof(short));
            }
        }
    }
}

static bool parse_map_field_spec(VMContext *vm, LexerContext *lex, const char *map_name, BppError *err) {
    BppToken tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) return false;

    char token_str[64];
    size_t tlen = (tok.length < sizeof(token_str) - 1) ? tok.length : sizeof(token_str) - 1;
    memcpy(token_str, tok.start, tlen);
    token_str[tlen] = '\0';

    MapFieldType type = MAP_TYPE_DOUBLE;
    int field_len = 8;
    char var_name[64] = {0};

    if (platform_strcasecmp(token_str, "STRING") == 0) {
        type = MAP_TYPE_STRING;
        field_len = 32;
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) return false;
        tlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.start, tlen);
        var_name[tlen] = '\0';
    } else if (platform_strcasecmp(token_str, "INTEGER") == 0 || platform_strcasecmp(token_str, "LONG") == 0) {
        type = MAP_TYPE_INTEGER;
        field_len = 4;
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) return false;
        tlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.start, tlen);
        var_name[tlen] = '\0';
    } else if (platform_strcasecmp(token_str, "DOUBLE") == 0 || platform_strcasecmp(token_str, "REAL") == 0) {
        type = MAP_TYPE_DOUBLE;
        field_len = 8;
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) return false;
        tlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.start, tlen);
        var_name[tlen] = '\0';
    } else if (platform_strcasecmp(token_str, "SINGLE") == 0) {
        type = MAP_TYPE_SINGLE;
        field_len = 4;
        tok = lex_next(lex);
        if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) return false;
        tlen = (tok.length < sizeof(var_name) - 1) ? tok.length : sizeof(var_name) - 1;
        memcpy(var_name, tok.start, tlen);
        var_name[tlen] = '\0';
    } else {
        // Standard variable name with type sigil or default
        strncpy(var_name, token_str, sizeof(var_name) - 1);
        char last_ch = var_name[strlen(var_name) - 1];
        if (last_ch == '$') {
            type = MAP_TYPE_STRING;
            field_len = 32;
        } else if (last_ch == '%') {
            type = MAP_TYPE_INTEGER;
            field_len = 4;
        } else if (last_ch == '#') {
            type = MAP_TYPE_DOUBLE;
            field_len = 8;
        } else if (last_ch == '!') {
            type = MAP_TYPE_SINGLE;
            field_len = 4;
        } else {
            type = MAP_TYPE_DOUBLE;
            field_len = 8;
        }
    }

    // Check for explicit length: var$ = len or var$ * len
    BppToken eq = lex_peek(lex);
    if (eq.type == TOK_EQ || eq.type == TOK_MUL) {
        lex_next(lex); // Consume '=' or '*'
        BValue len_val = eval_expression(vm, lex, err);
        if (err->code == 0 && (len_val.type == VAL_NUMBER || len_val.type == VAL_INTEGER)) {
            field_len = (int)len_val.as.number;
            if (field_len <= 0) field_len = 1;
        }
    }

    map_registry_add_field(vm, map_name, var_name, type, field_len);
    return true;
}

BppError stmt_map_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null context";
        return err;
    }

    BppToken tok = lex_next(lex);
    if (tok.type != TOK_LPAREN) {
        err.code = 2; err.message = "Expected '(' after MAP";
        return err;
    }

    tok = lex_next(lex);
    if (tok.type != TOK_IDENT && tok.type != TOK_KEYWORD) {
        err.code = 2; err.message = "Expected MAP buffer name";
        return err;
    }

    char map_name[64];
    size_t mlen = (tok.length < sizeof(map_name) - 1) ? tok.length : sizeof(map_name) - 1;
    memcpy(map_name, tok.start, mlen);
    map_name[mlen] = '\0';

    tok = lex_next(lex);
    if (tok.type != TOK_RPAREN) {
        err.code = 2; err.message = "Expected ')' after MAP buffer name";
        return err;
    }

    // Check if multi-line MAP block or single-line field list
    tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        // Multi-line MAP...MAPEND block: parse subsequent lines until MAPEND
        MemoryContext *mem = vm_get_mem(vm);
        size_t count = 0;
        BppProgramLine *lines = mem_program_get_all(mem, &count);
        BppLineNumber cur_ln = vm_get_current_line(vm);

        size_t start_idx = 0;
        for (size_t i = 0; i < count; ++i) {
            if (lines[i].line_number == cur_ln) { start_idx = i; break; }
        }

        for (size_t i = start_idx + 1; i < count; ++i) {
            LexerContext *scan = lex_init(mem, lines[i].text);
            if (!scan) continue;
            BppToken stok = lex_next(scan);
            if (stok.type == TOK_NUMBER) stok = lex_next(scan);

            if ((stok.type == TOK_KEYWORD && stok.as.keyword == KW_MAPEND) ||
                (stok.type == TOK_IDENT && stok.length == 6 && platform_strncasecmp(stok.start, "MAPEND", 6) == 0)) {
                lex_shutdown(scan);
                if (i + 1 < count) {
                    vm_jump(vm, lines[i + 1].line_number, NULL);
                }
                break;
            }

            // Parse fields in line
            LexerContext *field_lex = lex_init(mem, lines[i].text);
            if (field_lex) {
                BppToken ftok = lex_next(field_lex);
                if (ftok.type == TOK_NUMBER) ftok = lex_peek(field_lex);
                while (ftok.type != TOK_EOF && ftok.type != TOK_EOL) {
                    parse_map_field_spec(vm, field_lex, map_name, &err);
                    BppToken sep = lex_peek(field_lex);
                    if (sep.type == TOK_COMMA) lex_next(field_lex);
                    ftok = lex_peek(field_lex);
                }
                lex_shutdown(field_lex);
            }
            lex_shutdown(scan);
        }
        return err;
    }

    // Single-line MAP (map_name) field1, field2...
    while (true) {
        if (!parse_map_field_spec(vm, lex, map_name, &err)) {
            break;
        }
        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) {
            lex_next(lex);
            continue;
        }
        break;
    }

    return err;
}

BppError stmt_mapend_handler(VMContext *vm, LexerContext *lex) {
    (void)vm;
    (void)lex;
    BppError err;
    memset(&err, 0, sizeof(err));
    return err;
}
