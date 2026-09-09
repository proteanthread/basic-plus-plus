// FILENAME: defseg.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (segmented_mem.h, segmented_mem.c, string.h)
// NEEDS: libengine (defseg.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the DEFSEG statement in BASIC++.
//
// ---- Includes ----

#include "statements/system/hardware/defseg.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"
#include "memory/segmented_mem.h"
#include "runtime/language_descriptor.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

static const LangDesc g_def_seg_desc = {
    .name = "DEF SEG",
    .category = "Memory Management",
    .syntax = "DEF SEG [= address]",
    .description = "Sets the current segment address for PEEK, POKE, BLOAD, BSAVE, and CALL.",
    .error_summary = "Error 5: Illegal Function Call, Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_SYSTEM,
    .type = FEATURE_STATEMENT
};

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));

    BppToken tok = lex_peek(lex);
    // Check for optional 'SEG' identifier if invoked from DEF
    if (tok.type == TOK_IDENT && tok.length == 3 && runtime_strncasecmp(tok.start, "SEG", 3) == 0) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // Check for optional '='
    if (tok.type == TOK_EQ) {
        lex_next(lex);
        tok = lex_peek(lex);
    }

    // If no address specified (bare DEF SEG), reset default segment to 0
    if (tok.type == TOK_EOL || tok.type == TOK_EOF || tok.type == TOK_BACKSLASH) {
#ifndef BASIC_LITE_BUILD
        vmem_set_def_seg(vm_get_vmem(vm), 0);
#endif
        return err;
    }

    // Evaluate segment address
    BValue addr_val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (addr_val.type != VAL_NUMBER && addr_val.type != VAL_INTEGER) {
        if (addr_val.type == VAL_STRING && addr_val.as.string) {
            str_release(vm_get_str(vm), addr_val.as.string);
        }
        err.code = 13; err.message = "DEF SEG address must be numeric";
        return err;
    }

    uint16_t seg = (uint16_t)addr_val.as.number;
#ifndef BASIC_LITE_BUILD
    vmem_set_def_seg(vm_get_vmem(vm), seg);
#else
    (void)seg;
#endif

    return err;
}

void stmt_defseg_register(void) {
    lang_desc_register(&g_def_seg_desc);
}
