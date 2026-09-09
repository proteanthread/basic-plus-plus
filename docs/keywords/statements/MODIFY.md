<!--
Title:        MODIFY
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/modify.c
Generated:    no, hand-written
Status:       current
-->

# `MODIFY` Keyword Reference

## Source Header

```c
// FILENAME: modify.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (file.h, file.c, language_descriptor.h)
// NEEDS: libcore (string.h)
// NEEDS: libengine (eval.h, eval.c, modify.h, string.c)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the MODIFY statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Opens a file channel in read/write random access modify mode (SDS 940 / DEC PDP-10 Super BASIC).

## 2. Syntax

```basic
MODIFY #channel, "field_spec"
```

## 3. Code Example

```basic
10 REM MODIFY Demonstration
20 PRINT "MODIFY executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 52: Bad File Number, Error 55: File Already Open, Error 70: Permission Denied

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
| Name | MODIFY |
| Category | Filesystem I/O |
| Syntax | MODIFY #channel, "field_spec" |
| Description | Opens a file channel in read/write random access modify mode (SDS 940 / DEC PDP-10 Super BASIC). |
| Error Summary | Error 2: Syntax Error, Error 52: Bad File Number, Error 55: File Already Open, Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/modify.c |
