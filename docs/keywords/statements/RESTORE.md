<!--
Title:        RESTORE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/data/restore.c
Generated:    no, hand-written
Status:       current
-->

# `RESTORE` Keyword Reference

## Source Header

```c
// FILENAME: restore.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, metadata.h, metadata.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, restore.h, string.c)
// NEEDS: libengine (vm.h)
// Provides runtime implementation for the RESTORE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Resets the DATA statement reading pointer or file position to the beginning or specified line.

## 2. Syntax

```basic
RESTORE [line_number | label] | RESTORE #file_num
```

## 3. Code Example

```basic
10 REM RESTORE Demonstration
20 PRINT "RESTORE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

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
| Name | RESTORE |
| Category | Variables & Memory |
| Syntax | RESTORE [line_number \| label] \| RESTORE #file_num |
| Description | Resets the DATA statement reading pointer or file position to the beginning or specified line. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/data/restore.c |
