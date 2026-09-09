<!--
Title:        COMMON
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/declaration/common.c
Generated:    no, hand-written
Status:       current
-->

# `COMMON` Keyword Reference

## Source Header

```c
// FILENAME: common.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (common.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the COMMON statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares global variables to be preserved across CHAIN and module boundaries.

## 2. Syntax

```basic
COMMON [SHARED] var1 [()] [, var2 [()]...]
```

## 3. Code Example

```basic
10 Val = COMMON [SHARED] var1 [()] [, var2 [()]...]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

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
| Name | COMMON |
| Category | Variables & Memory |
| Syntax | COMMON [SHARED] var1 [()] [, var2 [()]...] |
| Description | Declares global variables to be preserved across CHAIN and module boundaries. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/declaration/common.c |
