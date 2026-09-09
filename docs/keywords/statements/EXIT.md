<!--
Title:        EXIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/loops/conditional/exit_loop.c
Generated:    no, hand-written
Status:       current
-->

# `EXIT` Keyword Reference

## Source Header

```c
// FILENAME: exit_loop.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, exit_loop.h, lexer.h, lexer.c, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the EXIT_LOOP statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Prematurely exits an active loop, SUB, or FUNCTION block.

## 2. Syntax

```basic
EXIT {FOR|DO|WHILE|SUB|FUNCTION}
```

## 3. Code Example

```basic
10 REM EXIT Demonstration
20 PRINT "EXIT executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 33: Invalid EXIT Scope

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Looping / Control Flow
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EXIT |
| Category | Looping / Control Flow |
| Syntax | EXIT {FOR\|DO\|WHILE\|SUB\|FUNCTION} |
| Description | Prematurely exits an active loop, SUB, or FUNCTION block. |
| Error Summary | Error 2: Syntax Error, Error 33: Invalid EXIT Scope |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/loops/conditional/exit_loop.c |
