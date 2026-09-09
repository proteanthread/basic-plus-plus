<!--
Title:        DEVCTL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/device/stmt_devctl.c
Generated:    no, hand-written
Status:       current
-->

# `DEVCTL` Keyword Reference

## Source Header

```c
// FILENAME: stmt_devctl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, str
// Implementation for DEVCTL statement micro-library.
//
// ---- Includes ----
```

## 1. Description & Usage

Directly executes a low-level IOCTL or control command on a device without requiring OPEN.

## 2. Syntax

```basic
DEVCTL dev_spec$, cmd [, aux1 [, aux2 [, data$]]]
```

## 3. Code Example

```basic
10 REM DEVCTL Demonstration
20 PRINT "DEVCTL executed successfully."
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
| Name | DEVCTL |
| Category | System & Hardware |
| Syntax | DEVCTL dev_spec$, cmd [, aux1 [, aux2 [, data$]]] |
| Description | Directly executes a low-level IOCTL or control command on a device without requiring OPEN. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/device/stmt_devctl.c |
