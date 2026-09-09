<!--
Title:        FORM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/form.c
Generated:    no, hand-written
Status:       current
-->

# `FORM` Keyword Reference

## Source Header

```c
// FILENAME: form.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (form.h, string.c)
// Provides runtime implementation for the FORM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Declares a structured data format specification (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
FORM format_specifier_list
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
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
| Name | FORM |
| Category | Input / Output |
| Syntax | FORM format_specifier_list |
| Description | Declares a structured data format specification (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/form.c |
