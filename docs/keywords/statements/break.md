# `break` Execution Breakpoint Statement

## 1. BASIC Usage and Keyword Definition

Configures or triggers an immediate breakpoint in the program execution engine.

### Syntax Signatures:
```basic
BREAK [ON | OFF]
```

### Operational Notes:
- Integrates with interactive debugger and DAP server.

---

## 2. Code Examples

```basic
10 BREAK ON
20 FOR I = 1 TO 10
30   IF I = 5 THEN BREAK
40 NEXT I
```
