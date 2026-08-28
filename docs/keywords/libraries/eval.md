# `eval` Unified Expression Evaluator (`libengine`)

## 1. Architectural Purpose & Overview

The `eval` subsystem (`engine/src/eval/eval.c`) implements the unified AST expression evaluation engine across all BASIC dialects in BASIC++ v6.5.2.

### Key Architectural Invariants:
- **Unified AST Evaluation**: All expressions use identical operator precedence and type conversion rules regardless of dialect.
- **String Ownership Protocol**: `eval_expression()` returns a `BValue` with an incremented refcount on `VAL_STRING`. Calling statement handlers own the string and MUST call `str_release(vm_get_str(vm), val.as.string)` on both success and error paths.
- **Type Guard Pattern**: Handlers must validate `val.type` before accessing tagged union members.

---

## 2. Technical API Signatures (C17)

```c
BValue eval_expression(VMContext *vm, LexerContext *lex, BppError *err);
BValue eval_primary(VMContext *vm, LexerContext *lex, BppError *err);
```
