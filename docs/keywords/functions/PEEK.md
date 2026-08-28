# `PEEK` Read Memory Byte Function

## 1. BASIC Usage and Function Definition

The `PEEK` function reads and returns an 8-bit unsigned byte value ($0$ to $255$) from the specified virtual memory address in the current segment/bank.

### Syntax Signatures:
```basic
byte_val% = PEEK(address_expression)
```

### Operational Rules:
- **Address Calculation**: Address is calculated relative to the active segment defined by `DEF SEG` or `BANK`.
- **Return Range**: Returns an integer in range $0 \le \text{byte\_val} \le 255$.
- **Security Check**: In sandboxed environments, memory reads outside allowed regions raise Error 70 (`ERR_PERMISSION_DENIED`).
- **Bounds Check**: Addresses beyond the 64KB segment limit trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).

---

## 2. Code Examples

```basic
10 DEF SEG = &H0040 : REM BIOS Data Area
20 EQUIP_FLAG = PEEK(&H0010)
30 PRINT "BIOS Equipment Word Low Byte: &H"; HEX$(EQUIP_FLAG)
40 DEF SEG : REM Reset to default segment
```
