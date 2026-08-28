# `SYS` Machine Code Execution Statement

## 1. BASIC Usage and Keyword Definition

The `SYS` statement calls a machine-language subroutine or native C extension located at a specified memory address, preserving compatibility with Commodore 64, VIC-20, and Apple II BASIC.

### Syntax Signatures:
```basic
SYS address_expression [, arg1, arg2, ...]
```

### Operational Rules:
- **Sandbox Enforcement**: Calling `SYS` is subject to security capability check `CAP_SYS` / `SECOP_EXEC`. In sandboxed execution environments, unauthorized calls raise Error 70 (`ERR_PERMISSION_DENIED`).
- **Segmented / Virtual Dispatch**: Dispatches to native machine code or host callback table in `VMContext`.

---

## 2. Code Examples

```basic
10 ADDR% = &H1000
20 SYS ADDR%, 42, 100 : REM Execute native routine with parameters
```
