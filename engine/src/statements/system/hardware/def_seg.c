// FILENAME: def_seg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (def_seg.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the DEF_SEG statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/def_seg.h"
#include "eval/eval.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

static uint16_t g_current_def_seg = 0x0000;

uint16_t runtime_get_def_seg(void) {
    return g_current_def_seg;
}

void stmt_def_seg_register(void) {
    MicroLibMetadata meta = {
        .name = "DEF SEG",
        .category = "System & Memory",
        .syntax = "DEF SEG [= address%]",
        .help_text = "Defines the current 16-bit memory segment for PEEK, POKE, BLOAD, and BSAVE.",
        .error_codes = "Error 2: Syntax Error, Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}

BppError stmt_def_seg_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_EOL || tok.type == TOK_EOF) {
        g_current_def_seg = 0x0000; // Reset to default
        return err;
    }

    if (tok.type == TOK_EQ) {
        lex_next(lex); // Consume '='
    }

    BValue seg_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;

    if (seg_val.type == VAL_STRING) {
        if (seg_val.as.string) str_release(vm_get_str(vm), seg_val.as.string);
        err.code = 13;
        err.message = "Type mismatch in DEF SEG";
        return err;
    }

    int seg = (int)seg_val.as.number;
    if (seg < 0 || seg > 65535) {
        err.code = 5;
        err.message = "Illegal function call in DEF SEG";
        return err;
    }

    g_current_def_seg = (uint16_t)seg;
    return err;
}
