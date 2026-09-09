<!--
Title:        BDOS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/system/hardware/func_vintage_vars.c
Generated:    no, hand-written
Status:       current
-->

# `BDOS` Keyword Reference

## Source Header

```c
// FILENAME: func_vintage_vars.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_vintage_vars.h)
// Provides runtime implementation and LanguageDescriptors for vintage dialect
```

## 1. Description & Usage

Returns the CP/M BDOS system call vector address (5).

## 2. Syntax

```basic
BDOS
```

## 3. Code Example

```basic
10 REM BDOS Demonstration
20 PRINT "BDOS executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Peripherals
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BDOS |
| Category | Hardware & Peripherals |
| Syntax | BDOS |
| Description | Returns the CP/M BDOS system call vector address (5). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/system/hardware/func_vintage_vars.c |
