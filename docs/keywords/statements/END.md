<!--
Title:        END
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/core/program/end.c
Generated:    no, hand-written
Status:       current
-->

# `END` Keyword Reference

## Source Header

```c
// FILENAME: end.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (end.h, string.c, vm.h)
// Provides runtime implementation for the END statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Terminates program execution or closes structured block definitions.

## 2. Syntax

```basic
END [IF | SUB | FUNCTION | SELECT | STRUCT]
```

## 3. Code Example

```basic
10 REM END Demonstration
20 PRINT "END executed successfully."
```

## 4. Error Conditions

Error 2: Syntax error (mismatched block terminator)

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
| Name | END |
| Category | Control Flow |
| Syntax | END [IF \| SUB \| FUNCTION \| SELECT \| STRUCT] |
| Description | Terminates program execution or closes structured block definitions. |
| Error Summary | Error 2: Syntax error (mismatched block terminator) |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/core/program/end.c |
