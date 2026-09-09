<!--
Title:        ON KEY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/trapping/on_key.c
Generated:    no, hand-written
Status:       current
-->

# `ON KEY` Keyword Reference

## Source Header

```c
// FILENAME: on_key.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, on_key.h, string.c, vm.
// NEEDS: libkernel (vcon.h, vcon.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ON_KEY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Establishes an asynchronous interrupt handler for function key presses, or toggles/customizes row 25 function key labels.

## 2. Syntax

```basic
ON KEY(n) GOSUB line_label | KEY(n) {ON|OFF|STOP} | KEY ON | KEY OFF | KEY n, string
```

## 3. Code Example

```basic
10 Val = ON KEY(n) GOSUB line_label | KEY(n) {ON|OFF|STOP} | KEY ON | KEY OFF | KEY n, string
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Trapping
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ON KEY |
| Category | Event Trapping |
| Syntax | ON KEY(n) GOSUB line_label \| KEY(n) {ON\|OFF\|STOP} \| KEY ON \| KEY OFF \| KEY n, string |
| Description | Establishes an asynchronous interrupt handler for function key presses, or toggles/customizes row 25 function key labels. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/trapping/on_key.c |
