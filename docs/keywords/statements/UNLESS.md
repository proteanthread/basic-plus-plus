<!--
Title:        UNLESS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/control/branch/unless.c
Generated:    no, hand-written
Status:       current
-->

# `UNLESS` Keyword Reference

## Source Header

```c
// FILENAME: unless.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (eval.h, eval.c, map.h, map.c, string.c, unless.h, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the UNLESS statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Evaluates boolean expression and executes THEN clause if false, ELSE clause if true (DEC / Tymshare).

## 2. Syntax

```basic
UNLESS expr [THEN] stmt/line [ELSE stmt/line]
```

## 3. Code Example

```basic
10 REM UNLESS Demonstration
20 PRINT "UNLESS executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error, Error 8: Undefined line number

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
| Name | UNLESS |
| Category | Control Flow |
| Syntax | UNLESS expr [THEN] stmt/line [ELSE stmt/line] |
| Description | Evaluates boolean expression and executes THEN clause if false, ELSE clause if true (DEC / Tymshare). |
| Error Summary | Error 2: Syntax error, Error 8: Undefined line number |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/control/branch/unless.c |
