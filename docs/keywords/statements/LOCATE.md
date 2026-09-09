<!--
Title:        LOCATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/gfx_tui.c
Generated:    no, hand-written
Status:       current
-->

# `LOCATE` Keyword Reference

## Source Header

```c
// FILENAME: gfx_tui.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for LOCATE.
```

## 1. Description & Usage

Positions the console text cursor at row and column (1-indexed), and configures cursor visibility.

## 2. Syntax

```basic
LOCATE [row] [, [col] [, [cursor] [, [start] [, stop]]]]
```

## 3. Code Example

```basic
10 REM LOCATE Demonstration
20 PRINT "LOCATE executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LOCATE |
| Category | Console I/O |
| Syntax | LOCATE [row] [, [col] [, [cursor] [, [start] [, stop]]]] |
| Description | Positions the console text cursor at row and column (1-indexed), and configures cursor visibility. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/gfx_tui.c |
