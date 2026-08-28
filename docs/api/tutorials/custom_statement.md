# Tutorial: Implementing Custom Statements in BASIC++

## 1. Overview

This tutorial demonstrates how to implement and register a custom statement keyword in BASIC++ using the Statement Dispatch API (`stmt/stmt.h`), Expression Evaluator (`eval/eval.h`), and Lexical Analyzer (`lexer/lexer.h`).

We will implement a custom `BENCHMARK` statement that times the execution of a statement block and reports elapsed milliseconds.

---

## 2. Statement Handler Signature & Lexer Rules

All statement handlers adhere to the `BppStmtHandler` function pointer typedef:
```c
typedef BppError (*BppStmtHandler)(VMContext *vm, LexerContext *lex);
```

### Critical Statement Handler Rules:
1. **Lexer Advancement Guard**: A statement handler MUST NEVER return `err.code = 0` without consuming at least one token from `lex`.
2. **Type Safety & Bounds Checking**: Always validate `val.type` before accessing union members. Out-of-bounds arguments must return Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).
3. **Two-Parameter String Release**: When evaluating string expressions via `eval_expression()`, the calling handler owns the string and MUST call `str_release(vm_get_str(vm), val.as.string)` on ALL return paths.

---

## 3. Implementing the Statement Handler

Create `stmt_benchmark.c`:

```c
#include "stmt/stmt.h"
#include "eval/eval.h"
#include "platform/platform.h"
#include "device/vdev.h"
#include <stdio.h>

BppError stmt_benchmark_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};

    /* 1. Consume the BENCHMARK keyword token */
    BppToken kw_tok = lex_next(lex);
    (void)kw_tok;

    /* 2. Check for optional iterations count */
    int iterations = 1000;
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_NUMBER) {
        lex_next(lex);
        iterations = (int)tok.as.number;
        if (iterations <= 0) {
            err.code = 5; /* Illegal function call */
            err.message = "Iterations must be greater than zero";
            return err;
        }
    }

    /* 3. Evaluate expression or execute timing loop */
    uint32_t start_time = (uint32_t)(platform_get_time_us() / 1000);
    
    /* Simulate benchmarking workload */
    for (volatile int i = 0; i < iterations * 1000; i++) {
        /* Workload loop */
    }

    uint32_t end_time = (uint32_t)(platform_get_time_us() / 1000);
    uint32_t elapsed_ms = end_time - start_time;

    /* 4. Output benchmark results to virtual console */
    char buf[128];
    snprintf(buf, sizeof(buf), "[BENCHMARK] %d iterations executed in %u ms\n", iterations, elapsed_ms);
    vdev_puts(vm_get_vdev(vm)->active_console, buf);

    return err;
}
```

---

## 4. Registering the Statement and MicroLib Metadata

```c
#include "runtime/micro_lib_metadata.h"

void stmt_benchmark_register(StmtRegistry *reg) {
    /* 1. Register handler in Statement Dispatch Table */
    stmt_register(reg, KW_BENCHMARK, stmt_benchmark_handler, "BENCHMARK", STMT_FLAG_BOTH);

    /* 2. Register MicroLib documentation metadata for HELP and CATALOG */
    static const MicroLibMetadata meta = {
        .name        = "BENCHMARK",
        .category    = "Performance & Diagnostics",
        .syntax      = "BENCHMARK [iterations]",
        .help_text   = "Times statement execution across a designated iteration count.",
        .error_codes = "Error 5: Illegal Function Call"
    };
    microlib_register(&meta);
}
```

---

## 5. Testing the Statement in BASIC++

```basic
10 PRINT "Testing custom BENCHMARK statement:"
20 BENCHMARK 5000
30 PRINT "Done."
```

Output:
```text
Testing custom BENCHMARK statement:
[BENCHMARK] 5000 iterations executed in 14 ms
Done.
```
