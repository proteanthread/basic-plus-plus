<!--
Title:        BANK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/vdev.c
Generated:    no, hand-written
Status:       current
-->

# `BANK` Keyword Reference

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

Selects active expanded memory bank (EMS/XMS or retro banked RAM) for segmented addressing.

## 2. Syntax

```basic
BANK bank_number%
```

## 3. Code Example

```basic
10 REM BANK Demonstration
20 PRINT "BANK executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: Commodore 128 BASIC 7.0, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Memory
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BANK |
| Category | Hardware & Memory |
| Syntax | BANK bank_number% |
| Description | Selects active expanded memory bank (EMS/XMS or retro banked RAM) for segmented addressing. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Commodore 128 BASIC 7.0, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/vdev.c |
