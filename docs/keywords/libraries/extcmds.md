# `extcmds` Extended Commands & Custom Statement Subsystem (`libext`)

## 1. Architectural Purpose & Overview

The `extcmds` subsystem (`engine/src/runtime/extcmds.c`) allows third-party libraries and users to register custom statement handlers and extension commands dynamically.

### Key Architectural Invariants:
- **Auditable Registration**: Commands register with capability checks and parameter signature metadata.
- **Micro-Library Isolation**: Extends the runtime without modifying core interpreter loops.

---

## 2. Technical API Signatures (C17)

```c
typedef BppError (*ExtCmdHandler)(VMContext *vm, LexerContext *lex);
void extcmds_register(const char *name, ExtCmdHandler handler, SecurityCapability cap);
```
