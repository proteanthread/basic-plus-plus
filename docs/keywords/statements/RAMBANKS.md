<!--
Title:        RAMBANKS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/vdev.c
Generated:    no, hand-written
Status:       current
-->

# `RAMBANKS` Keyword Reference

## Source Header

```c
// FILENAME: vdev.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkern
// NEEDS: libcore (alloc.h, ctype.h, hal.h, memops.h, runtime_snprintf.h, stro
// Implements virtual device subsystem, registry, subdevice multiplexing, and 
//
// ---- Includes ----
```

## 1. Description & Usage

Configures or reserves the total number of banked memory pages available to the runtime.

## 2. Syntax

```basic
RAMBANKS count%
```

## 3. Code Example

```basic
10 REM RAMBANKS Demonstration
20 PRINT "RAMBANKS executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Memory
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RAMBANKS |
| Category | Hardware & Memory |
| Syntax | RAMBANKS count% |
| Description | Configures or reserves the total number of banked memory pages available to the runtime. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/vdev.c |
