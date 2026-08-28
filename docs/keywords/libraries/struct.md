# `struct` User-Defined Structures & Types (`libengine`)

## 1. Architectural Purpose & Overview

The `struct` subsystem manages user-defined data records (`TYPE ... END TYPE`) and structured object memory layouts in BASIC++.

### Key Architectural Invariants:
- **Field Alignment**: Struct fields adhere to C-compatible structure padding and alignment constraints.
- **Nested Field Addressing**: Supports dot-notation member dereferencing (`record.field.subfield`).
- **Binary Compatibility**: Structures can be read and written directly to random-access file buffers (`GET` / `PUT`).

---

## 2. Technical API Signatures (C17)

```c
typedef struct StructField {
    char name[64];
    BValType type;
    size_t offset;
    size_t size;
} StructField;
```
