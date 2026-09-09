<!--
Title:        DEVCTL$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_devctl.c
Generated:    no, hand-written
Status:       current
-->

# `DEVCTL$` Keyword Reference

## Source Header

```c
// FILENAME: func_devctl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, sys_fn.c
// NEEDS: libkernel (types.h, errors.h, vdev.h, dev_user.h), libcore (strings.
// Implementation for DEVCTL, DEVCTL$, and MESG built-in functions.
//
// ---- Includes ----
```

## 1. Description & Usage

Sends hardware device control command and returns response string.

## 2. Syntax

```basic
DEVCTL$(dev$, cmd[, arg1, arg2])
```

## 3. Code Example

```basic
10 Val = DEVCTL$(dev$, cmd[, arg1, arg2])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call (invalid device or command)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Device Control
- **Subsystem**: SUBSYSTEM_KERNEL
- **Safety Level**: SAFETY_UNSAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEVCTL$ |
| Category | Device Control |
| Syntax | DEVCTL$(dev$, cmd[, arg1, arg2]) |
| Description | Sends hardware device control command and returns response string. |
| Error Summary | Error 5: Illegal Function Call (invalid device or command) |
| Subsystem | SUBSYSTEM_KERNEL |
| Safety Level | SAFETY_UNSAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_devctl.c |
