<!--
Title:        SETINDEX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/db/isam/isam.c
Generated:    no, hand-written
Status:       current
-->

# `SETINDEX` Keyword Reference

## Source Header

```c
// FILENAME: isam.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (delete.c, exec_internal.h)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c, struct.h, struct.c)
// NEEDS: libcore (variables.h, variables.c)
// NEEDS: libengine (eval.h, eval.c, isam.h, map.h, map.c, string.c)
// Provides runtime implementation for the ISAM statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage



## 2. Syntax

```basic
SETINDEX #ch, idx$
```

## 3. Code Example

```basic
10 REM SETINDEX Demonstration
20 PRINT "SETINDEX executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Database
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SETINDEX |
| Category | Database |
| Syntax | SETINDEX #ch, idx$ |
| Description | none |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/db/isam/isam.c |
