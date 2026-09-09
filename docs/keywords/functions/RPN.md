<!--
Title:        RPN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/functions/eval/func_rpn.c
Generated:    no, hand-written
Status:       current
-->

# `RPN` Keyword Reference

## Source Header

```c
// FILENAME: func_rpn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, common_reg_funcs.c
// NEEDS: eval/rpn.h, runtime/funcreg.h, runtime/language_descriptor.h
// Implements the freestanding RPN / STACK postfix calculator function in BASI
```

## 1. Description & Usage

Evaluates reverse Polish notation expression strings and manipulates HP operational stack levels.

## 2. Syntax

```basic
RPN(expr$) | STACK(expr$) | RPN(op$, ...)
```

## 3. Code Example

```basic
10 Val = RPN(e10pr$) | STACK(e10pr$) | RPN(op$, ...)
20 PRINT "Result: "; Val
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
| Name | RPN |
| Category | Math & Calculations |
| Syntax | RPN(expr$) \| STACK(expr$) \| RPN(op$, ...) |
| Description | Evaluates reverse Polish notation expression strings and manipulates HP operational stack levels. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_FUNCTION |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/functions/eval/func_rpn.c |
