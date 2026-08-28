# `strings` Reference-Counted String Manager (`libengine`)

## 1. Architectural Purpose & Overview

The `strings` subsystem (`engine/src/runtime/strings.c`) implements an immutable, reference-counted string pool with copy-on-write semantics and safe memory reclamation.

### Key Architectural Invariants:
- **Two-Parameter `str_release()`**: Calling `str_release()` requires two arguments: `str_release(StringContext *str_ctx, BppString *str)`.
- **String Mutation Safety**: String operations (`MID$`, `LEFT$`, concatenation) allocate new reference-counted instances rather than modifying in-place buffers.
- **Reference Counting**: Strings are automatically freed when their refcount drops to zero.

---

## 2. Technical API Signatures (C17)

```c
StringContext *str_create_context(void);
void str_destroy_context(StringContext *ctx);
BppString *str_create(StringContext *ctx, const char *data, size_t length);
BppString *str_retain(BppString *str);
void str_release(StringContext *ctx, BppString *str);
const char *str_data(const BppString *str);
size_t str_len(const BppString *str);
```
