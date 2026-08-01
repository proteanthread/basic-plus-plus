# Statement Dispatcher API Reference

Header File: [`include/bpp_stmt.h`](file:///c:/Users/rtdos/GitHub/basic-plus-plus/include/bpp_stmt.h)

## Overview
Registers statement handlers and dispatches execution of matched BASIC statement lines.

## Exposed API Entities
### Structs & Types
- `StmtRegistry StmtRegistry`

### Functions
| Function | Return Type | Arguments |
|----------|-------------|-----------|
| `stmt_defseg_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_registry_shutdown` | `void` | `StmtRegistry *reg` |
| `stmt_register` | `void` | `StmtRegistry *reg, BppKeywordId kw, BppStmtHandler handler, const char *name, uint32_t flags` |
| `stmt_lookup` | `BppStmtHandler` | `StmtRegistry *reg, BppKeywordId kw` |
| `vm_call_sub_procedure` | `BppError` | `VMContext *vm, const char *sub_name, BValue *args, int arg_count, const char *ret_pos` |
| `find_procedure` | `bool` | `VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text` |
| `stmt_assert_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_tron_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_troff_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_break_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |
| `stmt_vars_handler` | `BppError` | `VMContext *vm, LexerContext *lex` |

## C Integration Example
The following C example demonstrates how to integrate this subsystem:
```c
#include "bpp_stmt.h"

BppError my_stmt_handler(VMContext *vm, LexerContext *lex) {
    (void)vm; (void)lex;
    BppError err = {0, ERR_CAT_NONE, NULL, 0.0, 0, NULL};
    return err;
}
```

## Guidelines & Architecture Constraints
- **C17 Portability**: Compile under strict C17 standards.
- **Memory Integrity**: All contexts and pointers passed must be zero-initialized.
