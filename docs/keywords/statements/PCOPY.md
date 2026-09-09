<!--
Title:        PCOPY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/gfx.c
Generated:    no, hand-written
Status:       current
-->

# `PCOPY` Keyword Reference

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

Copies an entire video display buffer page from source_page to dest_page.

## 2. Syntax

```basic
PCOPY source_page, dest_page
```

## 3. Code Example

```basic
10 REM PCOPY Demonstration
20 PRINT "PCOPY executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: QuickBASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Graphics & Display
- **Subsystem**: SUBSYSTEM_ADVANCED
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PCOPY |
| Category | Graphics & Display |
| Syntax | PCOPY source_page, dest_page |
| Description | Copies an entire video display buffer page from source_page to dest_page. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ADVANCED |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | QuickBASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/gfx.c |
