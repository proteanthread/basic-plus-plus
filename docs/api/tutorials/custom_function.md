# Tutorial: Creating Native C Functions in BASIC++

## 1. Overview

This tutorial demonstrates how to extend the BASIC++ language by implementing and registering a custom native C function using the Function Registry API (`runtime/funcreg.h`).

By the end of this tutorial, you will have implemented a native mathematical function (`HYPOT(x, y)`) and registered it so that it is callable from interactive REPL mode and BASIC source scripts.

---

## 2. Function Signature & Handler Callback

All native functions conform to the `FuncHandler` function pointer typedef:
```c
typedef BValue (*FuncHandler)(BValue *args, int argc, void *rt);
```

### Implementing `HYPOT(x, y)`
Create `my_functions.c`:

```c
#include "runtime/funcreg.h"
#include "types/types.h"
#include <math.h>

/**
 * @brief Computes hypotenuse: sqrt(x^2 + y^2)
 */
static BValue func_hypot(BValue *args, int argc, void *rt) {
    (void)rt;
    
    /* Argument validation */
    if (argc != 2) {
        /* Evaluator automatically validates min/max args, but explicit guards are good practice */
        BValue err;
        err.type = VAL_ERROR;
        err.as.error = 5; /* Illegal function call */
        return err;
    }

    if (args[0].type != VAL_NUMBER || args[1].type != VAL_NUMBER) {
        BValue err;
        err.type = VAL_ERROR;
        err.as.error = 13; /* Type mismatch */
        return err;
    }

    double x = args[0].as.number;
    double y = args[1].as.number;
    double result = sqrt(x * x + y * y);

    return bval_float(result);
}
```

---

## 3. Registering the Function Entry

Define the `FunctionEntry` descriptor and register it via `funcreg_register()`:

```c
void register_my_math_functions(void) {
    FunctionEntry entry = {
        .name         = "HYPOT",
        .keyword      = KW_NONE,
        .category     = FCAT_MATH,
        .ret_type     = FRET_FLOAT,
        .min_args     = 2,
        .max_args     = 2,
        .safety       = FSAFE_PURE,
        .overridable  = 1,
        .handler      = func_hypot,
        .help_text    = "Calculates the Euclidean distance sqrt(x^2 + y^2).",
        .module_name  = "math_ext"
    };

    funcreg_register(&entry);
}
```

---

## 4. Returning Strings from Functions

When returning string data, allocate the string using `StringContext` from the runtime pointer:

```c
static BValue func_reverse(BValue *args, int argc, void *rt) {
    (void)argc;
    VMContext *vm = (VMContext *)rt;
    StringContext *str_ctx = vm_get_str(vm);

    if (args[0].type != VAL_STRING) {
        BValue err;
        err.type = VAL_ERROR;
        err.as.error = 13;
        return err;
    }

    const char *src = str_data(args[0].as.string);
    size_t len = str_len(args[0].as.string);

    /* Allocate reverse buffer */
    char *buf = malloc(len + 1);
    for (size_t i = 0; i < len; i++) {
        buf[i] = src[len - 1 - i];
    }
    buf[len] = '\0';

    BppStringRef result_str = str_create(str_ctx, buf, len);
    free(buf);

    BValue res;
    res.type = VAL_STRING;
    res.as.string = result_str;
    return res;
}
```

---

## 5. Testing from BASIC++

Once registered, invoke the new functions directly:

```basic
10 LET A = 3.0
20 LET B = 4.0
30 LET C = HYPOT(A, B)
40 PRINT "Hypotenuse of 3 and 4 is: "; C
50 PRINT "Reversed string: "; REVERSE$("BASIC++")
```

Output:
```text
Hypotenuse of 3 and 4 is:  5
Reversed string: ++CISAB
```
