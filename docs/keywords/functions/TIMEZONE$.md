<!--
Title:        TIMEZONE$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/platform/time/plat_time.c
Generated:    no, hand-written
Status:       current
-->

# `TIMEZONE$` Keyword Reference

## Source Header

```c
// FILENAME: plat_time.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for TIMEZONE$.
```

## 1. Description & Usage

Returns the host system timezone descriptive name string (e.g. 'Mountain Daylight Time').

## 2. Syntax

```basic
TIMEZONE$
```

## 3. Code Example

```basic
10 REM TIMEZONE$ Demonstration
20 PRINT "TIMEZONE$ executed successfully."
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
| Name | TIMEZONE$ |
| Category | Date & Time |
| Syntax | TIMEZONE$ |
| Description | Returns the host system timezone descriptive name string (e.g. 'Mountain Daylight Time'). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/platform/time/plat_time.c |
