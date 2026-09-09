<!--
Title:        CONSOLE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/platform/console/plat_console.c
Generated:    no, hand-written
Status:       current
-->

# `CONSOLE` Keyword Reference

## Source Header

```c
// FILENAME: plat_console.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for CONSOLE.
```

## 1. Description & Usage

Redirects standard console terminal input and output to an alternate virtual device or port.

## 2. Syntax

```basic
CONSOLE [device$ | mode%]
```

## 3. Code Example

```basic
10 REM CONSOLE Demonstration
20 PRINT "CONSOLE executed successfully."
```

## 4. Error Conditions

Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console I/O
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CONSOLE |
| Category | Console I/O |
| Syntax | CONSOLE [device$ \| mode%] |
| Description | Redirects standard console terminal input and output to an alternate virtual device or port. |
| Error Summary | Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/platform/console/plat_console.c |
