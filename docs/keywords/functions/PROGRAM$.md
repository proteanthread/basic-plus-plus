<!--
Title:        PROGRAM$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/func_program.c
Generated:    no, hand-written
Status:       current
-->

# `PROGRAM$` Keyword Reference

## Source Header

```c
// FILENAME: func_program.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, memory.h, string.h)
// NEEDS: libengine (func_program.h)
// Provides runtime implementation for the PROGRAM$ program source introspecti
```

## 1. Description & Usage

Returns the source text of a stored program line or program metrics string.

## 2. Syntax

```basic
PROGRAM$(line_number) | PROGRAM$("COUNT")
```

## 3. Code Example

```basic
10 Val = PROGRAM$(line_number) | PROGRAM$("COUNT")
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PROGRAM$ |
| Category | Introspection |
| Syntax | PROGRAM$(line_number) \| PROGRAM$("COUNT") |
| Description | Returns the source text of a stored program line or program metrics string. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/func_program.c |
