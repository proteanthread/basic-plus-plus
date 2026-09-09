<!--
Title:        DEVICE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/device/stmt_device.c
Generated:    no, hand-written
Status:       current
-->

# `DEVICE` Keyword Reference

## Source Header

```c
// FILENAME: stmt_device.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, str
// Implementation for DEVICE and DEVICES statements in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Manages virtual devices, aliases, User-Defined Devices (UDD), and dynamic driver bindings.

## 2. Syntax

```basic
DEVICE "dev_name:", "config_param=val"
```

## 3. Code Example

```basic
10 REM DEVICE Demonstration
20 PRINT "DEVICE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEVICE |
| Category | System & Hardware |
| Syntax | DEVICE "dev_name:", "config_param=val" |
| Description | Manages virtual devices, aliases, User-Defined Devices (UDD), and dynamic driver bindings. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/device/stmt_device.c |
