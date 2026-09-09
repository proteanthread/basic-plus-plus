<!--
Title:        VARPTR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/functions/varptr.c
Generated:    no, hand-written
Status:       current
-->

# `VARPTR` Keyword Reference

## Source Header

```c
// FILENAME: varptr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (varptr.h)
// NEEDS: libengine (string.c, vm.h)
// Provides runtime implementation for the VARPTR built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Returns the 64-bit integer memory address or descriptor string for a variable.

## 2. Syntax

```basic
VARPTR(var) | VARPTR$(var)
```

## 3. Code Example

```basic
10 Val = VARPTR(var) | VARPTR$(var)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VARPTR |
| Category | Variables & Memory |
| Syntax | VARPTR(var) \| VARPTR$(var) |
| Description | Returns the 64-bit integer memory address or descriptor string for a variable. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/functions/varptr.c |
