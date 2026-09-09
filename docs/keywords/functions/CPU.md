<!--
Title:        CPU
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/functions/hardware/func_cpu86.c
Generated:    no, hand-written
Status:       current
-->

# `CPU` Keyword Reference

## Source Header

```c
// FILENAME: func_cpu86.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: reg/reg_hw.h, bios/bios_cpu8086.h, runtime/funcreg.h, runtime/langua
// Implements the freestanding CPU / CPU86 function in BASIC++.
```

## 1. Description & Usage

Controls 8086/8088 virtual CPU execution, interrupts, and hardware registers.

## 2. Syntax

```basic
CPU(op$, ...) | CPU86(op$, ...)
```

## 3. Code Example

```basic
10 Val = CPU(op$, ...) | CPU86(op$, ...)
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CPU |
| Category | System & Hardware |
| Syntax | CPU(op$, ...) \| CPU86(op$, ...) |
| Description | Controls 8086/8088 virtual CPU execution, interrupts, and hardware registers. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/functions/hardware/func_cpu86.c |
