<!--
Title:        SPEC%
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/environment/spec_fn.c
Generated:    no, hand-written
Status:       current
-->

# `SPEC%` Keyword Reference

## Source Header

```c
// FILENAME: spec_fn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (spec_fn.h, string.c)
// Provides runtime implementation for the SPEC_FN built-in function in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Executes special device control operations (DEC RSTS/E).

## 2. Syntax

```basic
SPEC%(code, arg1 [, arg2])
```

## 3. Code Example

```basic
10 Val = SPEC%(code, arg1 [, arg2])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 13: Type Mismatch

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
| Name | SPEC% |
| Category | System Functions |
| Syntax | SPEC%(code, arg1 [, arg2]) |
| Description | Executes special device control operations (DEC RSTS/E). |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/environment/spec_fn.c |
