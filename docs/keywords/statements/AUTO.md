<!--
Title:        AUTO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/auto.c
Generated:    no, hand-written
Status:       current
-->

# `AUTO` Keyword Reference

## Source Header

```c
// FILENAME: auto.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (auto.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the AUTO statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Enables automatic line number generation starting at start_line with step interval.

## 2. Syntax

```basic
AUTO [start_line] [,step]
```

## 3. Code Example

```basic
10 REM AUTO Demonstration
20 PRINT "AUTO executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Mgmt & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | AUTO |
| Category | Program Mgmt & Editing |
| Syntax | AUTO [start_line] [,step] |
| Description | Enables automatic line number generation starting at start_line with step interval. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/auto.c |
