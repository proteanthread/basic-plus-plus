# `arrays` Multi-Dimensional Array Subsystem (`libengine`)

## 1. Architectural Purpose & Overview

The `arrays` subsystem (`engine/src/runtime/arrays.c`) implements dynamic multi-dimensional array allocation, indexing, redimensioning, and deallocation in BASIC++.

### Key Architectural Invariants:
- **`OPTION BASE` Adherence**: Loops and index offset calculations MUST query `arr_get_option_base()` rather than hardcoding `0` or `1`.
- **String Array Dereferencing**: Deallocating a string array releases all contained reference-counted strings via `str_release()`.
- **Bounds Checking**: Subscripts outside active dimensions trigger Error 9 (`ERR_SUBSCRIPT_OUT_OF_RANGE`).

---

## 2. Technical API Signatures (C17)

```c
ArrayContext *arr_create(void);
void arr_destroy(ArrayContext *arr_ctx);
BppArray *arr_declare(ArrayContext *arr_ctx, const char *name, int dims, int *bounds, BValType elem_type);
BValue *arr_get_element(ArrayContext *arr_ctx, const char *name, int dims, int *indices);
void arr_erase(ArrayContext *arr_ctx, const char *name);
```
