<!--
Title:        LPRINT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/lprint.c
Generated:    no, hand-written
Status:       current
-->

# `LPRINT` Keyword Reference

## Source Header

```c
// FILENAME: lprint.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libcore (using.h)
// NEEDS: libengine (eval.h, eval.c, lprint.h, string.c)
// NEEDS: libkernel (vprinter.h, vprinter.c)
// Provides runtime implementation for the LPRINT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Prints formatted or unformatted data to the line printer.

## 2. Syntax

```basic
LPRINT [USING format$;] expression_list [; | ,]
```

## 3. Code Example

```basic
10 REM LPRINT Demonstration
20 PRINT "LPRINT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Line Printer I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LPRINT |
| Category | Line Printer I/O |
| Syntax | LPRINT [USING format$;] expression_list [; \| ,] |
| Description | Prints formatted or unformatted data to the line printer. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/lprint.c |
