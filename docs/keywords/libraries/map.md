# `map` Hash Map & Associative Array Subsystem (`libflex`)

## 1. Architectural Purpose & Overview

The `map` subsystem (`engine/src/runtime/map.c`) implements dynamic string-to-value associative maps and dictionary data structures in BASIC++.

### Key Architectural Invariants:
- **String Keys**: Key lookups use MurmurHash3 / FNV-1a hash functions.
- **Reference Reclamation**: Value removal releases reference-counted strings and structures.

---

## 2. Technical API Signatures (C17)

```c
typedef struct HashMap HashMap;
HashMap *map_create(void);
void map_destroy(HashMap *map);
void map_set(HashMap *map, const char *key, BValue val);
BValue *map_get(HashMap *map, const char *key);
void map_remove(HashMap *map, const char *key);
```
