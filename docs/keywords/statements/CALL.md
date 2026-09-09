<!--
Title:        CALL
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/oop/call.c
Generated:    no, hand-written
Status:       current
-->

# `CALL` Keyword Reference

## Source Header

```c
// FILENAME: call.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (perform.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (call.h, eval.h, eval.c, interrupt.h, interrupt.c)
// NEEDS: libengine (lexer.h, lexer.c, map.h, map.c, string.c, sub.h, sub.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (security.h, security.c, vdev.h, vdev.c)
// Provides runtime implementation for the CALL statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Transfers control to a SUB procedure or C extension routine.

## 2. Syntax

```basic
CALL name [(argument_list)]
```

## 3. Code Example

```basic
10 Val = CALL name [(argument_list)]
20 PRINT "Result: "; Val
```

## 4. Error Conditions

Error 2: Syntax Error, Error 35: Subprogram Not Defined, Error 36: Parameter Mismatch

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
| Name | CALL |
| Category | Control Flow |
| Syntax | CALL name [(argument_list)] |
| Description | Transfers control to a SUB procedure or C extension routine. |
| Error Summary | Error 2: Syntax Error, Error 35: Subprogram Not Defined, Error 36: Parameter Mismatch |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/oop/call.c |
