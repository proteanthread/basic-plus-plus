<!--
Title:        OVER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/gfx.c
Generated:    no, hand-written
Status:       current
-->

# `OVER` Keyword Reference

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

Controls overstrike transparent printing mode (0=overwrite, 1=XOR/overstrike).

## 2. Syntax

```basic
OVER state%
```

## 3. Code Example

```basic
10 REM OVER Demonstration
20 PRINT "OVER executed successfully."
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
| Name | OVER |
| Category | Graphics & Display |
| Syntax | OVER state% |
| Description | Controls overstrike transparent printing mode (0=overwrite, 1=XOR/overstrike). |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ADVANCED |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Sinclair ZX Spectrum, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/gfx.c |
