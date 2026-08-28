# `IMAGE` Print Using Format Template Definition Statement

## 1. BASIC Usage and Keyword Definition

Defines a format string template referenced by formatted PRINT USING operations.

### Syntax Signatures:
```basic
IMAGE format_string$
```

### Operational Notes:
- ECMA-116 standard dialect feature.

---

## 2. Code Examples

```basic
10 IMAGE "Total: $$###.##"
20 PRINT USING 10; 123.45
```
