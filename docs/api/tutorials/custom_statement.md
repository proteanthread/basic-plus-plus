# Tutorial: How to Add a Statement

> **Purpose**: Guide to writing new syntax statement handlers and registering them into the compiler dispatcher parser.

---

## Step 1: Implement Statement Handler
Define a function matching the parser signature:
```c
#include "stmt.h"
#include "lexer.h"
#include "eval.h"
#include <stdio.h>
#include <string.h>

BppError stmt_my_keyword_handler(VMContext *vm, LexerContext *lex) {
    (void)lex;
    BppError err = {0, ERR_CAT_NONE, NULL, 0.0, 0, NULL};
    BppError eval_err;
    memset(&eval_err, 0, sizeof(eval_err));
    
    LexerContext *temp_lex = lex_init(NULL, "10");
    BValue val = eval_expression(vm, temp_lex, &eval_err);
    printf("My keyword got value: %f\n", val.as.number);
    lex_shutdown(temp_lex);
    return err;
}
```
