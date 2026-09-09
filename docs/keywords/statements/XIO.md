<!--
Title:        XIO
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/device/stmt_xio.c
Generated:    no, hand-written
Status:       current
-->

# `XIO` Keyword Reference

## Source Header

```c
// FILENAME: stmt_xio.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h, vdev.h), libcore (eval.h, str
// Implementation for Atari XIO statement micro-library.
//
// ---- Includes ----
```

## 1. Description & Usage

Performs extended device I/O control operations (format, lock, unlock, rename, baud rate) across virtual and hardware devices (Atari CIO).

## 2. Syntax

```basic
XIO cmd, [#]channel, aux1, aux2, "filespec$"
```

## 3. Code Example

```basic
10 REM XIO Demonstration
20 PRINT "XIO executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch

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
| Name | XIO |
| Category | System & Hardware |
| Syntax | XIO cmd, [#]channel, aux1, aux2, "filespec$" |
| Description | Performs extended device I/O control operations (format, lock, unlock, rename, baud rate) across virtual and hardware devices (Atari CIO). |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/device/stmt_xio.c |
