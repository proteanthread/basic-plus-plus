<!--
Title:        SET
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/keyword_props.c
Generated:    no, hand-written
Status:       current
-->

# `SET` Keyword Reference

## Source Header

```c
// FILENAME: keyword_props.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_internal.h, keyword.c)
// NEEDS: libcore (keyword_props.h, memops.h, memops.c, strops.h, strops.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h, vdev.h, vdev.c)
// Provides core logic and interface definitions for keyword_props within BASI
//
// ---- Includes ----
```

## 1. Description & Usage

Configures engine tunables, dialect switches, or environment configuration variables.

## 2. Syntax

```basic
SET option$ = value
```

## 3. Code Example

```basic
10 REM SET Demonstration
20 PRINT "SET executed successfully."
```

## 4. Error Conditions

Error 5: Illegal Function Call

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System & Environment
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SET |
| Category | System & Environment |
| Syntax | SET option$ = value |
| Description | Configures engine tunables, dialect switches, or environment configuration variables. |
| Error Summary | Error 5: Illegal Function Call |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/keyword_props.c |
