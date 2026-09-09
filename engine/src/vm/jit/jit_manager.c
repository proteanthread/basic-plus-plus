// FILENAME: jit_manager.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ast_eval_stmt_ctrl.c, common_boot.c, exec_interrupt.c)
// NEEDS: libcore, libengine, libkernel
// Implements lifecycle, profiling, and mode orchestration for JIT and AOT.
//
// ---- Includes ----

#include "vm/jit.h"
#include "vm/vm.h"
#include "eval/ast.h"
#include "eval/ast_internal.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"

extern void jit_native_shutdown(void);

void jit_init(VMContext *vm) {
    if (!vm) return;
    const char *env_jit = platform_getenv("BASICPP_JIT");
    if (env_jit) {
        if (env_jit[0] == '0' || env_jit[0] == 'f' || env_jit[0] == 'F' || runtime_strcmp(env_jit, "off") == 0) {
            vm_set_jit_mode(vm, JIT_MODE_OFF);
        } else if (runtime_strcmp(env_jit, "bytecode") == 0) {
            vm_set_jit_mode(vm, JIT_MODE_BYTECODE);
        } else if (runtime_strcmp(env_jit, "native") == 0) {
            vm_set_jit_mode(vm, JIT_MODE_NATIVE);
        } else {
            vm_set_jit_mode(vm, JIT_MODE_AUTO);
        }
    } else {
        vm_set_jit_mode(vm, JIT_MODE_AUTO);
    }
}

void jit_shutdown(VMContext *vm) {
    if (!vm) return;
    jit_native_shutdown();
}

int jit_get_mode(VMContext *vm) {
    return vm_get_jit_mode(vm);
}

void jit_set_mode(VMContext *vm, int mode) {
    vm_set_jit_mode(vm, mode);
}

bool jit_get_fast_mode(VMContext *vm) {
    return vm_get_jit_fast(vm);
}

void jit_set_fast_mode(VMContext *vm, bool fast) {
    vm_set_jit_fast(vm, fast);
}

void jit_notify_loop_iteration(VMContext *vm, EvalAstNode *loop_node) {
    if (!vm || !loop_node) return;
    if (jit_get_mode(vm) == JIT_MODE_OFF) return;

    // Check if JIT bytecode is not yet compiled
    if (!loop_node->target_ast) {
        JitBytecodeProgram *prog = jit_compile_ast(vm, loop_node);
        if (prog) {
            loop_node->target_ast = (EvalAstNode *)prog;
        }
    }
}

BppError jit_compile_and_run_aot(VMContext *vm) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    if (!vm) return err;

    MemoryContext *mem = vm_get_mem(vm);
    if (!mem) return err;

    size_t count = 0;
    BppProgramLine *lines = mem_program_get_all(mem, &count);
    if (!lines || count == 0) return err;

    // AOT pass: Pre-parse and compile all lines into AST caches and JIT blocks
    for (size_t i = 0; i < count; i++) {
        if (!lines[i].ast_valid || lines[i].ast_cache == NULL) {
            size_t skipped = 0;
            EvalAstNode *parsed = eval_ast_try_compile_multiline_block(vm, lines, i, count, &skipped);
            if (!parsed && lines[i].text) {
                parsed = eval_ast_try_parse_line(vm, lines[i].text);
            }
            if (parsed) {
                lines[i].ast_cache = parsed;
                lines[i].ast_valid = true;
                lines[i].ast_skip_lines = skipped;

                // Pre-compile JIT bytecode for loops
                if (parsed->type == AST_NODE_FOR_LOOP && !parsed->target_ast) {
                    JitBytecodeProgram *prog = jit_compile_ast(vm, parsed);
                    if (prog) {
                        parsed->target_ast = (EvalAstNode *)prog;
                    }
                }
            }
        }
    }

    // Execute via standard VM program loop with AOT-warmed caches
    vm_run_program(vm);
    if (vm_has_error(vm)) {
        err = vm_get_error(vm);
    }
    return err;
}
