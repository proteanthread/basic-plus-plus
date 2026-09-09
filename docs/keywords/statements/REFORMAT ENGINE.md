<!--
Title:        REFORMAT ENGINE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/reformat/reformat_report.c
Generated:    no, hand-written
Status:       current
-->

# `REFORMAT ENGINE` Keyword Reference

## Source Header

```c
// FILENAME: reformat_report.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (reformat_internal.h)
// Provides runtime implementation for the REFORMAT_REPORT statement in BASIC+
//
// ---- Includes ----
```

## 1. Description & Usage

Reformats source code lines according to BASIC++ style and layout rules.

## 2. Syntax

```basic
REFORMAT [line_start[-line_end]]
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

Error 8: Undefined Line Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Code Formatting
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REFORMAT ENGINE |
| Category | Code Formatting |
| Syntax | REFORMAT [line_start[-line_end]] |
| Description | Reformats source code lines according to BASIC++ style and layout rules. |
| Error Summary | Error 8: Undefined Line Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/reformat/reformat_report.c |
