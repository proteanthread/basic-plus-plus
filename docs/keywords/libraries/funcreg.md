# `funcreg` Built-in Function Registry (`libkernel`)

## 1. Architectural Purpose & Overview

The `funcreg` subsystem (`engine/src/runtime/funcreg.c`) manages the central dispatch registry for all mathematical, string, trigonometric, system, and virtual device functions.

### Key Architectural Invariants:
- **Domain Dispatch**: Standard functions (`DEF FN`, `DEF STR`, `DEF GFX`) are routed through the Domain Dispatch Registry.
- **Safety Categories**: Functions declare safety levels (`FSAFE_NONE`, `FSAFE_FS`, `FSAFE_SYSTEM`) checked prior to execution against the active `SecurityContext`.

---

## 2. Technical API Signatures (C17)

```c
typedef struct FunctionEntry {
    const char *name;
    BppKeywordId keyword;
    FuncCategory category;
    FuncRetType ret_type;
    int min_args;
    int max_args;
    FuncSafety safety;
    int overridable;
    BuiltinFuncHandler handler;
    const char *help_text;
    const char *module_name;
} FunctionEntry;

void funcreg_register(const FunctionEntry *entry);
const FunctionEntry *funcreg_lookup(const char *name);
```
