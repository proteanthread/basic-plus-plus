<!--
Title:        CURDIR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/curdir.c
Generated:    no, hand-written
Status:       current
-->

# `CURDIR$` Keyword Reference

## Source Header

```c
// FILENAME: curdir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (curdir.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the CURDIR$ function in BASIC++.
```

## 1. Description & Usage

Returns the current working directory path.

## 2. Syntax

```basic
CURDIR$([drive$])
```

## 3. Code Example

```basic
10 Val = CURDIR$([drive$])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CURDIR$ |
| Category | File Functions |
| Syntax | CURDIR$([drive$]) |
| Description | Returns the current working directory path. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/curdir.c |
