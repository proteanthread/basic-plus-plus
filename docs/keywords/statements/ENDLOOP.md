<!--
Title:        ENDLOOP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/endloop.c
Generated:    no, hand-written
Status:       current
-->

# `ENDLOOP` Keyword Reference

## Source Header

```c
// FILENAME: endloop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (endloop.h, eval.h, eval.c, string.c)
// Provides runtime implementation for the ENDLOOP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Terminates a BASIC09 structured LOOP...ENDLOOP block and loops back to LOOP.

## 2. Syntax

```basic
ENDLOOP
```

## 3. Code Example

```basic
10 REM ENDLOOP Demonstration
20 PRINT "ENDLOOP executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 32: ENDLOOP Without LOOP

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
| Name | ENDLOOP |
| Category | Looping / Control Flow |
| Syntax | ENDLOOP |
| Description | Terminates a BASIC09 structured LOOP...ENDLOOP block and loops back to LOOP. |
| Error Summary | Error 2: Syntax Error, Error 32: ENDLOOP Without LOOP |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/endloop.c |
