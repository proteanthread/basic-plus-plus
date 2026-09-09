<!--
Title:        DO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/do.c
Generated:    no, hand-written
Status:       current
-->

# `DO` Keyword Reference

## Source Header

```c
// FILENAME: do.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (do.h, eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the DO statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Initiates a structured DO...LOOP block, optionally evaluating a WHILE or UNTIL pre-condition.

## 2. Syntax

```basic
DO [{WHILE|UNTIL} condition]
```

## 3. Code Example

```basic
10 REM DO Demonstration
20 PRINT "DO executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 31: DO Without LOOP

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Looping / Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DO |
| Category | Looping / Control Flow |
| Syntax | DO [{WHILE\|UNTIL} condition] |
| Description | Initiates a structured DO...LOOP block, optionally evaluating a WHILE or UNTIL pre-condition. |
| Error Summary | Error 2: Syntax Error, Error 31: DO Without LOOP |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/do.c |
