// FILENAME: stmt_gemini.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (gemini.h, gemini.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt_gemini.h, string.c)
// NEEDS: libengine (vm.h)
// Implements GEMINI statement for hosting Gemini protocol capsules.
//
// ---- Includes ----

#include "statements/network/stmt_gemini.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/gemini.h"
#include <string.h>

BppError stmt_gemini_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    bool is_cert = false;
    bool is_tofu = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 4 && strncasecmp(sub.start, "CERT", 4) == 0) {
                is_cert = true;
                lex_next(lex);
            } else if (sub.length == 4 && strncasecmp(sub.start, "TOFU", 4) == 0) {
                is_tofu = true;
                lex_next(lex);
            } else {
                lex_next(lex);
            }
        }
    }

    if (is_tofu) {
        BppToken opt = lex_peek(lex);
        if (opt.type == TOK_IDENT || opt.type == TOK_KEYWORD) {
            if (opt.length == 2 && strncasecmp(opt.start, "ON", 2) == 0) {
                net_gemini_set_tofu_enabled(true);
                lex_next(lex);
            } else if (opt.length == 3 && strncasecmp(opt.start, "OFF", 3) == 0) {
                net_gemini_set_tofu_enabled(false);
                lex_next(lex);
            }
        }
        return err;
    }

    if (is_cert) {
        BValue cert_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        const char *cert_path = (cert_val.type == VAL_STRING) ? str_data(cert_val.as.string) : NULL;
        const char *key_path = NULL;

        tok = lex_peek(lex);
        if (tok.type == TOK_COMMA) {
            lex_next(lex);
            BValue key_val = eval_expression(vm, lex, &err);
            if (err.code == 0 && key_val.type == VAL_STRING) {
                key_path = str_data(key_val.as.string);
            }
            net_gemini_set_client_cert(cert_path, key_path);
            if (key_val.type == VAL_STRING) str_release(vm_get_str(vm), key_val.as.string);
        } else {
            net_gemini_set_client_cert(cert_path, NULL);
        }

        if (cert_val.type == VAL_STRING) str_release(vm_get_str(vm), cert_val.as.string);
        return err;
    }

    int port = 1965;
    BppToken peek = lex_peek(lex);
    if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
        BValue port_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (port_val.type == VAL_NUMBER) port = (int)port_val.as.number;
    }

    tok = lex_peek(lex);
    if (tok.type == TOK_COMMA) lex_next(lex);

    const char *root_dir = ".";
    peek = lex_peek(lex);
    if (peek.type != TOK_EOL && peek.type != TOK_EOF) {
        BValue root_val = eval_expression(vm, lex, &err);
        if (err.code == 0 && root_val.type == VAL_STRING) {
            root_dir = str_data(root_val.as.string);
            net_gemini_serve(port, root_dir);
            str_release(vm_get_str(vm), root_val.as.string);
            return err;
        }
    }

    net_gemini_serve(port, root_dir);
    return err;
}

void stmt_gemini_register(void) {
    static const MicroLibMetadata meta = {
        .name = "GEMINI",
        .category = "Network & Cloud",
        .syntax = "GEMINI.SERVE [port] [, root_dir$] | GEMINI.GET$(url$)",
        .help_text = "Hosts lightweight Gemini TLS capsules or fetches text/gemini documents.",
        .error_codes = "Error 2: Syntax Error, Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}
