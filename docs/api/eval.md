# C17 API Reference: Expression Evaluator (`eval/eval.h`)

## 1. Subsystem Overview & Responsibilities

The Expression Evaluator Subsystem (`eval/eval.h`, implemented in `engine/src/eval/eval.c`) provides non-recursive expression parsing, binary/unary operator precedence resolution, built-in and user-defined function dispatch, and tagged union `BValue` evaluation for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Strict Non-Recursive VM Architecture**: Expression evaluation stacks reside strictly on the interpreter-managed heap, preventing host C stack overflow during deeply nested expressions or recursive function calls.
- **Unified Expression Semantics**: Evaluates expressions identically across all dialect modes (GW-BASIC, QuickBASIC, ECMA-116, Super BASIC).
- **Operator Precedence Dispatch**: Handles unary operators (`+`, `-`, `NOT`), binary arithmetic (`^`, `*`, `/`, `\`, `MOD`, `+`, `-`), relational comparisons (`=`, `<>`, `<`, `<=`, `>`, `>=`), and bitwise/logical operators (`AND`, `OR`, `XOR`, `EQV`, `IMP`, `SHL`, `SHR`).
- **Memory Ownership & String Retention**: Any `BValue` returned by `eval_expression()` with type `VAL_STRING` holds an INCREMENTED refcount; the caller becomes the OWNER and MUST call `str_release(vm_get_str(vm), val.as.string)` on all success and error code paths.

## 2. Header Inclusion & Prerequisites

```c
#include "eval/eval.h"
#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
```

## 3. Data Structures & Types

```c
/* Returned Value Tagged Union */
typedef struct {
    BValueType type;    /* VAL_NULL, VAL_NUMBER, VAL_STRING, VAL_ERROR */
    union {
        double        number;
        BppString    *string;
        BppErrorCode  error;
    } as;
} BValue;
```

## 4. Function Prototypes & Operational Contracts

### Primary Expression Evaluator
```c
/**
 * @brief Parses and evaluates a BASIC expression from the token stream.
 * @param vm Pointer to active VMContext (used for variable and string lookups).
 * @param lex Pointer to active LexerContext to consume tokens from.
 * @param err Pointer to BppError structure populated on syntax or runtime error.
 * @return BValue representing the evaluated result (VAL_NUMBER, VAL_STRING, or VAL_ERROR).
 * @note If return type is VAL_STRING, caller OWNS the string and MUST call str_release().
 */
BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *err);
```

### User Function Invocation
```c
/**
 * @brief Invokes a user-defined FUNCTION or DEF FN by name.
 * @param vm Pointer to active VMContext.
 * @param name Function identifier (e.g. "FNCALC", "CalculateTax#").
 * @param args Array of argument BValue elements.
 * @param argc Number of arguments passed.
 * @param err Pointer to BppError structure populated on error.
 * @return BValue result returned by the function.
 */
BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);
```

## 5. Memory Safety & Statement Handler Pattern

All statement handlers invoking `eval_expression()` MUST adhere to the canonical type guard and string release pattern:

```c
BppError stmt_example_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    BValue val = eval_expression(vm, lex, &err);
    if (err.code != 0) {
        return err;
    }

    if (val.type == VAL_STRING) {
        const char *data = str_data(val.as.string);
        /* Consume data */
        
        /* MANDATORY: Release string ownership on BOTH success and error paths */
        str_release(vm_get_str(vm), val.as.string);
    } else {
        /* Wrong type: set error 13 (Type mismatch) */
        err.code = 13;
        err.message = "Type mismatch: Expected string expression";
    }

    return err;
}
```

## 6. Code Example: Evaluating Math Expressions in C

```c
#include "eval/eval.h"
#include "lexer/lexer.h"
#include <stdio.h>

void evaluate_demo(VMContext *vm, const char *expr_str) {
    BppError err = {0};
    LexerContext *lex = lex_init(vm_get_mem(vm), expr_str);
    
    BValue result = eval_expression(vm, lex, &err);
    if (err.code == 0) {
        if (result.type == VAL_NUMBER) {
            printf("Result: %f\n", result.as.number);
        } else if (result.type == VAL_STRING) {
            printf("Result: \"%s\"\n", str_data(result.as.string));
            str_release(vm_get_str(vm), result.as.string);
        }
    } else {
        printf("Eval Error %d: %s\n", err.code, err.message);
    }

    lex_shutdown(lex);
}
```
