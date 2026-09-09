<!--
Title:        ENUM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/structure/enum.c
Generated:    no, hand-written
Status:       current
-->

# `ENUM` Keyword Reference

## Source Header

```c
// FILENAME: enum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (enum.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the ENUM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines an enumeration group of named integer constants.

## 2. Syntax

```basic
ENUM enum_name 
 member [= value] 
 ... 
 END ENUM
```

## 3. Code Example

```basic
10 REM ENUM Demonstration
20 PRINT "ENUM executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 39: ENUM Without END ENUM, Error 10: Duplicate Definition

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
| Name | ENUM |
| Category | Variables & Memory |
| Syntax | ENUM enum_name 
 member [= value] 
 ... 
 END ENUM |
| Description | Defines an enumeration group of named integer constants. |
| Error Summary | Error 2: Syntax Error, Error 39: ENUM Without END ENUM, Error 10: Duplicate Definition |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/structure/enum.c |
