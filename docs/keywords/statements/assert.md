<!--
Title:        assert
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/ast/eval_ast.c
Generated:    no, hand-written
Status:       current
-->

# `ASSERT` Keyword Reference

## Source Header

```c
// FILENAME: eval_ast.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for assert.
```

## 1. Description & Usage

Evaluates boolean condition; raises Error 255 (Assertion Failure) and halts if condition evaluates to false.

## 2. Syntax

```basic
ASSERT condition [, message$]
```

## 3. Code Example

```basic
10 REM ASSERT Demonstration
20 PRINT "ASSERT executed successfully."
```

## 4. Error Conditions

Error 255: Assertion Failure

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Control & Debugging
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ASSERT |
| Category | Program Control & Debugging |
| Syntax | ASSERT condition [, message$] |
| Description | Evaluates boolean condition; raises Error 255 (Assertion Failure) and halts if condition evaluates to false. |
| Error Summary | Error 255: Assertion Failure |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/ast/eval_ast.c |
