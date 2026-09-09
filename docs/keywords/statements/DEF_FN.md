<!--
Title:        DEF_FN
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/eval/ast/eval_ast.c
Generated:    no, hand-written
Status:       current
-->

# `DEF_FN` Keyword Reference

## Source Header

```c
// FILENAME: eval_ast.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for DEF_FN.
```

## 1. Description & Usage

Defines a single-line user function formula adhering to classical Dartmouth / GW-BASIC DEF FN conventions.

## 2. Syntax

```basic
DEF FNname[(args)] = expr
```

## 3. Code Example

```basic
10 Val = DEF FNname[(args)] = e10pr
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: Dartmouth BASIC, GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Function Definitions
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DEF_FN |
| Category | Function Definitions |
| Syntax | DEF FNname[(args)] = expr |
| Description | Defines a single-line user function formula adhering to classical Dartmouth / GW-BASIC DEF FN conventions. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Dartmouth BASIC, GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/eval/ast/eval_ast.c |
