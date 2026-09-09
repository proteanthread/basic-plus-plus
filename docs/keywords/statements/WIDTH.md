<!--
Title:        WIDTH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/device/gfx_tui.c
Generated:    no, hand-written
Status:       current
-->

# `WIDTH` Keyword Reference

## Source Header

```c
// FILENAME: gfx_tui.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for WIDTH.
```

## 1. Description & Usage

Sets the text output column width for the console screen, active window, or printer channel.

## 2. Syntax

```basic
WIDTH [columns] [, rows] | WIDTH [#file_num,] columns
```

## 3. Code Example

```basic
10 REM WIDTH Demonstration
20 PRINT "WIDTH executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console & Printer I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | WIDTH |
| Category | Console & Printer I/O |
| Syntax | WIDTH [columns] [, rows] \| WIDTH [#file_num,] columns |
| Description | Sets the text output column width for the console screen, active window, or printer channel. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/device/gfx_tui.c |
