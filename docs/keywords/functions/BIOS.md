<!--
Title:        BIOS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/bios.c
Generated:    no, hand-written
Status:       current
-->

# `BIOS` Keyword Reference

## Source Header

```c
// FILENAME: bios.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, libhardware, libkernel
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (bios.h, eval.h, eval.c, stmt.h, string.c, vm.h)
// NEEDS: libkernel (bus.h, bus.c, security.h, security.c, vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the BIOS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Executes low-level x86 BIOS interrupt routines using AX%, BX%, CX%, DX% variables.

## 2. Syntax

```basic
BIOS int_num
```

## 3. Code Example

```basic
10 REM BIOS Demonstration
20 PRINT "BIOS executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied

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
| Name | BIOS |
| Category | Devices & Network |
| Syntax | BIOS int_num |
| Description | Executes low-level x86 BIOS interrupt routines using AX%, BX%, CX%, DX% variables. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/bios.c |
