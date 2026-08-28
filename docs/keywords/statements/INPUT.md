# `INPUT` Interactive Console & File Input Statement

## 1. BASIC Usage and Keyword Definition

Reads interactive user input from the console, or comma-delimited data fields from an open file.

### Syntax Signatures:
```basic
INPUT [;] ["prompt" {; | ,}] variable [, variable...]
INPUT #filenum%, variable [, variable...]
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: File channel is not open for input.

### Operational Notes:
- Supports trailing semicolons to suppress carriage returns.

---

## 2. Code Examples

```basic
10 INPUT "Enter your name: ", Name$
20 INPUT "Enter age: "; Age%
30 PRINT "Hello, "; Name$; " Age: "; Age%
```
