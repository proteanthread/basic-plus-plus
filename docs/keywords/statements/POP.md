<!--
Title:        POP
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_udx.c
Generated:    no, hand-written
Status:       current
-->

# `POP` Keyword Reference

## Source Header

```c
// FILENAME: stmt_udx.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: libreg (reg_buffer.h, reg_udx.h, reg_hw.h), eval/eval.h, vm/vm.h
// Implements runtime statements for Universal Data Exchange (UDX, XCHG, PUSH,
```

## 1. Description & Usage

Pops a value from a designated UDX buffer or channel into a target variable.

## 2. Syntax

```basic
POP [FIFO|LIFO|LILO|FILO] [, channel$] var
```

## 3. Code Example

```basic
10 REM POP Demonstration
20 PRINT "POP executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Hardware
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | POP |
| Category | System & Hardware |
| Syntax | POP [FIFO\|LIFO\|LILO\|FILO] [, channel$] var |
| Description | Pops a value from a designated UDX buffer or channel into a target variable. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_udx.c |
