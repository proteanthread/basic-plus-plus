// FILENAME: ask.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (ask.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the ASK statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/ask.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "device/vcon.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

BppError stmt_ask_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    if (!vm || !lex) {
        err.code = 5; err.message = "Null VM or lexer context";
        return err;
    }

    int channel = 0;
    BppToken tok = lex_peek(lex);
    BppToken prop_tok;

    if (tok.type == TOK_HASH || tok.type == TOK_NUMBER) {
        if (tok.type == TOK_HASH) lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type == VAL_NUMBER) channel = (int)ch_val.as.number;

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_EOL || sep.type == TOK_COMMA) {
            lex_next(lex);
        }
        prop_tok = lex_next(lex);
    } else {
        prop_tok = lex_next(lex);
    }

    char prop_name[64] = {0};
    size_t plen = (prop_tok.length < sizeof(prop_name) - 1) ? prop_tok.length : sizeof(prop_name) - 1;
    memcpy(prop_name, prop_tok.start, plen);

    if (strcasecmp(prop_name, "MARGIN") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_MARGIN)) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name in ASK MARGIN";
            return err;
        }
        char var_name[64] = {0};
        size_t len = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        memcpy(var_name, var_tok.start, len);
        BValue val = { .type = VAL_NUMBER, .as.number = 80.0 };
        var_assign(vm_get_var(vm), var_name, val);
        return err;
    }

    if (strcasecmp(prop_name, "ZONEWIDTH") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_ZONE)) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected variable name in ASK ZONEWIDTH";
            return err;
        }
        char var_name[64] = {0};
        size_t len = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        memcpy(var_name, var_tok.start, len);
        BValue val = { .type = VAL_NUMBER, .as.number = 14.0 };
        var_assign(vm_get_var(vm), var_name, val);
        return err;
    }

    if (strcasecmp(prop_name, "CURSOR") == 0 || (prop_tok.type == TOK_KEYWORD && prop_tok.as.keyword == KW_CURSOR)) {
        BppToken r_tok = lex_next(lex);
        if (r_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected row variable in ASK CURSOR";
            return err;
        }
        char r_name[64] = {0};
        size_t rlen = (r_tok.length < sizeof(r_name) - 1) ? r_tok.length : sizeof(r_name) - 1;
        memcpy(r_name, r_tok.start, rlen);

        BppToken sep = lex_peek(lex);
        if (sep.type == TOK_COMMA) lex_next(lex);

        BppToken c_tok = lex_next(lex);
        if (c_tok.type != TOK_IDENT) {
            err.code = 2; err.message = "Expected column variable in ASK CURSOR";
            return err;
        }
        char c_name[64] = {0};
        size_t clen = (c_tok.length < sizeof(c_name) - 1) ? c_tok.length : sizeof(c_name) - 1;
        memcpy(c_name, c_tok.start, clen);

        int row = 1, col = 1;
        VConContext *vcon = vm_get_vcon(vm);
        if (vcon) {
            vcon_get_cursor(vcon, 0, &row, &col);
        }

        BValue r_val = { .type = VAL_NUMBER, .as.number = (double)row };
        BValue c_val = { .type = VAL_NUMBER, .as.number = (double)col };
        var_assign(vm_get_var(vm), r_name, r_val);
        var_assign(vm_get_var(vm), c_name, c_val);
        return err;
    }

    BppToken var_tok = lex_next(lex);
    if (var_tok.type != TOK_IDENT && var_tok.type != TOK_NUMBER) {
        err.code = 2; err.message = "Expected target variable in ASK statement";
        return err;
    }

    if (var_tok.type == TOK_IDENT) {
        char var_name[64] = {0};
        size_t len = (var_tok.length < sizeof(var_name) - 1) ? var_tok.length : sizeof(var_name) - 1;
        memcpy(var_name, var_tok.start, len);

        BValue val;
        memset(&val, 0, sizeof(val));
        val.type = VAL_NUMBER;
        val.as.number = (double)channel;

        var_assign(vm_get_var(vm), var_name, val);
    }
    return err;
}

void stmt_ask_register(void) {
    static const MicroLibMetadata meta = {
        .name = "ASK",
        .category = "File I/O",
        .syntax = "ASK #channel, property variable",
        .help_text = "ECMA-116 standard statement to query open file stream or channel attributes.",
        .error_codes = "Error 2: Syntax Error, Error 52: Bad File Number"
    };
    microlib_register(&meta);
}
