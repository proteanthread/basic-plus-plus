<!--
Title:        TTY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/tty.c
Generated:    no, hand-written
Status:       current
-->

# `TTY` Keyword Reference

## Source Header

```c
// FILENAME: tty.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, session.h)
// NEEDS: libengine (eval.h, tty.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the TTY statement in BASIC++.
```

## 1. Description & Usage

Displays or sets the current virtual terminal/teleprinter channel number.

## 2. Syntax

```basic
TTY [tty_number%]
```

## 3. Code Example

```basic
10 REM TTY Demonstration
20 PRINT "TTY executed successfully."
```

## 4. Error Conditions

Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Session & Multi-User
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TTY |
| Category | Session & Multi-User |
| Syntax | TTY [tty_number%] |
| Description | Displays or sets the current virtual terminal/teleprinter channel number. |
| Error Summary | Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/tty.c |
