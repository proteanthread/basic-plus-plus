<!--
Title:        PAUSE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/pause.c
Generated:    no, hand-written
Status:       current
-->

# `PAUSE` Keyword Reference

## Source Header

```c
// FILENAME: pause.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, pause.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the PAUSE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Pauses program execution or prompts user to press a key.

## 2. Syntax

```basic
PAUSE [seconds | prompt$]
```

## 3. Code Example

```basic
10 REM PAUSE Demonstration
20 PRINT "PAUSE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Execution
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PAUSE |
| Category | System & Execution |
| Syntax | PAUSE [seconds \| prompt$] |
| Description | Pauses program execution or prompts user to press a key. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/pause.c |
