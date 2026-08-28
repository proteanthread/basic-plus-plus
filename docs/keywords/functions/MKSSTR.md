# `MKS$` / `MKSSTR` Make Single String Function

## 1. BASIC Usage and Function Definition

The `MKS$` (Make Single String) function converts a single-precision floating-point number into a 4-byte binary string for storage in random access file records or binary communication buffers.

### Syntax Signatures:
```basic
result$ = MKS$(single_expr!)
```

### Operational Rules:
- **Output Length**: Exactly 4 bytes.
- **Precision**: Encodes 32-bit IEEE 754 float.
- **Inverse Operation**: Deserialized using `CVS(str$)`.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Return Length | Format |
|---|---|---|---|
| **GW-BASIC / BASICA** | `MKS$(X!)` | 4 bytes | MBF or IEEE |
| **QuickBASIC / QBASIC** | `MKS$(X!)` | 4 bytes | IEEE 754 Single |
| **BASIC++ (Master)** | `MKS$(X!)` / `MKSSTR(X!)` | 4 bytes | IEEE 754 Single |

---

## 3. Examples

```basic
10 OPEN "R", #1, "temps.dat", 4
20 FIELD #1, 4 AS T_RAW$
30 LSET T_RAW$ = MKS$(98.6)
40 PUT #1, 1
50 CLOSE #1
```
