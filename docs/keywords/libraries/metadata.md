# `metadata` Micro-Library & Keyword Metadata Registry (`libcore`)

## 1. Architectural Purpose & Overview

The `metadata` subsystem (`engine/src/runtime/micro_lib_metadata.c`) provides interactive discovery and introspection for every keyword, statement, function, and micro-library in BASIC++ v6.5.2.

### Key Architectural Invariants:
- **Zero Stub Invariant**: All registered keywords must map to operational implementations.
- **Introspection Parity**: Every micro-library registers its name, category, syntax, help text, and error codes via `MicroLibMetadata`.
- **Online Parity**: Backs interactive `HELP <keyword>` and `CATALOG` commands.

---

## 2. Technical API Signatures (C17)

```c
typedef struct MicroLibMetadata {
    const char *name;
    const char *category;
    const char *syntax;
    const char *help_text;
    const char *error_codes;
} MicroLibMetadata;

void microlib_register(const MicroLibMetadata *meta);
const MicroLibMetadata *microlib_lookup(const char *name);
```
