# `types` Unified Value & Type Representation (`libkernel`)

## 1. Architectural Purpose & Overview

The `types` header definitions (`engine/include/types/`) define the fundamental data types, values (`BValue`), token unions (`BppToken`), and error structures (`BppError`) in BASIC++.

### Value Model (`BValue`):
- `VAL_NONE`: Uninitialized or empty value.
- `VAL_NUMBER`: 64-bit IEEE 754 floating-point number (`double`).
- `VAL_STRING`: Reference-counted string pointer (`BppString*`).
- `VAL_ERROR`: Runtime error integer code.
- `VAL_OBJECT`: User-defined type or module handle.

---

## 2. Technical Definitions (C17)

```c
typedef enum BValType {
    VAL_NONE,
    VAL_NUMBER,
    VAL_STRING,
    VAL_ERROR,
    VAL_OBJECT
} BValType;

typedef struct BValue {
    BValType type;
    union {
        double number;
        struct BppString *string;
        int error_code;
        void *object;
    } as;
} BValue;
```
