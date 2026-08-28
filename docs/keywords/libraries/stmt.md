# `stmt` Statement Handler Architecture & Dispatch Registry

## 1. Architectural Purpose & Overview

The statement subsystem (`engine/src/statements/`) dispatches and executes all BASIC++ statements via modular micro-libraries.

### Key Architectural Invariants:
- **Strict Keyword-to-Filename Mapping**: Every statement maps to a dedicated `.c` file and micro-library target (e.g. `stmt_mux.c` for `MUX`/`DEMUX`).
- **Token Advancement Guard**: Statement handlers MUST never return `err.code = 0` without consuming at least one token via `lex_next(lex)`.
- **Parameter Bounds & Error 5**: Bounded arguments must be explicitly validated. Out-of-bounds parameters return Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Technical API Signatures (C17)

```c
typedef BppError (*StatementHandler)(VMContext *vm, LexerContext *lex);
void stmt_register(BppKeywordId kw, StatementHandler handler);
BppError stmt_dispatch(VMContext *vm, LexerContext *lex, BppKeywordId kw);
```
