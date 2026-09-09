<!--
Title:        SPC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/format/spc.c
Generated:    no, hand-written
Status:       current
-->

# `SPC` Keyword Reference

## Source Header

```c
// FILENAME: spc.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (spc.h, string.c, vm.h)
// Provides runtime implementation for the SPC built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Outputs or generates n space characters in a PRINT statement or expression.

## 2. Syntax

```basic
SPC(n%)
```

## 3. Code Example

```basic
10 Val = SPC(n%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPC expects one numeric argument)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Print / Formatting Functions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SPC |
| Category | Print / Formatting Functions |
| Syntax | SPC(n%) |
| Description | Outputs or generates n space characters in a PRINT statement or expression. |
| Error Summary | Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPC expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/format/spc.c |
