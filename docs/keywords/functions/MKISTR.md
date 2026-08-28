# `MKI$` / `MKISTR` Make Integer String Function

## 1. BASIC Usage and Function Definition

The `MKI$` (Make Integer String) function converts a signed integer value into a 2-byte binary string representation for storage in random access file records or binary communication buffers.

### Syntax Signatures:
```basic
result$ = MKI$(integer_expr%)
```

### Operational Rules:
- **Output Length**: Exactly 2 bytes.
- **Range Check**: Clamped or wrapped within signed 16-bit bounds ($-32768 \dots 32767$).
- **Inverse Operation**: Inverse of `CVI(str$)`.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Return Length | Format |
|---|---|---|---|
| **GW-BASIC / BASICA** | `MKI$(X%)` | 2 bytes | 16-bit signed |
| **QuickBASIC / QBASIC** | `MKI$(X%)` | 2 bytes | 16-bit signed |
| **BASIC++ (Master)** | `MKI$(X%)` / `MKISTR(X%)` | 2 bytes | 16-bit signed |

---

## 3. Examples

```basic
10 OPEN "R", #1, "indices.dat", 2
20 FIELD #1, 2 AS IDX_RAW$
30 LSET IDX_RAW$ = MKI$(1024)
40 PUT #1, 1
50 CLOSE #1
```
