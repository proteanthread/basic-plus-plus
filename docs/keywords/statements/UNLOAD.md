# `UNLOAD` Dynamic Module Unload Statement

## 1. BASIC Usage and Keyword Definition

Unloads an active dynamic extension library or plugin from memory.

### Syntax Signatures:
```basic
UNLOAD module_name$
```

### Error Handling & Boundary Conditions:
- **Error 68 (ERR_DEVICE_UNAVAILABLE)**: Module not loaded.

### Operational Notes:
- Releases all allocated memory and function registrations.

---

## 2. Code Examples

```basic
10 UNLOAD "sqlite3_ext"
```
