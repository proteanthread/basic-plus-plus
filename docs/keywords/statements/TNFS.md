<!--
Title:        TNFS
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/network/stmt_tnfs.c
Generated:    no, hand-written
Status:       current
-->

# `TNFS` Keyword Reference

## Source Header

```c
// FILENAME: stmt_tnfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (string.h, strings.h, strings.c, tnfs.h, tnfs.c)
// NEEDS: libengine (eval.h, eval.c, stmt_tnfs.h, string.c)
// Implements TNFS.MOUNT and TNFS.UNMOUNT statement handlers.
//
// ---- Includes ----
```

## 1. Description & Usage

Mounts or unmounts a remote Trusted Network File System (TNFS) server repository.

## 2. Syntax

```basic
TNFS.MOUNT host$ [, path$ [, port]] | TNFS.UNMOUNT
```

## 3. Code Example

```basic
10 REM TNFS Demonstration
20 PRINT "TNFS executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Hardware & Network
- **Subsystem**: SUBSYSTEM_HARDWARE
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | TNFS |
| Category | Hardware & Network |
| Syntax | TNFS.MOUNT host$ [, path$ [, port]] \| TNFS.UNMOUNT |
| Description | Mounts or unmounts a remote Trusted Network File System (TNFS) server repository. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_HARDWARE |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/network/stmt_tnfs.c |
