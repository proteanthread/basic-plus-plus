# `GUID` GUID Validation & Binary Conversion Function

## 1. BASIC Usage and Function Definition

The `GUID` function parses and validates a UUID/GUID string representation, returning a status code or raw 16-byte buffer representation.

### Syntax Signatures:
```basic
is_valid% = GUID(uuid_str$)
```

### Operational Rules:
- Returns non-zero (true) if `uuid_str$` conforms to RFC 4122 standard UUID format; 0 (false) otherwise.

---

## 2. Code Examples

```basic
10 S$ = GUID$
20 IF GUID(S$) THEN PRINT "Valid UUID generated: "; S$
```
