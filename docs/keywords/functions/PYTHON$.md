<!--
Title:        PYTHON$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/func_python.c
Generated:    no, hand-written
Status:       current
-->

# `PYTHON$` Keyword Reference

## Source Header

```c
// FILENAME: func_python.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (funcreg.h, funcreg.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (string.c, vm.h)
// Implements the PYTHON$ built-in function to evaluate Python expressions.
//
// ---- Includes ----
```

## 1. Description & Usage

Evaluates a Python expression string and returns resulting string representation.

## 2. Syntax

```basic
PYTHON$(expr$)
```

## 3. Code Example

```basic
10 Val = PYTHON$(e10pr$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Language Interop
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PYTHON$ |
| Category | Language Interop |
| Syntax | PYTHON$(expr$) |
| Description | Evaluates a Python expression string and returns resulting string representation. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/func_python.c |
