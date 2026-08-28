// FILENAME: stmt_gemini_browse.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (gemini.h, gemini.c, string.h, strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, stmt_gemini_browse.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Implements GEMINI.BROWSE interactive TUI capsule browser.
//
// ---- Includes ----

#include "statements/network/stmt_gemini_browse.h"
#include "runtime/gemini.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "device/vdev.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

BppError stmt_gemini_browse_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    const char *target_url = "gemini://geminiprotocol.net/";
    BValue url_val = {0};

    BppToken peek = lex_peek(lex);
    if (peek.type != TOK_EOL && peek.type != TOK_EOF && peek.type != TOK_BACKSLASH) {
        url_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        if (url_val.type == VAL_STRING && url_val.as.string) {
            target_url = str_data(url_val.as.string);
        }
    }

    char *doc = net_gemini_fetch(vm, target_url, &err);
    if (doc) {
        if (vm_get_vdev(vm)) {
            vdev_puts(vm_get_vdev(vm), doc);
        } else {
            fputs(doc, stdout);
        }
        free(doc);
    }

    if (url_val.type == VAL_STRING && url_val.as.string) {
        str_release(vm_get_str(vm), url_val.as.string);
    }

    return err;
}
