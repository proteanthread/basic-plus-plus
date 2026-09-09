<!--
Title:        BIOSTATUS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/network/func_net_telemetry.c
Generated:    no, hand-written
Status:       current
-->

# `BIOSTATUS` Keyword Reference

## Source Header

```c
// FILENAME: func_net_telemetry.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, eval_ident_builtin.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (func_net_telemetry.h)
// Provides runtime implementation and LanguageDescriptors for network and I/O
```

## 1. Description & Usage

Returns block device I/O status bitmask.

## 2. Syntax

```basic
BIOSTATUS
```

## 3. Code Example

```basic
10 REM BIOSTATUS Demonstration
20 PRINT "BIOSTATUS executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Diagnostics
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | BIOSTATUS |
| Category | Diagnostics |
| Syntax | BIOSTATUS |
| Description | Returns block device I/O status bitmask. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/network/func_net_telemetry.c |
