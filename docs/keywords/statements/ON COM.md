<!--
Title:        ON COM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/event/trapping/on_com.c
Generated:    no, hand-written
Status:       current
-->

# `ON COM` Keyword Reference

## Source Header

```c
// FILENAME: on_com.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, on_com.h, string.c, vm.
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the ON_COM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Establishes an event handler trap for serial communication port activity.

## 2. Syntax

```basic
ON COM(n) GOSUB line_num
```

## 3. Code Example

```basic
10 Val = ON COM(n) GOSUB line_num
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 8: Undefined Line Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Event Handling
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ON COM |
| Category | Event Handling |
| Syntax | ON COM(n) GOSUB line_num |
| Description | Establishes an event handler trap for serial communication port activity. |
| Error Summary | Error 5: Illegal Function Call, Error 8: Undefined Line Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/event/trapping/on_com.c |
