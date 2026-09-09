<!--
Title:        DEVINFO$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_devinfo.c
Generated:    no, hand-written
Status:       current
-->

# `DEVINFO$` Keyword Reference

## Source Header

```c
// FILENAME: func_devinfo.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, sys_fn.c
// NEEDS: libkernel (types.h, errors.h, vdev.h), libcore (strings.h, language_
// Implementation for DEVINFO$ and DEVCAPS introspection functions.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns property metadata (CLASS, DRIVER, VERSION, TARGET, STATUS) for a registered virtual device.

## 2. Syntax

```basic
DEVINFO$(dev_name$, property$)
```

## 3. Code Example

```basic
10 Val = DEVINFO$(dev_name$, property$)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | DEVINFO$ |
| Category | System & Hardware |
| Syntax | DEVINFO$(dev_name$, property$) |
| Description | Returns property metadata (CLASS, DRIVER, VERSION, TARGET, STATUS) for a registered virtual device. |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_devinfo.c |
