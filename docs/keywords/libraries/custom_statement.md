# `custom_statement` Custom Statement Extension Template

## 1. Architectural Purpose & Overview

The `custom_statement` template demonstrates how to implement a dedicated C statement handler, parse tokens, evaluate expressions, and register the statement with the runtime dispatcher.

### Implementation Blueprint (C17):
```c
#include "statements/variables/def.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include "eval/eval.h"

BppError stmt_my_custom_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) return err;
    if (val.type == VAL_STRING) {
        // Use string
        str_release(vm_get_str(vm), val.as.string); // Always release!
    }
    return err;
}
```
