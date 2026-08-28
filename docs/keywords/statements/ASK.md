# `ASK` Device and Screen Attribute Query Statement

## 1. BASIC Usage and Keyword Definition

Queries system, screen, or device configuration properties in ECMA-116 compliant mode.

### Syntax Signatures:
```basic
ASK item% [, result_var...]
```

### Error Handling & Boundary Conditions:
- **Error 5 (ERR_ILLEGAL_FUNCTION_CALL)**: Invalid property queried.

### Operational Notes:
- Conforms to ECMA-116 standard dialect extensions.

---

## 2. Code Examples

```basic
10 ASK SCREEN WIDTH W%
20 PRINT "Current screen width: "; W%
```
