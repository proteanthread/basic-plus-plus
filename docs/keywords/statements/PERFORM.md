<!--
Title:        PERFORM
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/system/environment/perform.c
Generated:    no, hand-written
Status:       current
-->

# `PERFORM` Keyword Reference

## Source Header

```c
// FILENAME: perform.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (call.h, call.c, perform.h, string.c)
// Provides runtime implementation for the PERFORM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Apple /// Business BASIC alias for CALL. Invokes a subprogram or driver routine with arguments.

## 2. Syntax

```basic
PERFORM routine_name [(arg1, arg2, ...)]
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

Error 2: Syntax Error, Error 35: Subprogram Not Defined

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
| Name | PERFORM |
| Category | Control Flow |
| Syntax | PERFORM routine_name [(arg1, arg2, ...)] |
| Description | Apple /// Business BASIC alias for CALL. Invokes a subprogram or driver routine with arguments. |
| Error Summary | Error 2: Syntax Error, Error 35: Subprogram Not Defined |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/system/environment/perform.c |
