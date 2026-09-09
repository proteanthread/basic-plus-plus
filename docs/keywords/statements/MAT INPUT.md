<!--
Title:        MAT INPUT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/matrices/io/mat_input.c
Generated:    no, hand-written
Status:       current
-->

# `MAT INPUT` Keyword Reference

## Source Header

```c
// FILENAME: mat_input.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, mat_input.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the MAT_INPUT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads numeric or string matrix elements from console input or an open file stream (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
MAT INPUT [#file_num,] array_name [(num_rows [, num_cols])]
```

## 3. Code Example

```basic
10 Val = MAT INPUT [#file_num,] array_name [(num_rows [, num_cols])]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number, Error 62: Input Past End

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Matrix Operations
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MAT INPUT |
| Category | Matrix Operations |
| Syntax | MAT INPUT [#file_num,] array_name [(num_rows [, num_cols])] |
| Description | Reads numeric or string matrix elements from console input or an open file stream (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number, Error 62: Input Past End |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/matrices/io/mat_input.c |
