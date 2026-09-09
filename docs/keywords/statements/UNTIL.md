<!--
Title:        UNTIL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/until.c
Generated:    no, hand-written
Status:       current
-->

# `UNTIL` Keyword Reference

## Source Header

```c
// FILENAME: until.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, until.h)
// Provides runtime implementation for the UNTIL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Terminates a BASIC09 REPEAT...UNTIL loop block when condition becomes true.

## 2. Syntax

```basic
UNTIL condition
```

## 3. Code Example

```basic
10 REM UNTIL Demonstration
20 PRINT "UNTIL executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 32: UNTIL Without REPEAT

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
| Name | UNTIL |
| Category | Looping / Control Flow |
| Syntax | UNTIL condition |
| Description | Terminates a BASIC09 REPEAT...UNTIL loop block when condition becomes true. |
| Error Summary | Error 2: Syntax Error, Error 32: UNTIL Without REPEAT |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/until.c |
