<!--
Title:        ADVANCE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/assign.c
Generated:    no, hand-written
Status:       current
-->

# `ADVANCE` Keyword Reference

## Source Header

```c
// FILENAME: assign.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h, strings.h, strings.c, variables.h, variables.c)
// NEEDS: libengine (assign.h, eval.h, eval.c, string.c, vm.h)
// NEEDS: libkernel (errors.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for the ASSIGN statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Advances or rewinds relative record position within an open channel (HP TSB).

## 2. Syntax

```basic
ASSIGN #channel TO "filespec$" [, BUFFER n]
```

## 3. Code Example

```basic
10 REM ADVANCE Demonstration
20 PRINT "ADVANCE executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem I/O
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | ADVANCE |
| Category | Filesystem I/O |
| Syntax | ASSIGN #channel TO "filespec$" [, BUFFER n] |
| Description | Advances or rewinds relative record position within an open channel (HP TSB). |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 62: Input Past End |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/assign.c |
