<!--
Title:        TAB
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/string/format/tab.c
Generated:    no, hand-written
Status:       current
-->

# `TAB` Keyword Reference

## Source Header

```c
// FILENAME: tab.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, tab.h, vm.h)
// NEEDS: libkernel (vcon.h, vcon.c)
// Provides runtime implementation for the TAB built-in function in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Positions output at the specified column in a PRINT statement.

## 2. Syntax

```basic
TAB(column%)
```

## 3. Code Example

```basic
10 Val = TAB(column%)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch (TAB expects one numeric argument)

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
| Name | TAB |
| Category | Print / Formatting Functions |
| Syntax | TAB(column%) |
| Description | Positions output at the specified column in a PRINT statement. |
| Error Summary | Error 13: Type Mismatch (TAB expects one numeric argument) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/string/format/tab.c |
