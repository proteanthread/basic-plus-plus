<!--
Title:        PREFIX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/filesystem/binary_ops/prefix.c
Generated:    no, hand-written
Status:       current
-->

# `PREFIX` Keyword Reference

## Source Header

```c
// FILENAME: prefix.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libengine (chdir.h, chdir.c, prefix.h, string.c)
// Provides runtime implementation for the PREFIX statement in BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Apple /// Business BASIC alias for CHDIR. Sets the default SOS directory prefix.

## 2. Syntax

```basic
PREFIX dir_path$
```

## 3. Code Example

```basic
10 REM PREFIX Demonstration
20 PRINT "PREFIX executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error, Error 13: Type Mismatch, Error 76: Path Not Found

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Filesystem
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PREFIX |
| Category | Filesystem |
| Syntax | PREFIX dir_path$ |
| Description | Apple /// Business BASIC alias for CHDIR. Sets the default SOS directory prefix. |
| Error Summary | Error 2: Syntax Error, Error 13: Type Mismatch, Error 76: Path Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/filesystem/binary_ops/prefix.c |
