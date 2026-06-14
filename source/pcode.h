#ifndef BASICPP_PCODE_H
#define BASICPP_PCODE_H

#include "ast.h"
#include "vm.h"
#include "value.h"
#include "runtime.h"

typedef enum {
    PCODE_NOP = 0,
    PCODE_PUSH_CONST,
    PCODE_PUSH_VAR,
    PCODE_POP_VAR,
    PCODE_ADD,
    PCODE_SUB,
    PCODE_MUL,
    PCODE_DIV,
    PCODE_PRINT,
    PCODE_JUMP,
    PCODE_JUMP_IF_FALSE,
    PCODE_HALT,
    PCODE_RET,
    PCODE_CALL
} PCodeOp;

typedef struct {
    PCodeOp op;
    BValue operand;
} PCodeInstr;

typedef struct {
    PCodeInstr *instrs;
    int count;
    int capacity;
} PCodeProgram;

int pcode_compile(ProgramStore *program, PCodeProgram *out_pcode);
void pcode_free(PCodeProgram *pcode);

int vm_exec_pcode(RuntimeState *rt, PCodeProgram *pcode);

#endif /* BASICPP_PCODE_H */
