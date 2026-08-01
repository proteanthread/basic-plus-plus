#include "bpp_stmt.h"
#include "bpp_vm.h"
#include "bpp_eval.h"
#include "bpp_segmented_mem.h"
#include <string.h>

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    memset(&err, 0, sizeof(err));

    /* Check for '=' (optional in some implementations, but standard GW-BASIC requires it or just space) */
    if (lex_peek(lex).type == TOK_EQ) {
        lex_next(lex);
    }
    
    /* Evaluate address */
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
