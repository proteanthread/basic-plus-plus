<!--
Title:        BIOSIZE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/network/func_net_telemetry.c
Generated:    no, hand-written
Status:       current
-->

# `BIOSIZE` Keyword Reference

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

Returns block device total size in bytes.

## 2. Syntax

```basic
BIOSIZE
```

## 3. Code Example

```basic
10 REM BIOSIZE Demonstration
20 PRINT "BIOSIZE executed successfully."
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
| Name | BIOSIZE |
| Category | Diagnostics |
| Syntax | BIOSIZE |
| Description | Returns block device total size in bytes. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/network/func_net_telemetry.c |
