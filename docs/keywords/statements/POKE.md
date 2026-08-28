# `POKE` Write Memory Byte Statement

## 1. BASIC Usage and Keyword Definition

The `POKE` statement writes an 8-bit byte value ($0$ to $255$) directly into the specified virtual memory address in the active segment/bank.

### Syntax Signatures:
```basic
POKE address_expression, byte_value_expression
```

### Operational Rules:
- **Value Clamping/Validation**: `byte_value` must evaluate to an integer in the range $0 \le \text{byte} \le 255$. Values outside this range trigger Error 5 (`ERR_ILLEGAL_FUNCTION_CALL`).
- **Target Segment**: Operates relative to the segment defined by `DEF SEG` or `BANK`.
- **Security Sandbox**: Attempting unauthorized writes outside sandboxed memory ranges generates Error 70 (`ERR_PERMISSION_DENIED`).

---

## 2. Code Examples

```basic
10 DEF SEG = &HB800 : REM Text video RAM
20 POKE 0, 65       : REM Character 'A'
30 POKE 1, 15       : REM White on black attribute
40 DEF SEG
```
