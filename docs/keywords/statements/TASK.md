<!--
Title:        TASK
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/task.c
Generated:    no, hand-written
Status:       current
-->

# `TASK` Keyword Reference

## Source Header

```c
// FILENAME: task.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (goodbye.c, resume.c, suspend.c, system.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, stmt.h, string.c, task.
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the TASK statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Manages concurrent background script tasks and thread execution.

## 2. Syntax

```basic
TASK [filename_expr$ | ::label | LIST | WAIT task_id | KILL task_id]
```

## 3. Code Example

```basic
10 REM TASK Demonstration
20 PRINT "TASK executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 70: Permission Denied

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
| Name | TASK |
| Category | Control Flow |
| Syntax | TASK [filename_expr$ \| ::label \| LIST \| WAIT task_id \| KILL task_id] |
| Description | Manages concurrent background script tasks and thread execution. |
| Error Summary | Error 2: Syntax Error, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/task.c |
