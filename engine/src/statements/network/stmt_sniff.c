// FILENAME: stmt_sniff.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (packet_sniff.h, packet_sniff.c, strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, stmt_sniff.h)
// Implements SNIFF statement handler for promiscuous packet capture.
//
// ---- Includes ----

#include "statements/network/stmt_sniff.h"
#include "runtime/packet_sniff.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_sniff_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_start = false;
    bool is_stop = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 5 && runtime_strncasecmp(sub.start, "START", 5) == 0) is_start = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "STOP", 4) == 0) is_stop = true;
            lex_next(lex);
        }
    }

    if (is_stop) {
        return packet_sniff_stop();
    }

    // Check ON or OFF keyword
    tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) ||
            (tok.length == 3 && runtime_strncasecmp(tok.start, "OFF", 3) == 0)) {
            lex_next(lex);
            return packet_sniff_stop();
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ON) ||
            (tok.length == 2 && runtime_strncasecmp(tok.start, "ON", 2) == 0)) {
            is_start = true;
            lex_next(lex);
        }
    }

    int ch = 1;
    char filter[64] = {0};
    tok = lex_peek(lex);
    if (tok.type != TOK_EOF && tok.type != TOK_EOL) {
        BValue ch_val = eval_expression(vm, lex, &err);
        if (err.code == 0 && ch_val.type == VAL_NUMBER) {
            ch = (int)ch_val.as.number;
        }
        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue f_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && f_val.type == VAL_STRING) {
                runtime_strncpy(filter, str_data(f_val.as.string), sizeof(filter) - 1);
                str_release(vm_get_str(vm), f_val.as.string);
            }
        }
    }

    return packet_sniff_start(ch, filter);
}

void stmt_sniff_register(void) {
    // Registered in VM dispatch
}
