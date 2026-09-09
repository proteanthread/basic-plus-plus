<!--
Title:        KEY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/trapping/key.c
Generated:    no, hand-written
Status:       current
-->

# `KEY` Keyword Reference

## Source Header

```c
// FILENAME: key.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, key.h, lexer.h, lexer.c, string.c, vm.h)
// NEEDS: libkernel (errors.h, vcon.h, vcon.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the KEY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Enables, disables, or customizes function key softkey display and key event trapping.

## 2. Syntax

```basic
KEY ON | KEY OFF | KEY LIST | KEY n, string$ | KEY(n) {ON|OFF|STOP}
```

## 3. Code Example

```basic
10 Val = KEY ON | KEY OFF | KEY LIST | KEY n, string$ | KEY(n) {ON|OFF|STOP}
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Trapping & Console
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | KEY |
| Category | Event Trapping & Console |
| Syntax | KEY ON \| KEY OFF \| KEY LIST \| KEY n, string$ \| KEY(n) {ON\|OFF\|STOP} |
| Description | Enables, disables, or customizes function key softkey display and key event trapping. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/trapping/key.c |
