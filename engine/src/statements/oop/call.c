// FILENAME: call.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (perform.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (call.h, eval.h, eval.c, interrupt.h, interrupt.c)
// NEEDS: libengine (lexer.h, lexer.c, map.h, map.c, string.c, sub.h, sub.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CALL statement in BASIC++.
//
// ---- Includes ----

#include "statements/oop/call.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "device/vdev.h"
#include "security/security.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

#include "statements/oop/sub.h"
#include "statements/system/hardware/interrupt.h"
#include "runtime/map.h"

BppError stmt_call_handler(VMContext *vm, LexerContext *lex) {
    BppToken tok = lex_peek(lex);
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_INTERRUPT) ||
        (tok.type == TOK_IDENT && tok.length == 9 && strncasecmp(tok.start, "INTERRUPT", 9) == 0)) {
        lex_next(lex);
        return stmt_interrupt_handler(vm, lex);
    }
    if ((tok.type == TOK_KEYWORD && tok.as.keyword == KW_INTERRUPTX) ||
        (tok.type == TOK_IDENT && tok.length == 10 && strncasecmp(tok.start, "INTERRUPTX", 10) == 0)) {
        lex_next(lex);
        return stmt_interruptx_handler(vm, lex);
    }
    if (tok.type == TOK_IDENT && memchr(tok.start, '.', tok.length) != NULL) {
        BppError err;
        memset(&err, 0, sizeof(err));
        BValue res = eval_expression(vm, lex, &err);
        if (err.code == 0) {
            if (res.type == VAL_STRING && res.as.string) {
                str_release(vm_get_str(vm), res.as.string);
            } else if (res.type == VAL_MAP && res.as.map) {
                map_release(vm_get_str(vm), res.as.map);
            }
        }
        return err;
    }
    return vm_call_sub_procedure(vm, lex);
}

void stmt_call_register(void) {
    static const MicroLibMetadata meta = {
        .name = "CALL",
        .category = "Control Flow",
        .syntax = "CALL name [(argument_list)]",
        .help_text = "Transfers control to a SUB procedure or C extension routine.",
        .error_codes = "Error 2: Syntax Error, Error 35: Subprogram Not Defined, Error 36: Parameter Mismatch"
    };
    microlib_register(&meta);
}

