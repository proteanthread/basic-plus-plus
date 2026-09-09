<!--
Title:        IF_POSTFIX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/ast/eval_ast.c
Generated:    no, hand-written
Status:       current
-->

# `IF_POSTFIX` Keyword Reference

## Source Header

```c
// FILENAME: eval_ast.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for IF_POSTFIX.
```

## 1. Description & Usage

Postfix conditional modifier executing the preceding statement only when condition is true.

## 2. Syntax

```basic
statement IF condition [ELSE statement]
```

## 3. Code Example

```basic
10 REM IF_POSTFIX Demonstration
20 PRINT "IF_POSTFIX executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: Perl, Ruby, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | IF_POSTFIX |
| Category | Control Flow |
| Syntax | statement IF condition [ELSE statement] |
| Description | Postfix conditional modifier executing the preceding statement only when condition is true. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Perl, Ruby, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/ast/eval_ast.c |
