# `tinydb` Embedded Key-Value Database Subsystem (`libserver`)

## 1. Architectural Purpose & Overview

The `tinydb` subsystem implements an embedded, in-memory and file-backed key-value database engine for high-speed record persistence in BASIC++.

### Key Architectural Invariants:
- **B-Tree / Hash Backing**: O(1) key lookups with transactional flush to disk.
- **Statement Integration**: Backs `DB.OPEN`, `DB.GET$`, `DB.PUT`, `DB.DELETE`, `DB.CLOSE`.

---

## 2. Technical API Signatures (C17)

```c
TinyDB *tinydb_open(const char *db_path);
int tinydb_put(TinyDB *db, const char *key, const void *val, size_t val_len);
int tinydb_get(TinyDB *db, const char *key, void *out_buf, size_t *out_len);
void tinydb_close(TinyDB *db);
```
