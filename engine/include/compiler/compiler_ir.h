// FILENAME: compiler_ir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (compiler_ir.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for compiler_ir within BASIC++.
//
// ---- Includes ----

#ifndef COMPILER_IR_H
#define COMPILER_IR_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
    IR_NOP,
    IR_LOAD_CONST_NUM,
    IR_LOAD_CONST_STR,
    IR_LOAD_VAR_NUM,
    IR_LOAD_VAR_STR,
    IR_STORE_VAR_NUM,
    IR_STORE_VAR_STR,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    IR_POW,
    IR_AND,
    IR_OR,
    IR_NOT,
    IR_XOR,
    IR_CMP_EQ,
    IR_CMP_NE,
    IR_CMP_LT,
    IR_CMP_GT,
    IR_CMP_LE,
    IR_CMP_GE,
    IR_JMP,
    IR_JMP_IF_TRUE,
    IR_JMP_IF_FALSE,
    IR_CALL,
    IR_RET,
    IR_PRINT,
    IR_INPUT,
    IR_GOTO,
    IR_GOSUB,
    IR_RETURN,
    IR_DIM,
    IR_ARRAY_LOAD,
    IR_ARRAY_STORE,
    IR_LABEL,
    IR_HALT
} CompilerIROpcode;

typedef struct {
    CompilerIROpcode opcode;
    union {
        double num;
        char *str;
        int label;
        int var_index;
    } operand;
    int line_number;
} CompilerIRInstr;

typedef struct {
    CompilerIRInstr *instrs;
    int count;
    int capacity;
} CompilerIRProgram;

CompilerIRProgram* compiler_ir_create(void);
int compiler_ir_emit(CompilerIRProgram *prog, CompilerIROpcode opcode, double num, char *str, int label, int var_index, int line_number);
void compiler_ir_destroy(CompilerIRProgram *prog);
const char *compiler_ir_opcode_name(CompilerIROpcode opcode);
bool compiler_ir_validate(const CompilerIRProgram *prog);
int compiler_ir_optimize(CompilerIRProgram *prog);

#endif // COMPILER_IR_H
