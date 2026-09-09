<!--
Title:        DUMP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/debug/logger.c
Generated:    no, hand-written
Status:       current
-->

# `DUMP` Keyword Reference

## Source Header

```c
// FILENAME: logger.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libboot, libcore, libengine, libkern
// NEEDS: libcore (alloc.h, alloc.c, hal.h, logger.h, memops.h, memops.c)
// NEEDS: libcore (runtime_snprintf.h, runtime_snprintf.c, strops.h, strops.c)
// NEEDS: libplatform (platform.h)
// Provides multi-level logging, circular ring buffer, and pluggable sink inte
//
// ---- Includes ----
```

## 1. Description & Usage

Outputs a formatted hexadecimal and ASCII memory dump of variable contents or address range.

## 2. Syntax

```basic
DUMP [var_or_address [, length%]]
```

## 3. Code Example

```basic
10 REM DUMP Demonstration
20 PRINT "DUMP executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Debugging & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DUMP |
| Category | Debugging & Memory |
| Syntax | DUMP [var_or_address [, length%]] |
| Description | Outputs a formatted hexadecimal and ASCII memory dump of variable contents or address range. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/debug/logger.c |
