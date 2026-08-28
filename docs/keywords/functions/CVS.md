# `CVS` Convert String to Single-Precision Number Function

## 1. BASIC Usage and Function Definition

The `CVS` (Convert to Single) function unpacks a 4-byte binary string and converts it into an IEEE 754 32-bit single-precision floating-point number.

### Syntax Signatures:
```basic
result! = CVS(string_4_bytes)
```

### Operational Rules:
- **String Length Requirement**: Exactly 4 bytes in length. If length is not 4, Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`) is triggered.
- **Binary Format**: Unpacks 4 bytes as single-precision float (`float` in C17).
- **Inverse Operation**: Inverse of `MKS$(number!)` (`MKSSTR`).

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Bit Width | Format |
|---|---|---|---|
| **GW-BASIC / BASICA** | `CVS(A$)` | 32-bit | MBF or IEEE |
| **QuickBASIC / QBASIC** | `CVS(A$)` | 32-bit | IEEE 754 Single |
| **BASIC++ (Master)** | `CVS(A$)` | 32-bit | IEEE 754 Single (`float`) |

---

## 3. Examples

```basic
10 OPEN "R", #1, "floats.dat", 4
20 FIELD #1, 4 AS VAL_RAW$
30 GET #1, 1
40 V! = CVS(VAL_RAW$)
50 PRINT "Single float value: "; V!
60 CLOSE #1
```
