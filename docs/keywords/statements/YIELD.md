<!--
Title:        YIELD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/flow/stmt_yield.c
Generated:    no, hand-written
Status:       current
-->

# `YIELD` Keyword Reference

## Source Header

```c
// FILENAME: stmt_yield.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h), libplatform (platform.h)
// Provides runtime implementation for the YIELD cooperative multitasking stat
//
// ---- Includes ----
```

## 1. Description & Usage

Yields the active thread execution timeslice to background tasks and OS event pump.

## 2. Syntax

```basic
YIELD
```

## 3. Code Example

```basic
10 REM YIELD Demonstration
20 PRINT "YIELD executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | YIELD |
| Category | Control Flow |
| Syntax | YIELD |
| Description | Yields the active thread execution timeslice to background tasks and OS event pump. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/flow/stmt_yield.c |
