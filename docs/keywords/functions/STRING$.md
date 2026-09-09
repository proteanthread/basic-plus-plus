<!--
Title:        STRING$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/manipulation/string.c
Generated:    no, hand-written
Status:       current
-->

# `STRING$` Keyword Reference

## Source Header

```c
// FILENAME: string.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libext,
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// Implements component functionality for string.c.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns a string of n repeating characters specified by ASCII code or 1st char of string.

## 2. Syntax

```basic
STRING$(n, char_spec)
```

## 3. Code Example

```basic
10 Val = STRING$(n, char_spec)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (STRING$ argument type error)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | STRING$ |
| Category | String Functions |
| Syntax | STRING$(n, char_spec) |
| Description | Returns a string of n repeating characters specified by ASCII code or 1st char of string. |
| Error Summary | Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (STRING$ argument type error) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/manipulation/string.c |
