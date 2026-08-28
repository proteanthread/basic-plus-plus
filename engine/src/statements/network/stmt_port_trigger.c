// FILENAME: stmt_port_trigger.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (eval.h, eval.c, events_net.h, events_net.c)
// NEEDS: libengine (stmt_port_trigger.h)
// Implements PORT statement for port knocking, triggering and trapping.
//
// ---- Includes ----

#include "statements/network/stmt_port_trigger.h"
#include "vm/events_net.h"
#include "eval/eval.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_port_trigger_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    bool is_trigger = false;
    bool is_fire = false;

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_PERIOD) {
        lex_next(lex);
        BppToken sub = lex_peek(lex);
        if (sub.type == TOK_IDENT || sub.type == TOK_KEYWORD) {
            if (sub.length == 7 && runtime_strncasecmp(sub.start, "TRIGGER", 7) == 0) is_trigger = true;
            if (sub.length == 4 && runtime_strncasecmp(sub.start, "FIRE", 4) == 0) is_fire = true;
            lex_next(lex);
        }
    }

    if (is_trigger) {
        // PORT.TRIGGER host$, p1 [, p2 [, p3 ...]]
        BValue host_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;

        while (lex_peek(lex).type == TOK_COMMA) {
            lex_next(lex);
            BValue p_val = eval_expression(vm, lex, &err);
            if (err.code != 0) {
                if (host_val.type == VAL_STRING) str_release(vm_get_str(vm), host_val.as.string);
                return err;
            }
            if (p_val.type == VAL_STRING && p_val.as.string) {
                str_release(vm_get_str(vm), p_val.as.string);
            }
            (void)p_val;
        }
        if (host_val.type == VAL_STRING) str_release(vm_get_str(vm), host_val.as.string);
        return err;
    }

    if (is_fire) {
        // PORT.FIRE port_num
        BValue p_val = eval_expression(vm, lex, &err);
        if (err.code == 0 && p_val.type == VAL_NUMBER) {
            vm_trigger_port_event(vm, (int)p_val.as.number);
        }
        return err;
    }

    // Handle PORT(p) ON / OFF / STOP
    if (tok.type == TOK_LPAREN) {
        lex_next(lex);
        BValue p_val = eval_expression(vm, lex, &err);
        if (err.code != 0) return err;
        int port = (p_val.type == VAL_NUMBER) ? (int)p_val.as.number : 0;
        if (lex_peek(lex).type == TOK_RPAREN) lex_next(lex);

        BppToken action = lex_peek(lex);
        int state = 1; // ON
        if (action.type == TOK_KEYWORD || action.type == TOK_IDENT) {
            if ((action.type == TOK_KEYWORD && action.as.keyword == KW_OFF) ||
                (action.length == 3 && runtime_strncasecmp(action.start, "OFF", 3) == 0)) {
                state = 0;
            } else if ((action.type == TOK_KEYWORD && action.as.keyword == KW_STOP) ||
                       (action.length == 4 && runtime_strncasecmp(action.start, "STOP", 4) == 0)) {
                state = 2;
            }
            lex_next(lex);
        }
        vm_set_port_trap(vm, port, 0, state);
        return err;
    }

    err.code = 5;
    err.message = "Syntax error in PORT statement";
    return err;
}

void stmt_port_trigger_register(void) {
    // Registered in VM dispatch
}
