<!--
Title:        DATE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/time/date.c
Generated:    no, hand-written
Status:       current
-->

# `DATE$` Keyword Reference

## Source Header

```c
// FILENAME: date.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (date.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DATE built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the current system date string, or formats a numeric day number into DD-Mon-YY (DEC BASIC-PLUS).

## 2. Syntax

```basic
DATE$ | DATE$(day_num)
```

## 3. Code Example

```basic
10 Val = DATE$ | DATE$(day_num)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DATE$ |
| Category | System Functions |
| Syntax | DATE$ \| DATE$(day_num) |
| Description | Returns the current system date string, or formats a numeric day number into DD-Mon-YY (DEC BASIC-PLUS). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/time/date.c |
