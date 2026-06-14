#include "pcode.h"
#include <stdlib.h>
#include <stdio.h>

int pcode_compile(ProgramStore *program, PCodeProgram *out_pcode) {
    if (!program || !out_pcode) return -1;
    
    out_pcode->count = 0;
    out_pcode->capacity = 64;
    out_pcode->instrs = (PCodeInstr *)malloc(sizeof(PCodeInstr) * out_pcode->capacity);
    
    if (!out_pcode->instrs) {
        return -1;
    }

    /* Basic stub: Just insert a HALT instruction for now. 
     * In a full implementation, this would walk the AST 
     * and emit PCode operations using the AstNodes. */
     
    out_pcode->instrs[0].op = PCODE_HALT;
    out_pcode->count = 1;
    
    return 0;
}

void pcode_free(PCodeProgram *pcode) {
    if (!pcode) return;
    if (pcode->instrs) {
        free(pcode->instrs);
        pcode->instrs = NULL;
    }
    pcode->count = 0;
    pcode->capacity = 0;
}
