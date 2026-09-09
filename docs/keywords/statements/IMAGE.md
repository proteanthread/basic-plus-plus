<!--
Title:        IMAGE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/image.c
Generated:    no, hand-written
Status:       current
-->

# `IMAGE` Keyword Reference

## Source Header

```c
// FILENAME: image.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (image.h, string.c)
// Provides runtime implementation for the IMAGE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares a line format template referenced by PRINT USING or PRINT IN FORM (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
IMAGE: format_template_specifiers
```

## 3. Code Example

```basic
10 REM IMAGE Demonstration
20 PRINT "IMAGE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

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
| Name | IMAGE |
| Category | Input / Output |
| Syntax | IMAGE: format_template_specifiers |
| Description | Declares a line format template referenced by PRINT USING or PRINT IN FORM (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/image.c |
