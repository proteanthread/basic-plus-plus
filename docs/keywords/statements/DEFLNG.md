<!--
Title:        DEFLNG
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/deflng.c
Generated:    no, hand-written
Status:       current
-->

# `DEFLNG` Keyword Reference

## Source Header

```c
// FILENAME: deflng.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, language_descriptor.h)
// NEEDS: libcore (string.h, variables.h, variables.c)
// NEEDS: libengine (deflng.h, string.c)
// Provides runtime implementation for the DEFLNG statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets default type of variables beginning with specified letters to long integer (32-bit).

## 2. Syntax

```basic
DEFLNG letter_range [, letter_range...]
```

## 3. Code Example

```basic
10 REM DEFLNG Demonstration
20 PRINT "DEFLNG executed successfully."
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
| Name | DEFLNG |
| Category | Variables & Types |
| Syntax | DEFLNG letter_range [, letter_range...] |
| Description | Sets default type of variables beginning with specified letters to long integer (32-bit). |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/deflng.c |
