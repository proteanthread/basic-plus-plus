<!--
Title:        LOOP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/loop.c
Generated:    no, hand-written
Status:       current
-->

# `LOOP` Keyword Reference

## Source Header

```c
// FILENAME: loop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, loop.h, string.c, vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the LOOP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Terminates a DO...LOOP block, or opens a BASIC09 structured LOOP...ENDLOOP block.

## 2. Syntax

```basic
LOOP [{WHILE|UNTIL} condition] | LOOP (BASIC09 block opener)
```

## 3. Code Example

```basic
10 Val = LOOP [{WHILE|UNTIL} condition] | LOOP (BASIC09 block opener)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 32: LOOP Without DO, Error 2: Syntax Error

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
| Name | LOOP |
| Category | Looping / Control Flow |
| Syntax | LOOP [{WHILE\|UNTIL} condition] \| LOOP (BASIC09 block opener) |
| Description | Terminates a DO...LOOP block, or opens a BASIC09 structured LOOP...ENDLOOP block. |
| Error Summary | Error 32: LOOP Without DO, Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/loop.c |
