<!--
Title:        DEFDBL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/defdbl.c
Generated:    no, hand-written
Status:       current
-->

# `DEFDBL` Keyword Reference

## Source Header

```c
// FILENAME: defdbl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h, variables.h, variables.c)
// NEEDS: libengine (defdbl.h, string.c)
// Provides runtime implementation for the DEFDBL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets default type of variables beginning with specified letters to double-precision float.

## 2. Syntax

```basic
DEFDBL letter_range [, letter_range...]
```

## 3. Code Example

```basic
10 REM DEFDBL Demonstration
20 PRINT "DEFDBL executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Types
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEFDBL |
| Category | Variables & Types |
| Syntax | DEFDBL letter_range [, letter_range...] |
| Description | Sets default type of variables beginning with specified letters to double-precision float. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/defdbl.c |
