# `KEYWORD` Runtime Keyword Management Statement

## 1. BASIC Usage and Keyword Definition

The `KEYWORD` statement provides programmatic introspection and control over the lexical analyzer's keyword dispatch table, enabling listing, defining, and disabling language keywords dynamically.

### Syntax Signatures:
```basic
KEYWORD LIST
KEYWORD DEFINE identifier AS existing_keyword
KEYWORD DISABLE identifier
KEYWORD RESTORE identifier
```

### Operational Rules:
- **`KEYWORD LIST`**: Outputs the complete registry of active keywords and token mappings to the virtual console.
- **`KEYWORD DEFINE`**: Registers a new lexical keyword token pointing to a statement or function dispatcher.
- **`KEYWORD DISABLE`**: Disables recognition of a keyword so that the identifier can be used as a regular variable name without syntax collision.

---

## 2. Code Examples

```basic
10 KEYWORD LIST : REM Inspect all active language keywords
20 KEYWORD DEFINE OUTPUT AS PRINT
30 OUTPUT "Keyword registered successfully!"
```
