<!--
Title:        MARGIN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/margin.c
Generated:    no, hand-written
Status:       current
-->

# `MARGIN` Keyword Reference

## Source Header

```c
// FILENAME: margin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, margin.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MARGIN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Sets line length / print margin width before auto-wrapping output (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
MARGIN [#channel,] width
```

## 3. Code Example

```basic
10 REM MARGIN Demonstration
20 PRINT "MARGIN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error, Error 5: Illegal function call, Error 52: Bad file number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Input / Output
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MARGIN |
| Category | Input / Output |
| Syntax | MARGIN [#channel,] width |
| Description | Sets line length / print margin width before auto-wrapping output (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax error, Error 5: Illegal function call, Error 52: Bad file number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/margin.c |
