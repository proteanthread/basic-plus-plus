<!--
Title:        INPUT$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/functions/io/func_input_str.c
Generated:    no, hand-written
Status:       current
-->

# `INPUT$` Keyword Reference

## Source Header

```c
// FILENAME: func_input_str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, sys_fn.c)
// NEEDS: libcore, libengine, libkernel
// Provides runtime implementation for INPUT$ function in BASIC++.
```

## 1. Description & Usage

Reads a fixed number of bytes from console input or open file channel.

## 2. Syntax

```basic
INPUT$(n% [, [#]channel%])
```

## 3. Code Example

```basic
10 Val = INPUT$(n% [, [#]channel%])
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 52: Bad File Number

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: File & Console I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | INPUT$ |
| Category | File & Console I/O |
| Syntax | INPUT$(n% [, [#]channel%]) |
| Description | Reads a fixed number of bytes from console input or open file channel. |
| Error Summary | Error 5: Illegal Function Call, Error 52: Bad File Number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/eval/functions/io/func_input_str.c |
