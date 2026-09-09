<!--
Title:        EXTERNAL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/external/external.c
Generated:    no, hand-written
Status:       current
-->

# `EXTERNAL` Keyword Reference

## Source Header

```c
// FILENAME: external.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (external.h, lexer.h, lexer.c, string.c, vm.h)
// Provides runtime implementation for the EXTERNAL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares an external SUB or FUNCTION procedure in ECMA-116 standard BASIC.

## 2. Syntax

```basic
EXTERNAL SUB|FUNCTION name [(parameter_list)]
```

## 3. Code Example

```basic
10 Val = EXTERNAL SUB|FUNCTION name [(parameter_list)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EXTERNAL |
| Category | Control Flow |
| Syntax | EXTERNAL SUB\|FUNCTION name [(parameter_list)] |
| Description | Declares an external SUB or FUNCTION procedure in ECMA-116 standard BASIC. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/external/external.c |
