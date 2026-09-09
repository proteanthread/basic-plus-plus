<!--
Title:        DISPLAY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/display.c
Generated:    no, hand-written
Status:       current
-->

# `DISPLAY` Keyword Reference

## Source Header

```c
// FILENAME: display.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for DISPLAY.
```

## 1. Description & Usage

Outputs formatted expression list to console screen buffer adhering to ECMA-116 standard.

## 2. Syntax

```basic
DISPLAY [exprlist]
```

## 3. Code Example

```basic
10 REM DISPLAY Demonstration
20 PRINT "DISPLAY executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: ECMA-116, Super BASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DISPLAY |
| Category | Console I/O |
| Syntax | DISPLAY [exprlist] |
| Description | Outputs formatted expression list to console screen buffer adhering to ECMA-116 standard. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | ECMA-116, Super BASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/statements/io/display.c |
