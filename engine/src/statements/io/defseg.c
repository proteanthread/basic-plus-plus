// FILENAME: defseg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (segmented_mem.h, segmented_mem.c, string.h)
// NEEDS: libengine (eval.h, eval.c, stmt.h, string.c, vm.h)
// Provides runtime implementation for the DEFSEG statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "runtime/micro_lib_metadata.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "memory/segmented_mem.h"
#include <string.h>

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    // Check for '=' (optional in some implementations, but standard GW-BASIC requires it or just space)
    if (lex_peek(lex).type == TOK_EQ) {
        lex_next(lex);
    }
    
    // Evaluate address
    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (addr_val.type == VAL_STRING) {
        err.code = 13; err.message = "DEF SEG address must be numeric";
        return err;
    }
    
    uint16_t seg = (uint16_t)addr_val.as.number;
    vmem_set_def_seg(vm_get_vmem(vm), seg);
    
    return err;
}

void stmt_defseg_register(void) {
    MicroLibMetadata meta = {
        .name = "DEF SEG",
        .category = "Memory Management",
        .syntax = "DEF SEG [= address]",
        .help_text = "Sets the current segment address for PEEK, POKE, BLOAD, BSAVE, and CALL.",
        .error_codes = "Error 5: Illegal Function Call (segment address out of bounds)"
    };
    microlib_register(&meta);
}
