<!--
Title:        LINPUT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/linput.c
Generated:    no, hand-written
Status:       current
-->

# `LINPUT` Keyword Reference

## Source Header

```c
// FILENAME: linput.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, linput.h, string.c)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the LINPUT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads an entire line of text into a string variable without delimiters (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
LINPUT [;] ["prompt";] string_var$
```

## 3. Code Example

```basic
10 REM LINPUT Demonstration
20 PRINT "LINPUT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End

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
| Name | LINPUT |
| Category | Input / Output |
| Syntax | LINPUT [;] ["prompt";] string_var$ |
| Description | Reads an entire line of text into a string variable without delimiters (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/linput.c |
