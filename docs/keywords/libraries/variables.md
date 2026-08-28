# `variables` Variable Context & Symbol Table Manager (`libkernel`)

## 1. Architectural Purpose & Overview

The `variables` subsystem (`engine/src/runtime/variables.c`) implements the variable symbol table, scope resolution stack, and lifecycle management in BASIC++.

### Key Architectural Invariants:
- **Scoping Stack**: Resolves global, procedure-local (`SUB`/`FUNCTION`), and `SHARED` variables across nested scopes.
- **Context Boundaries**: Supports `var_clear_all()` (for `RUN`/`NEW`) and `var_clear_for_chain()` (retaining `COMMON` variables for `CHAIN`).
- **Clean Cleanup**: All strings bound to variables are decremented via `str_release()` during variable context teardown.

---

## 2. Technical API Signatures (C17)

```c
VariableContext *var_create(void);
void var_destroy(VariableContext *vc);
BValue *var_lookup(VariableContext *vc, const char *name, bool create_if_missing);
void var_set(VariableContext *vc, const char *name, BValue val);
void var_clear_all(VariableContext *vc);
void var_clear_for_chain(VariableContext *vc);
```
