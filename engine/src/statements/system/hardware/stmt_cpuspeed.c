// FILENAME: stmt_cpuspeed.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Statement implementation for CPUSPEED statement (e.g. CPUSPEED EMULATE).

#include "statements/system/hardware/stmt_cpuspeed.h"
#include "eval/functions/system/hardware/func_cpuspeed.h"
#include "hardware/speed_emulate.h"
#include "hardware/speed_db.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/memory/alloc.h"
#include "eval/eval.h"
#include <string.h>
#include <ctype.h>

static int case_cmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return s1 ? 1 : (s2 ? -1 : 0);
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

static const LangDesc g_stmt_cpuspeed_desc = {
    .name = "CPUSPEED",
    .category = "Hardware & Peripherals",
    .syntax = "CPUSPEED EMULATE <model> [, IO] | CPUSPEED EMULATE OFF | CPUSPEED{...}",
    .description = "Configures authentic CPU and I/O speed emulation for vintage folklore systems.",
    .error_summary = "Error 5: Illegal Function Call",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_STATEMENT
};

void stmt_cpuspeed_register(void) {
    lang_desc_register(&g_stmt_cpuspeed_desc);
}

BppError stmt_cpuspeed_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));
    BppToken tok = lex_peek(lex);

    if (tok.type == TOK_RPN_LITERAL) {
        BppToken rpn_tok = lex_next(lex);
        char *rpn_buf = (char *)mem_scratch_alloc(vm_get_mem(vm), rpn_tok.length + 3);
        if (rpn_buf) {
            memcpy(rpn_buf, rpn_tok.as.string, rpn_tok.length);
            rpn_buf[rpn_tok.length] = '}';
            rpn_buf[rpn_tok.length + 1] = '\0';
            LexerContext *sub_lex = lex_init(vm_get_mem(vm), rpn_buf);
            func_cpuspeed_eval_brace(vm, sub_lex, &err);
            lex_shutdown(sub_lex);
            return err;
        }
    }
    if (tok.type == TOK_LBRACE) {
        lex_next(lex);
        func_cpuspeed_eval_brace(vm, lex, &err);
        return err;
    }

    if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
        char subcmd[32];
        size_t slen = (tok.length < sizeof(subcmd) - 1) ? tok.length : sizeof(subcmd) - 1;
        memcpy(subcmd, tok.start, slen);
        subcmd[slen] = '\0';

        if (case_cmp(subcmd, "EMULATE") == 0 || case_cmp(subcmd, "THROTTLE") == 0) {
            lex_next(lex); // consume subcmd
            tok = lex_peek(lex);

            if (tok.type == TOK_IDENT || tok.type == TOK_KEYWORD) {
                char off_str[16];
                size_t olen = (tok.length < sizeof(off_str) - 1) ? tok.length : sizeof(off_str) - 1;
                memcpy(off_str, tok.start, olen);
                off_str[olen] = '\0';
                if (case_cmp(off_str, "OFF") == 0 || case_cmp(off_str, "NONE") == 0) {
                    lex_next(lex);
                    speed_emulate_set_active(false);
                    return err;
                }
            }

            BValue target_val = eval_expression(vm, lex, &err);
            if (err.code != 0) return err;

            bool throttle_io = false;
            if (lex_peek(lex).type == TOK_COMMA) {
                lex_next(lex); // consume comma
                BppToken io_tok = lex_next(lex);
                char io_name[16];
                size_t ilen = (io_tok.length < sizeof(io_name) - 1) ? io_tok.length : sizeof(io_name) - 1;
                memcpy(io_name, io_tok.start, ilen);
                io_name[ilen] = '\0';
                if (case_cmp(io_name, "IO") == 0 || case_cmp(io_name, "TRUE") == 0 || case_cmp(io_name, "ON") == 0) {
                    throttle_io = true;
                }
            }

            if (target_val.type == VAL_STRING && target_val.as.string) {
                if (!speed_emulate_set_target(str_data(target_val.as.string), throttle_io)) {
                    err.code = 5;
                    err.message = "Illegal function call: Unknown emulation target system";
                }
            } else if (target_val.type == VAL_NUMBER || target_val.type == VAL_INTEGER) {
                speed_emulate_set_mhz(target_val.as.number, throttle_io);
            }
            return err;
        }
    }

    err.code = 2;
    err.message = "Syntax error: Expected EMULATE <model> [, IO] or EMULATE OFF";
    return err;
}
