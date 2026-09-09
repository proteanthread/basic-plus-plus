// FILENAME: lprint.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libcore (using.h)
// NEEDS: libengine (eval.h, eval.c, lprint.h, string.c)
// NEEDS: libkernel (vprinter.h, vprinter.c)
// Provides runtime implementation for the LPRINT statement in BASIC++.
//
// ---- Includes ----

#include "statements/io/lprint.h"
#include "eval/eval.h"
#include "device/vprinter.h"
#include "runtime/strings.h"
#include "runtime/num_format.h"
#include "runtime/using.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_lprint_desc = {
    .name = "LPRINT",
    .category = "Line Printer I/O",
    .syntax = "LPRINT [USING format$;] expression_list [; | ,]",
    .description = "Prints formatted or unformatted data to the line printer.",
    .error_summary = "Error 2: Syntax Error, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_STATEMENT
};

void stmt_lprint_register(void) {
    lang_desc_register(&g_lprint_desc);
}

BppError stmt_lprint_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    bool is_using = false;
    if (tok.type == TOK_KEYWORD && tok.as.keyword == KW_USING) {
        lex_next(lex); // Consume USING
        is_using = true;
    }

    if (is_using) {
        print_using_internal(vm, lex, PRINT_CHANNEL_PRINTER);
    } else {
        bool last_was_sep = false;
        while (true) {
            tok = lex_peek(lex);
            if (tok.type == TOK_EOL || tok.type == TOK_EOF || (tok.type == TOK_KEYWORD && tok.as.keyword == KW_ELSE)) {
                break;
            }

            if (tok.type == TOK_COMMA) {
                lex_next(lex);
                vprinter_write_str("              ");
                last_was_sep = true;
                continue;
            }
            if (tok.type == TOK_SEMICOLON) {
                lex_next(lex);
                last_was_sep = true;
                continue;
            }

            BValue val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            if (val.type == VAL_STRING) {
                if (val.as.string) {
                    vprinter_write_str(str_data(val.as.string));
                    str_release(vm_get_str(vm), val.as.string);
                }
            } else if (val.type == VAL_COMPLEX) {
                char cbuf[128];
                char r_buf[64], i_buf[64];
                num_format_display(r_buf, sizeof(r_buf), val.as.complex_val.real, true, false);
                double imag = val.as.complex_val.imag;
                if (imag >= 0.0) {
                    num_format_display(i_buf, sizeof(i_buf), imag, false, false);
                    runtime_snprintf(cbuf, sizeof(cbuf), "%s+%sI ", r_buf, i_buf);
                } else {
                    num_format_display(i_buf, sizeof(i_buf), -imag, false, false);
                    runtime_snprintf(cbuf, sizeof(cbuf), "%s-%sI ", r_buf, i_buf);
                }
                vprinter_write_str(cbuf);
            } else {
                char nbuf[64];
                num_format_display(nbuf, sizeof(nbuf), val.as.number, true, true);
                vprinter_write_str(nbuf);
            }
            last_was_sep = false;
        }
        if (!last_was_sep) {
            vprinter_write_str("\n");
        }
    }

    return err;
}
