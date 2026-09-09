<!--
Title:        LINE INPUT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/io/line_input.c
Generated:    no, hand-written
Status:       current
-->

# `LINE INPUT` Keyword Reference

## Source Header

```c
// FILENAME: line_input.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, file.h, file.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, line_input.h, string.c)
// NEEDS: libkernel (vdev.h, vdev.c)
// Provides runtime implementation for the LINE_INPUT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Reads an entire line of up to 255 characters from the keyboard or a file into a string variable.

## 2. Syntax

```basic
LINE INPUT [;] ["prompt";] string_var$
```

## 3. Code Example

```basic
10 REM LINE INPUT Demonstration
20 PRINT "LINE INPUT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Failed to read input, Error 52: Bad File Number, Error 62: Input Past End

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Console & File I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | LINE INPUT |
| Category | Console & File I/O |
| Syntax | LINE INPUT [;] ["prompt";] string_var$ |
| Description | Reads an entire line of up to 255 characters from the keyboard or a file into a string variable. |
| Error Summary | Error 2: Syntax Error, Error 5: Failed to read input, Error 52: Bad File Number, Error 62: Input Past End |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/io/line_input.c |
