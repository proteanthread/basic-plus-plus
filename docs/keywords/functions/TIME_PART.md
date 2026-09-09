<!--
Title:        TIME_PART
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/datetime/time_part.c
Generated:    no, hand-written
Status:       current
-->

# `TIME_PART` Keyword Reference

## Source Header

```c
// FILENAME: time_part.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, memory.h, string.h)
// NEEDS: libengine (string.c, time_part.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for TIME_PART / TIMEPART built-in functions
```

## 1. Description & Usage

Extracts date or time component ("yyyy", "m", "d", "h", "n", "s", "w", "q", "yday", "ms") from date_serial or current time.

## 2. Syntax

```basic
TIME_PART(interval$ [, date_serial#]) / TIMEPART(...)
```

## 3. Code Example

```basic
10 Val = TIME_PART(interval$ [, date_serial#]) / TIMEPART(...)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Date & Time
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TIME_PART |
| Category | Date & Time |
| Syntax | TIME_PART(interval$ [, date_serial#]) / TIMEPART(...) |
| Description | Extracts date or time component ("yyyy", "m", "d", "h", "n", "s", "w", "q", "yday", "ms") from date_serial or current time. |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/datetime/time_part.c |
