<!--
Title:        WEND
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/wend.c
Generated:    no, hand-written
Status:       current
-->

# `WEND` Keyword Reference

## Source Header

```c
// FILENAME: wend.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h, wend.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the WEND statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Marks the end of a WHILE loop block and jumps back to re-evaluate the WHILE condition.

## 2. Syntax

```basic
WEND
```

## 3. Code Example

```basic
10 REM WEND Demonstration
20 PRINT "WEND executed successfully."
```

## 4. Error Conditions

Error 29: WEND Without WHILE

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
| Name | WEND |
| Category | Looping / Control Flow |
| Syntax | WEND |
| Description | Marks the end of a WHILE loop block and jumps back to re-evaluate the WHILE condition. |
| Error Summary | Error 29: WEND Without WHILE |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/wend.c |
