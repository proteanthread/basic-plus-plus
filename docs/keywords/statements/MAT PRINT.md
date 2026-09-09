<!--
Title:        MAT PRINT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/matrices/io/mat_print.c
Generated:    no, hand-written
Status:       current
-->

# `MAT PRINT` Keyword Reference

## Source Header

```c
// FILENAME: mat_print.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h, mat_write.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for the MAT_PRINT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Outputs formatted 1D or 2D matrix array elements to console or file stream (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
MAT PRINT [#file_num,] array_name [;|,]
```

## 3. Code Example

```basic
10 REM MAT PRINT Demonstration
20 PRINT "MAT PRINT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number

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
| Name | MAT PRINT |
| Category | Matrix Operations |
| Syntax | MAT PRINT [#file_num,] array_name [;\|,] |
| Description | Outputs formatted 1D or 2D matrix array elements to console or file stream (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/matrices/io/mat_print.c |
