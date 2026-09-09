<!--
Title:        SYS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/sys.c
Generated:    no, hand-written
Status:       current
-->

# `SYS` Keyword Reference

## Source Header

```c
// FILENAME: sys.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, string.c, sys.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the SYS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Executes machine code subroutine at memory address or runs host OS command.

## 2. Syntax

```basic
SYS address% | SYS command_string$
```

## 3. Code Example

```basic
10 REM SYS Demonstration
20 PRINT "SYS executed successfully."
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
| Name | SYS |
| Category | System & Execution |
| Syntax | SYS address% \| SYS command_string$ |
| Description | Executes machine code subroutine at memory address or runs host OS command. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/sys.c |
