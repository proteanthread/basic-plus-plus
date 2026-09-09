<!--
Title:        COMPLEX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/complex.c
Generated:    no, hand-written
Status:       current
-->

# `COMPLEX` Keyword Reference

## Source Header

```c
// FILENAME: complex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (complex.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the COMPLEX statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares complex variables and arrays with real and imaginary components (Dartmouth DTSS).

## 2. Syntax

```basic
COMPLEX var1 [, var2, arr(dim1 [, dim2])]
```

## 3. Code Example

```basic
10 Val = COMPLEX var1 [, var2, arr(dim1 [, dim2])]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax error, Error 9: Subscript out of range

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Declarations
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | COMPLEX |
| Category | Variables & Declarations |
| Syntax | COMPLEX var1 [, var2, arr(dim1 [, dim2])] |
| Description | Declares complex variables and arrays with real and imaginary components (Dartmouth DTSS). |
| Error Summary | Error 2: Syntax error, Error 9: Subscript out of range |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/complex.c |
