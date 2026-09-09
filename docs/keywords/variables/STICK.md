<!--
Title:        STICK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/joystick.c
Generated:    no, hand-written
Status:       current
-->

# `STICK` Keyword Reference

## Source Header

```c
// FILENAME: joystick.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, joystick.h, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the JOYSTICK statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Enables, disables, or suspends joystick axis and trigger button event trapping.

## 2. Syntax

```basic
STICK ON | OFF | STOP | STRIG ON | OFF | STOP
```

## 3. Code Example

```basic
10 REM STICK Demonstration
20 PRINT "STICK executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Devices & Network
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | STICK |
| Category | Devices & Network |
| Syntax | STICK ON \| OFF \| STOP \| STRIG ON \| OFF \| STOP |
| Description | Enables, disables, or suspends joystick axis and trigger button event trapping. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/joystick.c |
