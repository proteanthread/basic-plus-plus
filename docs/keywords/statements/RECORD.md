<!--
Title:        RECORD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/structure/class.c
Generated:    no, hand-written
Status:       current
-->

# `RECORD` Keyword Reference

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

Defines a VAX BASIC / DEC RECORD structured composite data type.

## 2. Syntax

```basic
RECORD record_name 
 member_name AS type 
 ... 
 END RECORD
```

## 3. Code Example

```basic
10 REM RECORD Demonstration
20 PRINT "RECORD executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 38: RECORD Without END RECORD, Error 10: Duplicate Definition

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
| Name | RECORD |
| Category | Variables & Memory |
| Syntax | RECORD record_name 
 member_name AS type 
 ... 
 END RECORD |
| Description | Defines a VAX BASIC / DEC RECORD structured composite data type. |
| Error Summary | Error 2: Syntax Error, Error 38: RECORD Without END RECORD, Error 10: Duplicate Definition |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/structure/class.c |
