# `GUID$` Globally Unique Identifier String Generator Function

## 1. BASIC Usage and Function Definition

The `GUID$` function generates a random version-4 UUID / GUID formatted as a standard 36-character hyphenated hexadecimal string.

### Syntax Signatures:
```basic
uuid_str$ = GUID$
```

### Operational Rules:
- Returns a string in standard 8-4-4-4-12 UUID format (e.g. `123e4567-e89b-12d3-a456-426614174000`).
- Returns a reference-counted string (`VAL_STRING`).

---

## 2. Code Examples

```basic
10 PRINT "Generated Session ID: "; GUID$
```
