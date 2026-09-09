<!--
Title:        PYTHON
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/stmt_python.c
Generated:    no, hand-written
Status:       current
-->

# `PYTHON` Keyword Reference

## Source Header

```c
// FILENAME: stmt_python.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, string.c, vm.h)
// Implements the PYTHON statement for embedded Python/MicroPython script exec
//
// ---- Includes ----
```

## 1. Description & Usage

Executes inline Python / MicroPython code string.

## 2. Syntax

```basic
PYTHON code$
```

## 3. Code Example

```basic
10 REM PYTHON Demonstration
20 PRINT "PYTHON executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Language Interop
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PYTHON |
| Category | Language Interop |
| Syntax | PYTHON code$ |
| Description | Executes inline Python / MicroPython code string. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/stmt_python.c |
