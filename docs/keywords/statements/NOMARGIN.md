<!--
Title:        NOMARGIN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/io/page.c
Generated:    no, hand-written
Status:       current
-->

# `NOMARGIN` Keyword Reference

## Source Header

```c
// FILENAME: page.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, page.h, string.c)
// NEEDS: libkernel (errors.h, vdev.h, vdev.c)
// Provides runtime implementation for the PAGE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Disables automatic line width wrapping limit (DEC / ECMA-116).

## 2. Syntax

```basic
NOMARGIN [#channel]
```

## 3. Code Example

```basic
10 REM NOMARGIN Demonstration
20 PRINT "NOMARGIN executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error, Error 52: Bad file number

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
| Name | NOMARGIN |
| Category | Input / Output |
| Syntax | NOMARGIN [#channel] |
| Description | Disables automatic line width wrapping limit (DEC / ECMA-116). |
| Error Summary | Error 2: Syntax error, Error 52: Bad file number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/io/page.c |
