// FILENAME: selftest.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the SELFTEST statement in BASIC++.
//
// ---- Includes ----

#include "stmt/stmt.h"
#include "device/vdev.h"
#include "lexer/lexer.h"
#include "memory/memory.h"
#include "runtime/variables.h"
#include "runtime/strings.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

BppError stmt_selftest_handler(VMContext *vm, LexerContext *lex) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    (void)lex;

    VDevContext *vdev = vm_get_vdev(vm);
    vdev_puts(vdev, "--- Running Built-In Diagnostics SELFTEST ---\n");

    // Test 1: Lexer Subsystem
    {
        const char *src = "10 PRINT \"OK\"";
        LexerContext *tlex = lex_init(vm_get_mem(vm), src);
        if (!tlex) {
            err.code = 1001; err.message = "Lexer: Init failed"; return err;
        }

        BppToken t1 = lex_next(tlex);
        BppToken t2 = lex_next(tlex);
        BppToken t3 = lex_next(tlex);
        BppToken t4 = lex_next(tlex);

        if (t1.type != TOK_NUMBER || t1.as.number != 10.0) {
            lex_shutdown(tlex); err.code = 1002; err.message = "Lexer: Number token error"; return err;
        }
        if (t2.type != TOK_KEYWORD || t2.as.keyword != KW_PRINT) {
            lex_shutdown(tlex); err.code = 1003; err.message = "Lexer: Keyword token error"; return err;
        }
        if (t3.type != TOK_STRING || (t3.start && runtime_strncmp(t3.start, "OK", t3.length) != 0)) {
            lex_shutdown(tlex); err.code = 1004; err.message = "Lexer: String token error"; return err;
        }
        if (t4.type != TOK_EOF) {
            lex_shutdown(tlex); err.code = 1005; err.message = "Lexer: EOF token error"; return err;
        }

        lex_shutdown(tlex);
        vdev_puts(vdev, "  Lexer check: PASS\n");
    }

    // Test 2: Memory Allocator Subsystem
    {
        MemoryContext *mem = vm_get_mem(vm);
        void *p1 = mem_scratch_alloc(mem, 100);
        void *p2 = mem_scratch_alloc(mem, 200);
        if (!p1 || !p2) {
            err.code = 2001; err.message = "Memory: Scratch alloc failed"; return err;
        }
        mem_scratch_reset(mem);
        vdev_puts(vdev, "  Memory check: PASS\n");
    }

    // Test 3: String Heap Subsystem
    {
        StringContext *str = vm_get_str(vm);
        BppStringRef s1 = str_create(str, "TEST_STRING", 11);
        if (!s1) {
            err.code = 3001; err.message = "String: Create failed"; return err;
        }
        if (runtime_strcmp(str_data(s1), "TEST_STRING") != 0) {
            str_release(str, s1);
            err.code = 3002; err.message = "String: Data mismatch"; return err;
        }
        str_add_ref(s1);
        str_release(str, s1);
        str_release(str, s1);
        vdev_puts(vdev, "  String check: PASS\n");
    }

    // Test 4: Variable Storage Subsystem
    {
        VariableContext *var = vm_get_var(vm);
        BValue *v1 = var_lookup(var, "TESTVAR%", true);
        if (!v1) {
            err.code = 4001; err.message = "Variable: Lookup failed"; return err;
        }
        BValue val;
        val.type = VAL_INTEGER;
        val.as.number = 42.0;
        if (!var_assign(var, "TESTVAR%", val)) {
            err.code = 4002; err.message = "Variable: Assignment failed"; return err;
        }
        BValue *v2 = var_lookup(var, "TESTVAR%", false);
        if (!v2 || v2->type != VAL_INTEGER || v2->as.number != 42.0) {
            err.code = 4003; err.message = "Variable: Integrity check failed"; return err;
        }
        vdev_puts(vdev, "  Variable check: PASS\n");
    }

    // Test 5: Array Multi-Dimensional Subsystem
    {
        ArrayContext *arr = vm_get_arr(vm);
        int bounds[] = {5, 5};
        BppError dim_err = arr_dim(arr, "TESTARR%", 2, bounds);
        if (dim_err.code != 0) {
            err.code = 5001; err.message = "Array: Declaration failed"; return err;
        }
        int indices[] = {2, 3};
        BValue *elem = arr_get_element(arr, "TESTARR%", 2, indices, &err);
        if (err.code != 0 || !elem) {
            err.code = 5002; err.message = "Array: Retrieve element failed"; return err;
        }
        elem->type = VAL_INTEGER;
        elem->as.number = 100.0;

        BValue *elem2 = arr_get_element(arr, "TESTARR%", 2, indices, &err);
        if (!elem2 || elem2->type != VAL_INTEGER || elem2->as.number != 100.0) {
            err.code = 5003; err.message = "Array: Integrity check failed"; return err;
        }
        vdev_puts(vdev, "  Array check: PASS\n");
    }

    vdev_puts(vdev, "SELFTEST COMPLETED: ALL SYSTEM TESTS PASSED\n");
    return err;
}

void stmt_selftest_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SELFTEST",
        .category = "System",
        .syntax = "SELFTEST",
        .help_text = "Executes internal diagnostic suite verifying lexer, memory, string, variable, and array subsystems.",
        .error_codes = "Error 1001-5003: Subsystem Diagnostic Failure"
    };
    microlib_register(&meta);
}
