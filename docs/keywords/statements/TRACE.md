<!--
Title:        TRACE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/debug/logger.c
Generated:    no, hand-written
Status:       current
-->

# `TRACE` Keyword Reference

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

Configures execution line tracing, printing line numbers to console as they execute.

## 2. Syntax

```basic
TRACE {ON | OFF | line_num}
```

## 3. Code Example

```basic
10 REM TRACE Demonstration
20 PRINT "TRACE executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: Vintage BASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Debugging
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TRACE |
| Category | Debugging |
| Syntax | TRACE {ON \| OFF \| line_num} |
| Description | Configures execution line tracing, printing line numbers to console as they execute. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Vintage BASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/debug/logger.c |
