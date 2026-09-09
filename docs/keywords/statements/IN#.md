<!--
Title:        IN#
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/device/stmt_slot_io.c
Generated:    no, hand-written
Status:       current
-->

# `IN#` Keyword Reference

## Source Header

```c
// FILENAME: stmt_slot_io.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, str
// Implementation for Apple II PR# and IN# statement micro-library.
//
// ---- Includes ----
```

## 1. Description & Usage

Redirects keyboard and character input to Apple II peripheral expansion slot 0..7.

## 2. Syntax

```basic
IN# slot_number
```

## 3. Code Example

```basic
10 REM IN# Demonstration
20 PRINT "IN# executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

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
| Name | IN# |
| Category | System & Hardware |
| Syntax | IN# slot_number |
| Description | Redirects keyboard and character input to Apple II peripheral expansion slot 0..7. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/device/stmt_slot_io.c |
