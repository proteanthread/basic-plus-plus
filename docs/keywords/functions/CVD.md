# `CVD` Convert String to Double-Precision Number Function

## 1. BASIC Usage and Function Definition

The `CVD` (Convert to Double) function unpacks an 8-byte binary string (typically read from a random access file buffer or network stream) and translates it into an IEEE 754 64-bit double-precision floating-point number.

### Syntax Signatures:
```basic
result# = CVD(string_8_bytes)
```

### Operational Rules:
- **String Length Requirement**: Exactly 8 bytes in length. Passing a string shorter or longer than 8 bytes triggers Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).
- **Binary Format**: Interprets bytes in native host byte order (little-endian on x86/ARM).
- **Inverse Operation**: Inverse of `MKD$(number#)` (`MKDSTR`).
- **Non-String Input**: Passing a non-string argument triggers Error 13 (`ERR_TYPE_MISMATCH`).

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Precision | Binary Format |
|---|---|---|---|
| **GW-BASIC / BASICA** | `CVD(A$)` | Microsoft Binary Format (MBF 64-bit) or IEEE | 8 bytes |
| **QuickBASIC / QBASIC** | `CVD(A$)` | IEEE 754 64-bit Double | 8 bytes |
| **BASIC++ (Master)** | `CVD(A$)` | IEEE 754 64-bit Double (`double`) | 8 bytes |

---

## 3. Examples

### Reading Double-Precision Values from Random Access Files
```basic
10 OPEN "R", #1, "physics.dat", 8
20 FIELD #1, 8 AS ENERGY$
30 GET #1, 1
40 E# = CVD(ENERGY$)
50 PRINT "Recovered double value: "; E#
60 CLOSE #1
```
