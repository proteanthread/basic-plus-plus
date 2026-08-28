# `VARPTR` Variable Pointer Function

## 1. BASIC Usage and Function Definition

The `VARPTR` function returns the virtual memory address offset (pointer) where a variable, array element, or file control block is stored in memory.

### Syntax Signatures:
```basic
addr% = VARPTR(variable_name)
addr% = VARPTR(array_element)
addr% = VARPTR(#file_number)
```

### Operational Rules:
- Returns an address/offset (`uintptr_t` formatted as integer) relative to the default BASIC data segment.
- If passed `#file_number`, returns the pointer to the internal file handle control structure.
- Often paired with `PEEK`, `POKE`, or assembly subroutines (`CALL ABSOLUTE`).

---

## 2. Code Examples

```basic
10 N% = 1234
20 ADDR = VARPTR(N%)
30 PRINT "Variable N% is located at offset: &H"; HEX$(ADDR)
40 PRINT "Low byte read via PEEK: "; PEEK(ADDR)
```
