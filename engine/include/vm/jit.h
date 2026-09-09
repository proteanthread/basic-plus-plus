// FILENAME: jit.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_stmt_ctrl.c, exec_interrupt.c, jit_vm.c)
// NEEDS: libcore, libkernel, libengine
// Implements core interface definitions and types for interpreter JIT and AOT.
//
// ---- Includes ----

#ifndef ENGINE_VM_JIT_H
#define ENGINE_VM_JIT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "types/types.h"
#include "types/errors.h"

struct VMContext;
struct EvalAstNode;

#define JIT_MODE_OFF      0
#define JIT_MODE_AUTO     1
#define JIT_MODE_BYTECODE 2
#define JIT_MODE_NATIVE   3

typedef enum {
    JIT_OP_NOP = 0,
    JIT_OP_LOAD_CONST_NUM,
    JIT_OP_LOAD_CONST_STR,
    JIT_OP_LOAD_VAR_FAST,
    JIT_OP_STORE_VAR_FAST,
    JIT_OP_LOAD_VAR_NAME,
    JIT_OP_STORE_VAR_NAME,
    JIT_OP_ADD,
    JIT_OP_SUB,
    JIT_OP_MUL,
    JIT_OP_DIV,
    JIT_OP_MOD,
    JIT_OP_NEG,
    JIT_OP_CMP_EQ,
    JIT_OP_CMP_NE,
    JIT_OP_CMP_LT,
    JIT_OP_CMP_GT,
    JIT_OP_CMP_LE,
    JIT_OP_CMP_GE,
    JIT_OP_JMP,
    JIT_OP_JMP_IF_ZERO,
    JIT_OP_JMP_IF_NOT_ZERO,
    JIT_OP_FOR_STEP,
    JIT_OP_ARRAY_READ,
    JIT_OP_ARRAY_STORE,
    JIT_OP_PRINT_NUM,
    JIT_OP_PRINT_STR,
    JIT_OP_PRINT_NL,
    JIT_OP_HALT
} JitOpcode;

typedef struct {
    JitOpcode opcode;
    union {
        double num;
        const char *str;
        double *var_ptr;
        char name[64];
        int target_ip;
        struct {
            double *counter_ptr;
            double step;
            double limit;
            int loop_body_ip;
        } for_info;
    } as;
    int line_num;
} JitInstr;

typedef struct {
    JitInstr *instrs;
    size_t count;
    size_t capacity;
} JitBytecodeProgram;

typedef BppError (*JitNativeFn)(struct VMContext *vm, void *args);

typedef struct JitBlock {
    uint32_t hot_count;
    int level;
    JitBytecodeProgram bytecode;
    JitNativeFn native_fn;
    void *dl_handle;
    char native_lib_path[256];
    struct JitBlock *next;
} JitBlock;

// Management and execution interface
void jit_init(struct VMContext *vm);
void jit_shutdown(struct VMContext *vm);
int  jit_get_mode(struct VMContext *vm);
void jit_set_mode(struct VMContext *vm, int mode);
bool jit_get_fast_mode(struct VMContext *vm);
void jit_set_fast_mode(struct VMContext *vm, bool fast);
void jit_notify_loop_iteration(struct VMContext *vm, struct EvalAstNode *loop_node);

JitBytecodeProgram *jit_compile_ast(struct VMContext *vm, struct EvalAstNode *ast);
void jit_free_bytecode(JitBytecodeProgram *prog);
BppError jit_execute_bytecode(struct VMContext *vm, const JitBytecodeProgram *prog);
BppError jit_execute_for_loop(struct VMContext *vm, struct EvalAstNode *for_node, bool *handled);
BppError jit_compile_and_run_aot(struct VMContext *vm);

#endif // ENGINE_VM_JIT_H
