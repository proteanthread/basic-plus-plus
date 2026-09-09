<!--
Title:        C_Host_Embedding_API
Tier:         2
Applies to:   BASIC++ v6.5.2 (C17 Host Applications)
Authority:    engine/include/bpp_api.h
Generated:    no, manual reference
Status:       current
-->

# BASIC++ v6.5.2 C Host Embedding API Reference

## 1. HOST LANGUAGE INTEROPERABILITY OVERVIEW

BASIC++ provides a clean, decoupled ISO C17 API for embedding the virtual machine inside host C and C++ applications. The embedding interface is defined in `engine/include/bpp_api.h` and provides functions for instantiating isolated VM contexts, compiling and executing BASIC statements, loading script files, and bidirectionally exchanging data.

## 2. INITIALIZING & EMBEDDING THE VIRTUAL MACHINE

Host applications manage the interpreter lifecycle through explicit context handles (`VMContext`):

```c
#include "bpp_api.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
    // 1. Create an isolated VM context
    VMContext *vm = bpp_create();
    if (!vm) {
        fprintf(stderr, "Failed to allocate BASIC++ VM context\n");
        return 1;
    }

    // 2. Initialize runtime structures and memory pools
    bpp_init(vm, argc, argv);

    // 3. Execute an inline BASIC statement
    bpp_exec(vm, "PRINT \"Hello from Embedded BASIC++!\"");

    // 4. Load and run a script file
    bpp_load(vm, "scripts/application.bas");
    bpp_run(vm);

    // 5. Cleanly tear down and reclaim memory
    bpp_destroy(vm);
    return 0;
}
```

## 3. BIDIRECTIONAL DATA EXCHANGE

Host programs can read and set BASIC++ numeric and string variables directly through the API:

```c
// Assign values from host C to BASIC++ variables
bpp_set_num(vm, "THRESHOLD", 85.5);
bpp_set_str(vm, "SERVER_NAME$", "gateway.local");

// Execute BASIC code that computes results
bpp_exec(vm, "TOTAL = THRESHOLD * 1.15");

// Read computed values back into host C variables
double total = bpp_get_num(vm, "TOTAL");
const char *server = bpp_get_str(vm, "SERVER_NAME$");

printf("Calculated Total: %f\n", total);
```

Variable accesses respect BASIC++ naming rules: type suffixes (`%`, `&`, `!`, `#`, `$`) determine data representation in the symbol table.

## 4. REGISTERING NATIVE C CALLBACKS

Host applications can expose native C functions to BASIC scripts as callable subroutines or functions:

```c
static BValue host_log_callback(VMContext *vm, LexerContext *lex, BppError *err) {
    BValue msg = eval_expression(vm, lex, err);
    if (err->code == 0 && msg.type == VAL_STRING) {
        printf("[HOST LOG] %s\n", msg.as.string->chars);
    }
    return bvalue_make_integer(0);
}

// Inside host initialization:
bpp_register_function(vm, "HOSTLOG", host_log_callback);
```

In BASIC code, the callback is invoked as an intrinsic function:
```basic
10 RESULT% = HOSTLOG("Transaction initialized successfully")
```

## 5. ERROR HANDLING & DIAGNOSTIC INSPECTION

When a statement execution fails, `bpp_exec()` or `bpp_run()` returns a non-zero error code. Host applications retrieve diagnostic details using `bpp_get_error()`:

```c
if (bpp_exec(vm, "PRINT 10 / 0") != 0) {
    BppError err = bpp_get_error(vm);
    printf("Error %d on line %d: %s\n", err.code, (int)err.line, err.message);
}
```

## 6. MULTI-INSTANCE ISOLATION

Because all runtime state is encapsulated within `VMContext`, host applications may instantiate multiple concurrent VM instances within the same process. Provided that host callbacks do not access unprotected global variables, each VM executes independently with its own program memory, variable table, and virtual device channels.
