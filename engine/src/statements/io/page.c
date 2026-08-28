// FILENAME: page.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, micro_lib_metadata.h, micro_lib_metadata.c)
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
#include "runtime/micro_lib_metadata.h"
#include "types/errors.h"
#include <string.h>

void stmt_page_register(void) {
    static const MicroLibMetadata meta_page = {
        .name = "PAGE",
        .category = "Input / Output",
        .syntax = "PAGE [#channel,] [height]",
        .help_text = "Sets lines per page before form-feed or emits a page eject (DEC PDP-10 / ECMA-116).",
        .error_codes = "Error 2: Syntax error, Error 5: Illegal function call, Error 52: Bad file number"
    };
    microlib_register(&meta_page);

    static const MicroLibMetadata meta_nopage = {
        .name = "NOPAGE",
        .category = "Input / Output",
        .syntax = "NOPAGE [#channel]",
        .help_text = "Disables automatic page boundary wrapping and form feeds (DEC / ECMA-116).",
        .error_codes = "Error 2: Syntax error, Error 52: Bad file number"
    };
    microlib_register(&meta_nopage);

    static const MicroLibMetadata meta_nomargin = {
        .name = "NOMARGIN",
        .category = "Input / Output",
        .syntax = "NOMARGIN [#channel]",
        .help_text = "Disables automatic line width wrapping limit (DEC / ECMA-116).",
        .error_codes = "Error 2: Syntax error, Error 52: Bad file number"
    };
    microlib_register(&meta_nomargin);
}

BppError stmt_page_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_PAGE) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        // Emits form feed / page eject
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
    memset(&err, 0, sizeof(err));

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
    memset(&err, 0, sizeof(err));

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
