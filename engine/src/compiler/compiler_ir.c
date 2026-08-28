// FILENAME: compiler_ir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (alloc.h, alloc.c, compiler_ir.h, memops.h, memops.c)
// NEEDS: libcore (strops.h, strops.c)
// Provides core logic and interface definitions for compiler_ir within BASIC++.
//
// ---- Includes ----

#include "compiler/compiler_ir.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"

CompilerIRProgram* compiler_ir_create(void) {
    CompilerIRProgram *prog = (CompilerIRProgram *)runtime_calloc(1, sizeof(CompilerIRProgram));
    if (!prog) return NULL;
    
    prog->capacity = 256;
    prog->count = 0;
    prog->instrs = (CompilerIRInstr *)runtime_calloc(prog->capacity, sizeof(CompilerIRInstr));
    if (!prog->instrs) {
        runtime_free(prog);
        return NULL;
    }
    
    return prog;
}

int compiler_ir_emit(CompilerIRProgram *prog, CompilerIROpcode opcode, double num, char *str, int label, int var_index, int line_number) {
    if (!prog) return -1;
    
    if (prog->count >= prog->capacity) {
        int new_capacity = prog->capacity * 2;
        CompilerIRInstr *new_instrs = (CompilerIRInstr *)runtime_calloc(new_capacity, sizeof(CompilerIRInstr));
        if (!new_instrs) return -1;
        
        for (int i = 0; i < prog->count; i++) {
            new_instrs[i] = prog->instrs[i];
        }
        
        runtime_free(prog->instrs);
        prog->instrs = new_instrs;
        prog->capacity = new_capacity;
    }
    
    CompilerIRInstr *instr = &prog->instrs[prog->count++];
    instr->opcode = opcode;
    instr->line_number = line_number;
    
    if (str) {
        instr->operand.str = str;
    } else if (opcode == IR_LOAD_CONST_NUM) {
        instr->operand.num = num;
    } else if (opcode == IR_LABEL || opcode == IR_JMP || opcode == IR_JMP_IF_TRUE || opcode == IR_JMP_IF_FALSE || opcode == IR_GOTO || opcode == IR_GOSUB) {
        instr->operand.label = label;
    } else {
        instr->operand.var_index = var_index;
    }
    
    return 0;
}

void compiler_ir_destroy(CompilerIRProgram *prog) {
    if (!prog) return;
    
    if (prog->instrs) {
        for (int i = 0; i < prog->count; i++) {
            if (prog->instrs[i].opcode == IR_LOAD_CONST_STR && prog->instrs[i].operand.str) {
                runtime_free(prog->instrs[i].operand.str);
                prog->instrs[i].operand.str = NULL;
            }
        }
        runtime_free(prog->instrs);
        prog->instrs = NULL;
    }
    
    runtime_free(prog);
}

const char *compiler_ir_opcode_name(CompilerIROpcode opcode) {
    switch (opcode) {
        case IR_NOP: return "NOP";
        case IR_LOAD_CONST_NUM: return "LOAD_CONST_NUM";
        case IR_LOAD_CONST_STR: return "LOAD_CONST_STR";
        case IR_LOAD_VAR_NUM: return "LOAD_VAR_NUM";
        case IR_LOAD_VAR_STR: return "LOAD_VAR_STR";
        case IR_STORE_VAR_NUM: return "STORE_VAR_NUM";
        case IR_STORE_VAR_STR: return "STORE_VAR_STR";
        case IR_ADD: return "ADD";
        case IR_SUB: return "SUB";
        case IR_MUL: return "MUL";
        case IR_DIV: return "DIV";
        case IR_MOD: return "MOD";
        case IR_NEG: return "NEG";
        case IR_POW: return "POW";
        case IR_AND: return "AND";
        case IR_OR: return "OR";
        case IR_NOT: return "NOT";
        case IR_XOR: return "XOR";
        case IR_CMP_EQ: return "CMP_EQ";
        case IR_CMP_NE: return "CMP_NE";
        case IR_CMP_LT: return "CMP_LT";
        case IR_CMP_GT: return "CMP_GT";
        case IR_CMP_LE: return "CMP_LE";
        case IR_CMP_GE: return "CMP_GE";
        case IR_JMP: return "JMP";
        case IR_JMP_IF_TRUE: return "JMP_IF_TRUE";
        case IR_JMP_IF_FALSE: return "JMP_IF_FALSE";
        case IR_CALL: return "CALL";
        case IR_RET: return "RET";
        case IR_PRINT: return "PRINT";
        case IR_INPUT: return "INPUT";
        case IR_GOTO: return "GOTO";
        case IR_GOSUB: return "GOSUB";
        case IR_RETURN: return "RETURN";
        case IR_DIM: return "DIM";
        case IR_ARRAY_LOAD: return "ARRAY_LOAD";
        case IR_ARRAY_STORE: return "ARRAY_STORE";
        case IR_LABEL: return "LABEL";
        case IR_HALT: return "HALT";
        default: return "UNKNOWN";
    }
}

bool compiler_ir_validate(const CompilerIRProgram *prog) {
    if (!prog || !prog->instrs || prog->count < 0) return false;
    for (int i = 0; i < prog->count; i++) {
        if (prog->instrs[i].opcode < IR_NOP || prog->instrs[i].opcode > IR_HALT) {
            return false;
        }
    }
    return true;
}

int compiler_ir_optimize(CompilerIRProgram *prog) {
    if (!prog || !prog->instrs || prog->count <= 0) return 0;
    int opt_count = 0;
    // Constant folding and dead instruction peephole
    for (int i = 0; i < prog->count - 2; i++) {
        if (prog->instrs[i].opcode == IR_LOAD_CONST_NUM &&
            prog->instrs[i+1].opcode == IR_LOAD_CONST_NUM &&
            prog->instrs[i+2].opcode == IR_ADD) {
            double folded = prog->instrs[i].operand.num + prog->instrs[i+1].operand.num;
            prog->instrs[i].operand.num = folded;
            prog->instrs[i+1].opcode = IR_NOP;
            prog->instrs[i+2].opcode = IR_NOP;
            opt_count++;
            i += 2;
        }
    }
    return opt_count;
}
