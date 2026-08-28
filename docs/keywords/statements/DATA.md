# `DATA` Embedded Constant Data Declaration Statement

## 1. BASIC Usage and Keyword Definition

Embeds static numeric and string constants within program source code for retrieval by READ statements.

### Syntax Signatures:
```basic
DATA constant [, constant...]
```

### Error Handling & Boundary Conditions:
- **Error 4 (ERR_OUT_OF_DATA)**: Read attempted past last DATA item.

### Operational Notes:
- Data elements are preserved in program memory and navigated via RESTORE.

---

## 2. Code Examples

```basic
10 READ A, B, C$
20 PRINT A; B; C$
30 DATA 10, 20, "Hello World"
```
