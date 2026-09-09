<!--
Title:        INKEY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/platform/console/plat_console.c
Generated:    no, hand-written
Status:       current
-->

# `INKEY` Keyword Reference

## Source Header

```c
// FILENAME: plat_console.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for INKEY.
```

## 1. Description & Usage

Reads a single character from the console keyboard buffer without waiting or with optional timeout.

## 2. Syntax

```basic
INKEY [(timeout_ms)]
```

## 3. Code Example

```basic
10 Val = INKEY [(timeout_ms)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console Input
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INKEY |
| Category | Console Input |
| Syntax | INKEY [(timeout_ms)] |
| Description | Reads a single character from the console keyboard buffer without waiting or with optional timeout. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/platform/console/plat_console.c |
