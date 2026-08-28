# `CVI` Convert String to Integer Function

## 1. BASIC Usage and Function Definition

The `CVI` (Convert to Integer) function unpacks a 2-byte (or 4-byte depending on integer word size) binary string and converts it into a signed integer number.

### Syntax Signatures:
```basic
result% = CVI(string_2_bytes)
```

### Operational Rules:
- **String Length Requirement**: Exactly 2 bytes (or 4 bytes in 32-bit integer mode). If the string length is invalid, Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`) is returned.
- **Signed Representation**: Interprets the binary bytes as a 16-bit signed two's-complement integer ($-32768 \dots 32767$).
- **Inverse Operation**: Inverse of `MKI$(number%)` (`MKISTR`).

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Bit Width | Range |
|---|---|---|---|
| **GW-BASIC / BASICA** | `CVI(A$)` | 16-bit signed | -32,768 to 32,767 |
| **QuickBASIC / QBASIC** | `CVI(A$)` | 16-bit signed | -32,768 to 32,767 |
| **BASIC++ (Master)** | `CVI(A$)` | 16-bit signed (or 32-bit via `CVL`) | -32,768 to 32,767 |

---

## 3. Examples

### Unpacking Integer from Field Buffer
```basic
10 OPEN "R", #1, "scores.dat", 2
20 FIELD #1, 2 AS SCORE_RAW$
30 GET #1, 1
40 SCORE% = CVI(SCORE_RAW$)
50 PRINT "Score: "; SCORE%
60 CLOSE #1
```
