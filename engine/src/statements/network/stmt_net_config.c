// FILENAME: stmt_net_config.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, events_net.h, events_net.c)
// NEEDS: libengine (stmt_net_config.h, stmt_nil_bead.h, stmt_nil_bead.c)
// Implements NET.CONFIG statement for network interface and IP setup.
//
// ---- Includes ----

#include "statements/network/stmt_net_config.h"
#include "statements/network/stmt_nil_bead.h"
#include "eval/eval.h"
#include "vm/events_net.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_net_config_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_config = false;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 6 && runtime_strncasecmp(sub.start, "CONFIG", 6) == 0) is_config = true;
            if (sub.length == 6 && runtime_strncasecmp(sub.start, "UNPACK", 6) == 0) {
                return stmt_nil_unpack_handler(vm, lex);
            }
            lex_next(lex);
        }
    }

    // Check NET ON / OFF / STOP
    tok = lex_peek(lex);
    if (tok.type == TOK_KEYWORD || tok.type == TOK_IDENT) {
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_OFF) ||
            (tok.length == 3 && runtime_strncasecmp(tok.start, "OFF", 3) == 0)) {
            lex_next(lex);
            vm_set_net_trap(vm, 0, 0);
            return err;
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_ON) ||
            (tok.length == 2 && runtime_strncasecmp(tok.start, "ON", 2) == 0)) {
            lex_next(lex);
            vm_set_net_trap(vm, 0, 1);
            return err;
        }
        if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_STOP) ||
            (tok.length == 4 && runtime_strncasecmp(tok.start, "STOP", 4) == 0)) {
            lex_next(lex);
            vm_set_net_trap(vm, 0, 2);
            return err;
        }
    }

    if (is_config) {
        // NET.CONFIG iface$, ip$, netmask$, gateway$, dns$
        BValue iface_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        while (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BValue p_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                if (iface_val.type == VAL_STRING) str_release(vm_get_str(vm), iface_val.as.string);
                return err;
            }
            if (p_val.type == VAL_STRING) str_release(vm_get_str(vm), p_val.as.string);
        }

        if (iface_val.type == VAL_STRING) str_release(vm_get_str(vm), iface_val.as.string);
        return err;
    }

    err.code = 5;
    err.message = "Syntax error in NET statement";
    return err;
}

void stmt_net_config_register(void) {
    // Registered in VM dispatch
}
