<!--
Title:        MODDIR$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/moddir.c
Generated:    no, hand-written
Status:       current
-->

# `MODDIR$` Keyword Reference

## Source Header

```c
// FILENAME: moddir.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (moddir.h, string.c, vm.h)
// Provides runtime implementation for the MODDIR built-in function in BASIC++
//
// ---- Includes ----
```

## 1. Description & Usage

Returns active OS-9 Level 2 memory modules directory listing or module count.

## 2. Syntax

```basic
modules$ = MODDIR$ | count% = MODDIR(0)
```

## 3. Code Example

```basic
10 Val = modules$ = MODDIR$ | count% = MODDIR(0)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Introspection
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MODDIR$ |
| Category | System Introspection |
| Syntax | modules$ = MODDIR$ \| count% = MODDIR(0) |
| Description | Returns active OS-9 Level 2 memory modules directory listing or module count. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/moddir.c |
