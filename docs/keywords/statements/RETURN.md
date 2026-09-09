<!--
Title:        RETURN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/return.c
Generated:    no, hand-written
Status:       current
-->

# `RETURN` Keyword Reference

## Source Header

```c
// FILENAME: return.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (return.h, string.c, vm.h)
// Provides runtime implementation for the RETURN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Returns control from a GOSUB subroutine to the statement following GOSUB, or specified line.

## 2. Syntax

```basic
RETURN [line_num]
```

## 3. Code Example

```basic
10 REM RETURN Demonstration
20 PRINT "RETURN executed successfully."
```

## 4. Error Conditions

Error 3: RETURN without GOSUB (call stack empty)

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
| Name | RETURN |
| Category | Control Flow |
| Syntax | RETURN [line_num] |
| Description | Returns control from a GOSUB subroutine to the statement following GOSUB, or specified line. |
| Error Summary | Error 3: RETURN without GOSUB (call stack empty) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/return.c |
