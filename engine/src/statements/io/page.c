// FILENAME: page.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, page.h, string.c)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the PAGE statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/page.h"
#include "eval/eval.h"
#include "runtime/file.h"
#include "device/vdev.h"
#include "device/vcon.h"
#include "runtime/language_descriptor.h"
#include "types/errors.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_page_desc = {
    .name = "PAGE",
    .category = "Input / Output",
    .syntax = "PAGE [#channel,] [height]",
    .description = "Sets lines per page before form-feed or emits a page eject (DEC PDP-10 / ECMA-116).",
    .error_summary = "Error 2: Syntax error, Error 5: Illegal function call, Error 52: Bad file number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_nopage_desc = {
    .name = "NOPAGE",
    .category = "Input / Output",
    .syntax = "NOPAGE [#channel]",
    .description = "Disables automatic page boundary wrapping and form feeds (DEC / ECMA-116).",
    .error_summary = "Error 2: Syntax error, Error 52: Bad file number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

static const LangDesc g_nomargin_desc = {
    .name = "NOMARGIN",
    .category = "Input / Output",
    .syntax = "NOMARGIN [#channel]",
    .description = "Disables automatic line width wrapping limit (DEC / ECMA-116).",
    .error_summary = "Error 2: Syntax error, Error 52: Bad file number",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_page_register(void) {
    lang_desc_register(&g_page_desc);

    lang_desc_register(&g_nopage_desc);

    lang_desc_register(&g_nomargin_desc);
}

BppError stmt_page_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PAGE) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        // Tektronix 4050 / ECMA-116: Clear screen and emit form feed / page eject
        VConContext *vcon = vm_get_vcon(vm);
        if (vcon) {
            vcon_clear_screen(vcon, 0, -3);
            vcon_locate(vcon, 0, 1, 1);
        }
        vdev_putc(vm_get_vdev(vm), '\f');
        return err;
    }

    int channel = -1;
    if (tok.type == TOK_HASH) {
        lex_next(lex); // Consume '#'
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type != VAL_NUMBER && ch_val.type != VAL_INTEGER) {
            if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        channel = (int)ch_val.as.number;

        BppToken comma = lex_peek(lex);
        if (comma.type == TOK_COMMA) {
            lex_next(lex);
        } else if (comma.type == TOK_EOF || comma.type == TOK_EOL) {
            // PAGE #ch emits form feed to file
            if (!file_is_open(vm_get_file(vm), channel)) {
                err.code = ERR_BAD_FILE_NUMBER;
                return err;
            }
            char ff = '\f';
            file_write(vm_get_file(vm), channel, &ff, 1);
            return err;
        }
    }

    tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL) {
        BValue h_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (h_val.type != VAL_NUMBER && h_val.type != VAL_INTEGER) {
            if (h_val.type == VAL_STRING && h_val.as.string) str_release(vm_get_str(vm), h_val.as.string);
            err.code = ERR_TYPE_MISMATCH;
            return err;
        }
        int height = (int)h_val.as.number;
        if (height < 1 || height > 65535) {
            err.code = ERR_ILLEGAL_FUNCTION_CALL;
            err.message = "PAGE height must be 1 to 65535";
            return err;
        }
        (void)channel;
        (void)height;
    }

    return err;
}

BppError stmt_nopage_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_NOPAGE) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_HASH) {
        lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
    }
    return err;
}

BppError stmt_nomargin_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_NOMARGIN) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_HASH) {
        lex_next(lex);
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (ch_val.type == VAL_STRING && ch_val.as.string) str_release(vm_get_str(vm), ch_val.as.string);
    } else {
        vm_set_margin(vm, 0); // 0 disables auto-wrapping
    }
    return err;
}
