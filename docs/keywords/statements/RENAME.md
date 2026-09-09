<!--
Title:        RENAME
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/stmt_rename.c
Generated:    no, hand-written
Status:       current
-->

# `RENAME` Keyword Reference

## Source Header

```c
// FILENAME: stmt_rename.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c, common_reg_funcs.c)
// NEEDS: libcore (language_descriptor.h, memory.h, string.h)
// NEEDS: libengine (lexer.h, stmt_rename.h, vm.h)
// Provides runtime implementation for the RENAME statement and function in BA
//
// ---- Includes ----
```

## 1. Description & Usage

Renames variables and symbols in-program to modernize legacy code and resolve keyword collisions.

## 2. Syntax

```basic
RENAME | RENAME old_var TO new_var | RENAME CONFLICTS [PREFIX "v_"] | RENAME SMART [CASE "mixed"|"snake"] | RENAME UNDO
```

## 3. Code Example

```basic
10 REM RENAME Demonstration
20 PRINT "RENAME executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Management
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | RENAME |
| Category | Program Management |
| Syntax | RENAME \| RENAME old_var TO new_var \| RENAME CONFLICTS [PREFIX "v_"] \| RENAME SMART [CASE "mixed"\|"snake"] \| RENAME UNDO |
| Description | Renames variables and symbols in-program to modernize legacy code and resolve keyword collisions. |
| Error Summary | Error 2: Syntax Error, Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/stmt_rename.c |
