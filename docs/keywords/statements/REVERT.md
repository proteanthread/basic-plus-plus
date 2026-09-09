<!--
Title:        REVERT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/program/stmt_revert.c
Generated:    no, hand-written
Status:       current
-->

# `REVERT` Keyword Reference

## Source Header

```c
// FILENAME: stmt_revert.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libengine (vm.h, lexer.h, semantic_harvester.h, vdev.h)
// Provides runtime implementation for the REVERT statement in BASIC++.
```

## 1. Description & Usage

Rolls back previous program operations from in-memory RAMbank snapshot ring stack.

## 2. Syntax

```basic
REVERT [tool_tag$] | REVERT [ tool_tag$ ] | REVERT { tool: "..." }
```

## 3. Code Example

```basic
10 REM REVERT Demonstration
20 PRINT "REVERT executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Mgmt & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | REVERT |
| Category | Program Mgmt & Editing |
| Syntax | REVERT [tool_tag$] \| REVERT [ tool_tag$ ] \| REVERT { tool: "..." } |
| Description | Rolls back previous program operations from in-memory RAMbank snapshot ring stack. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/program/stmt_revert.c |
