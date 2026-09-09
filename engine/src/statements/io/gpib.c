// FILENAME: gpib.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, sys_fn.c)
// NEEDS: libkernel (vm.h, eval.h, lexer.h, errors.h)
// Provides runtime implementation for Tektronix 4050 GPIB I/O statements and functions.
//
// ---- Includes ----

#include "statements/io/gpib.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/strings.h"
#include "runtime/variables.h"

#define GPIB_MAX_DEVICES 32
#define GPIB_BUF_SIZE 512

static uint8_t s_gpib_buf[GPIB_MAX_DEVICES][GPIB_BUF_SIZE];
static int s_gpib_head[GPIB_MAX_DEVICES];
static int s_gpib_tail[GPIB_MAX_DEVICES];
static int s_gpib_count[GPIB_MAX_DEVICES];

static void gpib_write_byte(int dev, uint8_t b) {
    if (dev < 0 || dev >= GPIB_MAX_DEVICES) dev = 0;
    if (s_gpib_count[dev] < GPIB_BUF_SIZE) {
        s_gpib_buf[dev][s_gpib_head[dev]] = b;
        s_gpib_head[dev] = (s_gpib_head[dev] + 1) % GPIB_BUF_SIZE;
        s_gpib_count[dev]++;
    }
}

static uint8_t gpib_read_byte(int dev) {
    if (dev < 0 || dev >= GPIB_MAX_DEVICES) dev = 0;
    if (s_gpib_count[dev] > 0) {
        uint8_t b = s_gpib_buf[dev][s_gpib_tail[dev]];
        s_gpib_tail[dev] = (s_gpib_tail[dev] + 1) % GPIB_BUF_SIZE;
        s_gpib_count[dev]--;
        return b;
    }
    return 0;
}

static const LangDesc g_wbyte_desc = {
    .name = "WBYTE",
    .category = "Hardware & I/O",
    .syntax = "WBYTE [@dev [, sec] :] byte1 [, byte2 ...] | WBYTE[dev, sec] byte1 ...",
    .description = "Transmits raw bytes across GPIB IEEE-488 bus to instrumentation (Tektronix 4050 series).",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_rbyte_desc = {
    .name = "RBYTE",
    .category = "Hardware & I/O",
    .syntax = "RBYTE [@dev [, sec] :] var1 [, var2 ...] | RBYTE[dev, sec] var1 ...",
    .description = "Receives raw bytes across GPIB IEEE-488 bus from instrumentation (Tektronix 4050 series).",
    .error_summary = "Error 2: Syntax Error, Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_gpib_register(void) {
    lang_desc_register(&g_wbyte_desc);
    lang_desc_register(&g_rbyte_desc);
}

static void parse_gpib_header(VMContext *vm, LexerContext *lex, int *dev, int *sec, BppError *err) {
    *dev = 0;
    *sec = 0;
    BppToken tok = lex_peek(lex);

    if (tok.type == TOK_AT) {
        lex_next(lex); // consume @
        BValue vdev = eval_expression(vm, lex, err);
        if (err->code != 0) return;
        *dev = (int)vdev.as.number;

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BValue vsec = eval_expression(vm, lex, err);
            if (err->code != 0) return;
            *sec = (int)vsec.as.number;
        }
        if (lex_peek(lex).type == TOK_EOL && lex_peek(lex).start && lex_peek(lex).start[0] == ':') {
            lex_next(lex); // consume :
        }
    } else if (tok.type == TOK_LBRACKET) {
        lex_next(lex); // consume [
        BValue vdev = eval_expression(vm, lex, err);
        if (err->code != 0) return;
        *dev = (int)vdev.as.number;

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BValue vsec = eval_expression(vm, lex, err);
            if (err->code != 0) return;
            *sec = (int)vsec.as.number;
        }
        if (lex_peek(lex).type == TOK_RBRACKET) {
            lex_next(lex); // consume ]
        }
    }
}

BppError stmt_wbyte_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_WBYTE) {
        lex_next(lex);
    }

    int dev = 0, sec = 0;
    parse_gpib_header(vm, lex, &dev, &sec, &err);
    if (err.code != 0) return err;

    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        BValue val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        if (val.type == VAL_STRING && val.as.string) {
            size_t slen = str_len(val.as.string);
            const char *s = str_data(val.as.string);
            for (size_t i = 0; i < slen; i++) {
                gpib_write_byte(dev, (uint8_t)s[i]);
            }
        } else {
            gpib_write_byte(dev, (uint8_t)((int)val.as.number & 0xFF));
        }

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

BppError stmt_rbyte_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm || !lex) return err;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_RBYTE) {
        lex_next(lex);
    }

    int dev = 0, sec = 0;
    parse_gpib_header(vm, lex, &dev, &sec, &err);
    if (err.code != 0) return err;

    while (lex_peek(lex).type != TOK_EOL && lex_peek(lex).type != TOK_EOF) {
        BppToken var_tok = lex_next(lex);
        if (var_tok.type != TOK_IDENT) {
            err.code = 2;
            err.message = "Expected variable name in RBYTE";
            return err;
        }

        char vname[128];
        size_t nlen = (var_tok.length < sizeof(vname) - 1) ? var_tok.length : sizeof(vname) - 1;
        runtime_memcpy(vname, var_tok.start, nlen);
        vname[nlen] = '\0';

        uint8_t b = gpib_read_byte(dev);
        BValue bval;
        runtime_memset(&bval, 0, sizeof(bval));
        bval.type = VAL_NUMBER;
        bval.as.number = (double)b;
        var_assign(vm_get_var(vm), vname, bval);

        if (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
        } else {
            break;
        }
    }

    return err;
}

BValue func_wbyte_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (arg_count <= 0) {
        err->code = 5;
        err->message = "WBYTE expects at least 1 argument";
        return res;
    }

    int dev = 0;
    int start_idx = 0;
    if (arg_count >= 2) {
        dev = (int)args[0].as.number;
        start_idx = 1;
    }

    int count = 0;
    for (int i = start_idx; i < arg_count; i++) {
        if (args[i].type == VAL_STRING && args[i].as.string) {
            size_t slen = str_len(args[i].as.string);
            const char *s = str_data(args[i].as.string);
            for (size_t k = 0; k < slen; k++) {
                gpib_write_byte(dev, (uint8_t)s[k]);
                count++;
            }
        } else {
            gpib_write_byte(dev, (uint8_t)((int)args[i].as.number & 0xFF));
            count++;
        }
    }

    res.as.number = (double)count;
    return res;
}

BValue func_rbyte_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)uname;
    (void)err;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    int dev = 0;
    if (arg_count >= 1 && args[0].type != VAL_STRING) {
        dev = (int)args[0].as.number;
    }

    uint8_t b = gpib_read_byte(dev);
    res.as.number = (double)b;
    return res;
}
