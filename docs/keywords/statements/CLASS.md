<!--
Title:        CLASS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/structure/class.c
Generated:    no, hand-written
Status:       current
-->

# `CLASS` Keyword Reference

## Source Header

```c
// FILENAME: class.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (record.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (struct.h, struct.c)
// NEEDS: libengine (class.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h
// Provides runtime implementation for the CLASS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines an Object-Oriented class with inheritance and encapsulation support.

## 2. Syntax

```basic
CLASS class_name [EXTENDS parent] 
 member_name AS type 
 ... 
 END CLASS
```

## 3. Code Example

```basic
10 REM CLASS Demonstration
20 PRINT "CLASS executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 38: CLASS Without END CLASS, Error 10: Duplicate Definition

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Object-Oriented Programming
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CLASS |
| Category | Object-Oriented Programming |
| Syntax | CLASS class_name [EXTENDS parent] 
 member_name AS type 
 ... 
 END CLASS |
| Description | Defines an Object-Oriented class with inheritance and encapsulation support. |
| Error Summary | Error 2: Syntax Error, Error 38: CLASS Without END CLASS, Error 10: Duplicate Definition |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/structure/class.c |
