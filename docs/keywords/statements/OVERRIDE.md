<!--
Title:        OVERRIDE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/override.c
Generated:    no, hand-written
Status:       current
-->

# `OVERRIDE` Keyword Reference

## Source Header

```c
// FILENAME: override.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h)
// NEEDS: libcore (language_descriptor.h, memops.h, strops.h)
// NEEDS: libengine (lexer.h, override.h, vm.h)
// NEEDS: libkernel (types.h, vdev.h)
// Provides runtime implementation for the OVERRIDE statement in BASIC++.
```

## 1. Description & Usage

Replaces or hooks built-in statement execution with user-defined SUB or GOSUB routines.

## 2. Syntax

```basic
OVERRIDE target WITH GOSUB line_num | OVERRIDE target WITH sub_name | OVERRIDE CLEAR
```

## 3. Code Example

```basic
10 REM OVERRIDE Demonstration
20 PRINT "OVERRIDE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Permission Denied (Protected Keyword)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_FLEX
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | OVERRIDE |
| Category | Introspection |
| Syntax | OVERRIDE target WITH GOSUB line_num \| OVERRIDE target WITH sub_name \| OVERRIDE CLEAR |
| Description | Replaces or hooks built-in statement execution with user-defined SUB or GOSUB routines. |
| Error Summary | Error 2: Syntax Error, Error 13: Permission Denied (Protected Keyword) |
| Subsystem | SUBSYSTEM_FLEX |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/override.c |
