<!--
Title:        DEF USR
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/hardware/def_usr.c
Generated:    no, hand-written
Status:       current
-->

# `DEF USR` Keyword Reference

## Source Header

```c
// FILENAME: def_usr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (def_usr.h, string.c)
// Provides runtime implementation for the DEF_USR statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Specifies the starting address of an assembly language subroutine called by USR.

## 2. Syntax

```basic
DEF USR[digit%] = address%
```

## 3. Code Example

```basic
10 REM DEF USR Demonstration
20 PRINT "DEF USR executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEF USR |
| Category | System & Memory |
| Syntax | DEF USR[digit%] = address% |
| Description | Specifies the starting address of an assembly language subroutine called by USR. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/hardware/def_usr.c |
