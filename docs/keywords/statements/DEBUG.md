# `DEBUG` Debug Engine Control Statement

## 1. BASIC Usage and Keyword Definition

Controls interactive VM execution debugging, line monitoring, and DAP debug server hooks.

### Syntax Signatures:
```basic
DEBUG ON | OFF | TRACE | BREAK
```

### Operational Notes:
- Integrates with the Debug Adapter Protocol (DAP) server in libstandard.

---

## 2. Code Examples

```basic
10 DEBUG ON
20 LET X = 100
30 DEBUG OFF
```
