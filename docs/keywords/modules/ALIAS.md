# `ALIAS` Metaprogramming Identifier & Operator Synonym Statement

## 1. BASIC Usage and Keyword Definition

The `ALIAS` statement creates custom synonyms, alternative language keywords, or custom operator mappings at runtime without requiring recompilation of the BASIC++ engine. It allows developers to customize language syntax, provide localized keywords (e.g. `IMPRIMIR` for `PRINT`), or define domain-specific syntax.

### Syntax Signatures:
```basic
ALIAS new_keyword = existing_keyword
ALIAS OPER new_operator = existing_operator
ALIAS LIST
ALIAS CLEAR
```

### Operational Rules:
- **Keyword Aliasing**: Registers `new_keyword` in the lexical analyzer's dynamic keyword table, mapping all future occurrences to the token ID and statement handler of `existing_keyword`.
- **Protected Keywords**: Keywords vital to VM integrity (`SCOPE`, `ALIAS`, `KEYWORD`, `OVERRIDE`, `END`, `STOP`, `RUN`, `CLEAR`) cannot be overridden or clobbered.
- **Operator Aliasing**: `ALIAS OPER` registers operator aliases (such as `MODULO` for `MOD` or `!=` for `<>`).
- **Scope**: Aliases persist for the active VM session or within the enclosing `SCOPE` block.

---

## 2. Language Dialect & Compatibility

| Dialect | Syntax | Dynamic Aliasing | Operator Aliasing |
|---|---|---|---|
| **GW-BASIC / BASICA** | *None* | Not supported | Not supported |
| **QuickBASIC / QBASIC** | `ALIAS "c_func"` (in DECLARE) | External symbol linking only | Not supported |
| **BASIC++ (Master)** | `ALIAS new = existing` | Full runtime syntax customization | Supported (`ALIAS OPER`) |

---

## 3. Code Examples

```basic
10 ALIAS SAY = PRINT
20 ALIAS DISPLAY = PRINT
30 SAY "Hello from custom keyword ALIAS!"
40 DISPLAY "Multi-language or DSL syntax enabled."
```
