# Tutorial: Embedding BASIC++ in Host C17 Applications

## 1. Overview

This tutorial demonstrates how to embed the BASIC++ v6.5.2 virtual machine directly into a C17 host application (such as a game engine, CAD tool, automated build pipeline, or embedded system).

You will learn how to:
1. Initialize the VM with custom memory pools.
2. Inject host C variables into the BASIC execution environment.
3. Expose host C functions as built-in BASIC functions.
4. Execute BASIC script code and handle runtime errors.
5. Extract computed results from BASIC variables back into host C.
6. Gracefully shut down and reclaim memory resources.

---

## 2. Minimal Embedding Program

Create `embed_demo.c`:

```c
#include "core/boot.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "runtime/variables.h"
#include "runtime/funcreg.h"
#include <stdio.h>
#include <math.h>

/* Host C Function Exposed to BASIC */
static BValue host_log_message(BValue *args, int argc, void *rt) {
    (void)argc; (void)rt;
    if (args[0].type == VAL_STRING) {
        printf("[HOST LOG] %s\n", str_data(args[0].as.string));
    }
    return bval_number(1.0);
}

int main(void) {
    printf("Initializing embedded BASIC++ Virtual Machine...\n");

    /* 1. Boot the VM with a 64 MB memory pool */
    VMContext *vm = boot_system(67108864L);
    if (!vm) {
        fprintf(stderr, "Failed to initialize BASIC++ VM\n");
        return 1;
    }

    /* 2. Register host C functions */
    FunctionEntry host_fn = {
        .name        = "HOSTLOG",
        .category    = FCAT_UTIL,
        .ret_type    = FRET_INT,
        .min_args    = 1,
        .max_args    = 1,
        .safety      = FSAFE_IO,
        .handler     = host_log_message,
        .help_text   = "Logs a message to host stdout."
    };
    funcreg_register(&host_fn);

    /* 3. Inject initial variables into BASIC environment */
    VariableContext *vc = vm_get_var(vm);
    BValue *base_price = var_lookup(vc, "BASE_PRICE#", true);
    if (base_price) {
        base_price->type = VAL_NUMBER;
        base_price->as.number = 1500.00;
    }

    /* 4. Execute BASIC script snippet */
    const char *script = 
        "TAX_RATE = 0.08\n"
        "FINAL_TOTAL# = BASE_PRICE# * (1.0 + TAX_RATE)\n"
        "HOSTLOG(\"Computed Total: $\" + STR$(FINAL_TOTAL#))\n";

    printf("Executing script...\n");
    BppError err = vm_execute_line(vm, script);
    if (err.code != 0) {
        fprintf(stderr, "BASIC Execution Error %d: %s\n", err.code, err.message);
    }

    /* 5. Extract calculated result back into C */
    BValue *final_total = var_lookup(vc, "FINAL_TOTAL#", false);
    if (final_total && final_total->type == VAL_NUMBER) {
        printf("Extracted Result in C: Final Price = $%.2f\n", final_total->as.number);
    }

    /* 6. Clean shutdown */
    printf("Shutting down VM...\n");
    boot_shutdown_vm(vm);
    printf("Embedded execution completed successfully.\n");
    return 0;
}
```

---

## 3. Compiling and Linking

### Linux / GCC:
```bash
gcc -std=c17 -O2 -I../../engine/include embed_demo.c -L../../build/lib -lbaspp_engine -lm -o embed_demo
```

### Windows / MSVC:
```cmd
cl /std:c17 /O2 /I..\..\engine\include embed_demo.c /link /LIBPATH:..\..\build_win\lib baspp_engine.lib
```

---

## 4. Expected Output

```text
Initializing embedded BASIC++ Virtual Machine...
Executing script...
[HOST LOG] Computed Total: $ 1620
Extracted Result in C: Final Price = $1620.00
Shutting down VM...
Embedded execution completed successfully.
```
