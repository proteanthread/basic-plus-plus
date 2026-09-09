<!--
Title:        BASENAME$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/func_scriptpath.c
Generated:    no, hand-written
Status:       current
-->

# `BASENAME$` Keyword Reference

## Source Header

```c
// FILENAME: func_scriptpath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_scriptpath.h)
// Provides runtime implementation and LanguageDescriptors for script and file
```

## 1. Description & Usage

Returns the base name of the running script without directory path or extension.

## 2. Syntax

```basic
BASENAME$ or BASNAME$
```

## 3. Code Example

```basic
10 REM BASENAME$ Demonstration
20 PRINT "BASENAME$ executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Environment
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BASENAME$ |
| Category | Environment |
| Syntax | BASENAME$ or BASNAME$ |
| Description | Returns the base name of the running script without directory path or extension. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/func_scriptpath.c |
