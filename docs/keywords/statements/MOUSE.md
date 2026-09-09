<!--
Title:        MOUSE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/mouse.c
Generated:    no, hand-written
Status:       current
-->

# `MOUSE` Keyword Reference

## Source Header

```c
// FILENAME: mouse.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, mouse.h, string.c, vm.h
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MOUSE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Enables, disables, shows, or hides the mouse pointer and event tracking.

## 2. Syntax

```basic
MOUSE ON | OFF | SHOW | HIDE | INPUT
```

## 3. Code Example

```basic
10 REM MOUSE Demonstration
20 PRINT "MOUSE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Input / Output
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MOUSE |
| Category | Input / Output |
| Syntax | MOUSE ON \| OFF \| SHOW \| HIDE \| INPUT |
| Description | Enables, disables, shows, or hides the mouse pointer and event tracking. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/mouse.c |
