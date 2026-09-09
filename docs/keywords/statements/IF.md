<!--
Title:        IF
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/if.c
Generated:    no, hand-written
Status:       current
-->

# `IF` Keyword Reference

## Source Header

```c
// FILENAME: if.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, if.h, string.c, vm.h)
// Provides runtime implementation for the IF statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Evaluates boolean expression expr and executes THEN clause if true, ELSE clause if false.

## 2. Syntax

```basic
IF expr THEN stmt/line [ELSE stmt/line]
```

## 3. Code Example

```basic
10 REM IF Demonstration
20 PRINT "IF executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error (missing THEN or malformed expression)

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | IF |
| Category | Control Flow |
| Syntax | IF expr THEN stmt/line [ELSE stmt/line] |
| Description | Evaluates boolean expression expr and executes THEN clause if true, ELSE clause if false. |
| Error Summary | Error 2: Syntax error (missing THEN or malformed expression) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/if.c |
