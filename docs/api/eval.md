# Expression Evaluator API Reference

Header File: [`include/eval.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/eval.h)

## Overview
Calculates math, logic, and string expressions using the VM operator stacks.

## Exposed API Entities
### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `eval_expression` | `BValue` | `VMContext *vm, LexerContext *lex, BppError *err` |
| `invoke_user_function` | `BValue` | `VMContext *vm, const char *name, BValue *args, int argc, BppError *err` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "eval.h"

void evaluate(VMContext *vm, const char *expr) {
    BppError err;
    memset(&err, 0, sizeof(err));
    LexerContext *lex = lex_init(NULL, expr);
    BValue res = eval_expression(vm, lex, &err);
    (void)res;
    lex_shutdown(lex);
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
