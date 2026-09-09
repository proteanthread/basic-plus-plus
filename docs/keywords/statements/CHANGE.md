<!--
Title:        CHANGE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/change.c
Generated:    no, hand-written
Status:       current
-->

# `CHANGE` Keyword Reference

## Source Header

```c
// FILENAME: change.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (arrays.h, arrays.c, ctype.h, ctype.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (change.h, eval.h, eval.c, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the CHANGE statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Converts between string and a 1D numeric array of ASCII codes (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
CHANGE string_expr TO num_array | CHANGE num_array TO string_var
```

## 3. Code Example

```basic
10 REM CHANGE Demonstration
20 PRINT "CHANGE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error, Error 9: Subscript out of range, Error 13: Type mismatch, Error 5: Illegal function call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | CHANGE |
| Category | Variables & Memory |
| Syntax | CHANGE string_expr TO num_array \| CHANGE num_array TO string_var |
| Description | Converts between string and a 1D numeric array of ASCII codes (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax error, Error 9: Subscript out of range, Error 13: Type mismatch, Error 5: Illegal function call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/change.c |
