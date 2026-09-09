<!--
Title:        TYPE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/structure/type.c
Generated:    no, hand-written
Status:       current
-->

# `TYPE` Keyword Reference

## Source Header

```c
// FILENAME: type.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the TYPE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Defines a user-defined data structure (UDT/record) containing element fields.

## 2. Syntax

```basic
TYPE type_name 
 member_name AS type | member: type 
 ... 
 END TYPE | ENDTYPE
```

## 3. Code Example

```basic
10 REM TYPE Demonstration
20 PRINT "TYPE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 38: TYPE Without END TYPE, Error 10: Duplicate Definition

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
| Name | TYPE |
| Category | Variables & Memory |
| Syntax | TYPE type_name 
 member_name AS type \| member: type 
 ... 
 END TYPE \| ENDTYPE |
| Description | Defines a user-defined data structure (UDT/record) containing element fields. |
| Error Summary | Error 2: Syntax Error, Error 38: TYPE Without END TYPE, Error 10: Duplicate Definition |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/structure/type.c |
