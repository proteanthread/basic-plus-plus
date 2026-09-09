<!--
Title:        MAT WRITE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/matrices/io/mat_write.c
Generated:    no, hand-written
Status:       current
-->

# `MAT WRITE` Keyword Reference

## Source Header

```c
// FILENAME: mat_write.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_internal.h)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (mat_print.h, mat_print.c, mat_write.h)
// Provides runtime implementation for the MAT_WRITE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Outputs formatted matrix elements to an open file stream or console (Timesharing Matrix File I/O).

## 2. Syntax

```basic
MAT WRITE [#channel,] array_name [;|,]
```

## 3. Code Example

```basic
10 REM MAT WRITE Demonstration
20 PRINT "MAT WRITE executed successfully."
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
| Name | MAT WRITE |
| Category | Matrix Operations |
| Syntax | MAT WRITE [#channel,] array_name [;\|,] |
| Description | Outputs formatted matrix elements to an open file stream or console (Timesharing Matrix File I/O). |
| Error Summary | Error 2: Syntax Error, Error 9: Subscript Out of Range, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/matrices/io/mat_write.c |
