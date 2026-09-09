<!--
Title:        STACK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_stack.c
Generated:    no, hand-written
Status:       current
-->

# `STACK` Keyword Reference

## Source Header

```c
// FILENAME: stmt_stack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: eval/rpn.h, eval/pn.h, runtime/variables.h, vm/vm.h
// Implements runtime statements for HP-based STACK operations (STACK POP, STA
```

## 1. Description & Usage

Controls and exports HP operational stack levels and storage registers to BASIC++ program variables.

## 2. Syntax

```basic
STACK POP var1 [, var2, ...] | STACK EXPORT var1 [, var2] | STACK DUMP | STACK CLEAR | STACK SWAP
```

## 3. Code Example

```basic
10 REM STACK Demonstration
20 PRINT "STACK executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Math & Calculations
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | STACK |
| Category | Math & Calculations |
| Syntax | STACK POP var1 [, var2, ...] \| STACK EXPORT var1 [, var2] \| STACK DUMP \| STACK CLEAR \| STACK SWAP |
| Description | Controls and exports HP operational stack levels and storage registers to BASIC++ program variables. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_stack.c |
