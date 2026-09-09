<!--
Title:        FLASH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/gfx.c
Generated:    no, hand-written
Status:       current
-->

# `FLASH` Keyword Reference

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

Controls blinking text attribute on vintage video displays (0=off, 1=on).

## 2. Syntax

```basic
FLASH state%
```

## 3. Code Example

```basic
10 REM FLASH Demonstration
20 PRINT "FLASH executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: Sinclair ZX Spectrum, Apple II, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Display
- **Subsystem**: SUBSYSTEM_ADVANCED
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | FLASH |
| Category | Graphics & Display |
| Syntax | FLASH state% |
| Description | Controls blinking text attribute on vintage video displays (0=off, 1=on). |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ADVANCED |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Sinclair ZX Spectrum, Apple II, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/gfx.c |
