<!--
Title:        FUNCTION
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/structure/function.c
Generated:    no, hand-written
Status:       current
-->

# `FUNCTION` Keyword Reference

## Source Header

```c
// FILENAME: function.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, function.h, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the FUNCTION statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares the name, parameters, and code that define a FUNCTION procedure block.

## 2. Syntax

```basic
FUNCTION name [(parameter_list)] ... END FUNCTION
```

## 3. Code Example

```basic
10 Val = FUNCTION name [(parameter_list)] ... END FUNCTION
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 35: SUB/FUNCTION Without END, Error 36: Illegal Parameter List

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
| Name | FUNCTION |
| Category | Control Flow |
| Syntax | FUNCTION name [(parameter_list)] ... END FUNCTION |
| Description | Declares the name, parameters, and code that define a FUNCTION procedure block. |
| Error Summary | Error 2: Syntax Error, Error 35: SUB/FUNCTION Without END, Error 36: Illegal Parameter List |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/structure/function.c |
