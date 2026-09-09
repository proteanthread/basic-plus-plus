<!--
Title:        DIR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/dir_fn.c
Generated:    no, hand-written
Status:       current
-->

# `DIR$` Keyword Reference

## Source Header

```c
// FILENAME: dir_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (dir_fn.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the DIR$ function in BASIC++.
```

## 1. Description & Usage

Returns the next matching filename matching the filespec filter.

## 2. Syntax

```basic
DIR$([filespec$])
```

## 3. Code Example

```basic
10 Val = DIR$([filespec$])
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
| Name | DIR$ |
| Category | File Functions |
| Syntax | DIR$([filespec$]) |
| Description | Returns the next matching filename matching the filespec filter. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/dir_fn.c |
