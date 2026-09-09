<!--
Title:        ENVIRON$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/environ.c
Generated:    no, hand-written
Status:       current
-->

# `ENVIRON$` Keyword Reference

## Source Header

```c
// FILENAME: environ.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (environ.h, string.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ENVIRON built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the value of host environment variable var_name$. Returns empty string if not set.

## 2. Syntax

```basic
ENVIRON$(var_name$)
```

## 3. Code Example

```basic
10 Val = ENVIRON$(var_name$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (ENVIRON$ expects one string argument)

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
| Name | ENVIRON$ |
| Category | System Functions |
| Syntax | ENVIRON$(var_name$) |
| Description | Returns the value of host environment variable var_name$. Returns empty string if not set. |
| Error Summary | Error 13: Type Mismatch (ENVIRON$ expects one string argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/environ.c |
