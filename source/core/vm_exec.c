#include "pcode.h"
#include <stdio.h>

int vm_exec_pcode(RuntimeState *rt, PCodeProgram *pcode) {
    int pc = 0;
    if (!pcode || !pcode->instrs) return -1;
    
    while(pc < pcode->count) {
        PCodeInstr *inst = &pcode->instrs[pc++];
        switch(inst->op) {
            case PCODE_HALT:
                return 0;
            case PCODE_NOP:
                break;
            default:
                /* Not implemented yet in this stub */
                break;
        }
    }
    return 0;
}
