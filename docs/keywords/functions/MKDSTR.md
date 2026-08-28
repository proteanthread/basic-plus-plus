# `MKD$` / `MKDSTR` Make Double String Function

## 1. BASIC Usage and Function Definition

The `MKD$` (Make Double String) function serializes a 64-bit double-precision floating-point number into an 8-byte binary string representation suitable for storing in fixed-length random access file records (`FIELD`, `PUT`) or transmitting across binary network sockets (`N:`).

### Syntax Signatures:
```basic
result$ = MKD$(double_expr#)
```

### Operational Rules:
- **Output Length**: Always returns a new reference-counted string of length exactly 8 bytes.
- **Precision**: Encodes full 64-bit IEEE 754 precision (`double` in C17).
- **String Ownership**: Returns a new `VAL_STRING` managed by `StringContext`.
- **Inverse Operation**: Deserialized using `CVD(str$)`.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Return Length | Format |
|---|---|---|---|
| **GW-BASIC / BASICA** | `MKD$(X#)` | 8 bytes | MBF or IEEE |
| **QuickBASIC / QBASIC** | `MKD$(X#)` | 8 bytes | IEEE 754 Double |
| **BASIC++ (Master)** | `MKD$(X#)` / `MKDSTR(X#)` | 8 bytes | IEEE 754 Double |

---

## 3. Examples

```basic
10 OPEN "R", #1, "records.dat", 8
20 FIELD #1, 8 AS RAW_DBL$
30 PI_VAL# = 3.141592653589793
40 LSET RAW_DBL$ = MKD$(PI_VAL#)
50 PUT #1, 1
60 CLOSE #1
```
