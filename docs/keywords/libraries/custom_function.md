# `custom_function` Custom Function Extension Template

## 1. Architectural Purpose & Overview

The `custom_function` guide and code template demonstrates how to implement and register a custom built-in function in BASIC++ using C17.

### Implementation Blueprint:
```c
#include "eval/eval.h"
#include "runtime/funcreg.h"
#include "runtime/micro_lib_metadata.h"

BValue func_my_custom_eval(VMContext *vm, const char *uname, int argc, BValue *args, BppError *err) {
    (void)uname;
    BValue res = { .type = VAL_NUMBER, .as.number = 0.0 };
    if (argc != 1 || args[0].type != VAL_NUMBER) {
        err->code = 13; err->message = "Expected 1 numeric argument";
        return res;
    }
    res.as.number = args[0].as.number * 2.0;
    return res;
}

void func_my_custom_register(void) {
    FunctionEntry entry = {
        .name = "MYDOUBLE",
        .category = FCAT_MATH,
        .ret_type = FRET_NUM,
        .min_args = 1, .max_args = 1,
        .handler = func_my_custom_eval,
        .help_text = "Doubles the input number"
    };
    funcreg_register(&entry);
}
```
