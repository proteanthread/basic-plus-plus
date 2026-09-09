<!--
Title:        TZ$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/platform/time/plat_time.c
Generated:    no, hand-written
Status:       current
-->

# `TZ$` Keyword Reference

## Source Header

```c
// FILENAME: plat_time.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for TZ$.
```

## 1. Description & Usage

Returns the short timezone abbreviation string (e.g. 'MDT', 'UTC', 'EST').

## 2. Syntax

```basic
TZ$
```

## 3. Code Example

```basic
10 REM TZ$ Demonstration
20 PRINT "TZ$ executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Date & Time
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TZ$ |
| Category | Date & Time |
| Syntax | TZ$ |
| Description | Returns the short timezone abbreviation string (e.g. 'MDT', 'UTC', 'EST'). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/platform/time/plat_time.c |
