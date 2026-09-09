<!--
Title:        BRIGHT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/gfx.c
Generated:    no, hand-written
Status:       current
-->

# `BRIGHT` Keyword Reference

## Source Header

```c
// FILENAME: gfx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore, libengine, libkernel, libplatform
// Implements virtual device and graphics rendering logic for gfx.
//
// ---- Includes ----
```

## 1. Description & Usage

Controls ZX Spectrum text foreground and background highlight attribute (0=normal, 1=bright).

## 2. Syntax

```basic
BRIGHT state%
```

## 3. Code Example

```basic
10 REM BRIGHT Demonstration
20 PRINT "BRIGHT executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: Sinclair ZX Spectrum, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Display
- **Subsystem**: SUBSYSTEM_ADVANCED
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BRIGHT |
| Category | Graphics & Display |
| Syntax | BRIGHT state% |
| Description | Controls ZX Spectrum text foreground and background highlight attribute (0=normal, 1=bright). |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ADVANCED |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Sinclair ZX Spectrum, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/gfx.c |
