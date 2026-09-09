<!--
Title:        MID$ STMT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/variables/assignment/mid_stmt.c
Generated:    no, hand-written
Status:       current
-->

# `MID$ STMT` Keyword Reference

## Source Header

```c
// FILENAME: mid_stmt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (eval.h, eval.c, lexer.h, lexer.c, mid_stmt.h, string.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the MID_STMT statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Replaces characters inside a string variable starting at specified position.

## 2. Syntax

```basic
MID$(string_var, start [, length]) = replacement$
```

## 3. Code Example

```basic
10 Val = MID$(string_var, start [, length]) = replacement$
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 5: Illegal Function Call, Error 13: Type Mismatch

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: String Manipulation
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | MID$ STMT |
| Category | String Manipulation |
| Syntax | MID$(string_var, start [, length]) = replacement$ |
| Description | Replaces characters inside a string variable starting at specified position. |
| Error Summary | Error 5: Illegal Function Call, Error 13: Type Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/variables/assignment/mid_stmt.c |
