<!--
Title:        RENUM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/renum.c
Generated:    no, hand-written
Status:       current
-->

# `RENUM` Keyword Reference

## Source Header

```c
// FILENAME: renum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (runtime_snprintf.h, runtime_snprintf.c, string.h)
// NEEDS: libengine (lexer.h, lexer.c, renum.h, string.c, vm.h)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the RENUM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Renumbers program lines in memory or on disk, updating branch targets.

## 2. Syntax

```basic
RENUM ["file.bas" [,start [,step]] [TO "out.bas"]] | [new_start] [,[old_start] [,step]]
```

## 3. Code Example

```basic
10 REM RENUM Demonstration
20 PRINT "RENUM executed successfully."
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
| Name | RENUM |
| Category | Program Mgmt & Editing |
| Syntax | RENUM ["file.bas" [,start [,step]] [TO "out.bas"]] \| [new_start] [,[old_start] [,step]] |
| Description | Renumbers program lines in memory or on disk, updating branch targets. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/renum.c |
