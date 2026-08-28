# C17 API Reference: Statement Dispatch Subsystem (`stmt/stmt.h`)

## 1. Subsystem Overview & Responsibilities

The Statement Dispatch Subsystem (`stmt/stmt.h`, implemented in `engine/src/stmt/stmt.c`) manages statement handler registration, keyword token mapping, execution behavior flags (`STMT_FLAG_IMMEDIATE`, `STMT_FLAG_PROGRAM`), structured procedure resolution (`SUB`, `FUNCTION`), and statement execution dispatch for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Central Statement Registry (`StmtRegistry`)**: Maps `BppKeywordId` enums to `BppStmtHandler` C callback function pointers.
- **Immediate vs Program Execution Flags**:
  - `STMT_FLAG_IMMEDIATE`: Permitted in direct REPL mode without line numbers (e.g. `PRINT`, `RUN`, `FILES`).
  - `STMT_FLAG_PROGRAM`: Permitted in numbered program lines (e.g. `DATA`, `DEF FN`, `GOTO`).
  - `STMT_FLAG_BOTH`: Available in both contexts.
- **Structured Procedure Dispatch**: Resolves `SUB` and `FUNCTION` entry points across program memory via `find_procedure()` and `vm_call_sub_procedure()`.
- **Lexer Consumption Invariant**: Every statement handler MUST either consume tokens via `lex_next()` or return a non-zero error code (`err.code != 0`), guaranteeing execution loop progress.

## 2. Header Inclusion & Prerequisites

```c
#include "stmt/stmt.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
```

## 3. Data Structures & Types

```c
/* Behavior Configuration Flags */
#define STMT_FLAG_NONE       0
#define STMT_FLAG_IMMEDIATE  (1 << 0)  /* Can be run directly in REPL */
#define STMT_FLAG_PROGRAM    (1 << 1)  /* Can be run inside program lines */
#define STMT_FLAG_BOTH       (STMT_FLAG_IMMEDIATE | STMT_FLAG_PROGRAM)

/* Statement Handler Callback Signature */
typedef BppError (*BppStmtHandler)(VMContext *vm, LexerContext *lex);

/* Opaque Handle to Statement Registry Context */
typedef struct StmtRegistry StmtRegistry;
```

## 4. Function Prototypes & Operational Contracts

```c
/**
 * @brief Initializes the statement registry.
 */
StmtRegistry *stmt_registry_init(MemoryContext *mem);

/**
 * @brief Shuts down the registry and frees dispatch tables.
 */
void stmt_registry_shutdown(StmtRegistry *reg);

/**
 * @brief Registers a statement handler function pointer with the engine.
 * @param reg Statement registry pointer.
 * @param kw Associated BppKeywordId enum.
 * @param handler C statement handler callback.
 * @param name Diagnostic identifier string.
 * @param flags STMT_FLAG_IMMEDIATE, STMT_FLAG_PROGRAM, or STMT_FLAG_BOTH.
 */
void stmt_register(StmtRegistry *reg, BppKeywordId kw, BppStmtHandler handler, const char *name, uint32_t flags);

/**
 * @brief Looks up the statement handler registered for a keyword ID.
 */
BppStmtHandler stmt_lookup(StmtRegistry *reg, BppKeywordId kw);

/**
 * @brief Searches for a SUB or FUNCTION procedure definition in program memory.
 */
bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text);

/**
 * @brief Calls a SUB procedure, setting up local variable scope and call frames.
 */
BppError vm_call_sub_procedure(VMContext *vm, const char *sub_name, BValue *args, int arg_count, const char *ret_pos);
```

## 5. Architectural Invariants

- **No-Op Token Advancement Guard**: A statement handler must NEVER return `err.code = 0` without consuming at least one token from `lex`.
- **String Memory Discipline**: All temporary `BValue` strings evaluated during argument parsing must be freed via `str_release(vm_get_str(vm), val.as.string)`.

## 6. Code Example: Implementing and Registering a Custom Statement

```c
#include "stmt/stmt.h"
#include "eval/eval.h"
#include <stdio.h>

static BppError stmt_hello_handler(VMContext *vm, LexerContext *lex) {
    BppError err = {0};
    /* Consume optional argument */
    BppToken tok = lex_peek(lex);
    if (tok.type == TOK_STRING) {
        lex_next(lex);
        printf("Hello, %s!\n", str_data(tok.as.string));
    } else {
        printf("Hello, World!\n");
    }
    return err;
}

void register_hello_statement(StmtRegistry *reg) {
    stmt_register(reg, KW_HELLO, stmt_hello_handler, "HELLO", STMT_FLAG_BOTH);
}
```
