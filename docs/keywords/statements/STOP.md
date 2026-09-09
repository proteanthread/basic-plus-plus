<!--
Title:        STOP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/stop.c
Generated:    no, hand-written
Status:       current
-->

# `STOP` Keyword Reference

## Source Header

```c
// FILENAME: stop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (stop.h, string.c, vm.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the STOP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Pauses program execution and prints 'Break in line X' message.

## 2. Syntax

```basic
STOP
```

## 3. Code Example

```basic
10 REM STOP Demonstration
20 PRINT "STOP executed successfully."
```

## 4. Error Conditions

None (STOP halts program execution cleanly)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | STOP |
| Category | Control Flow |
| Syntax | STOP |
| Description | Pauses program execution and prints 'Break in line X' message. |
| Error Summary | None (STOP halts program execution cleanly) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/stop.c |
