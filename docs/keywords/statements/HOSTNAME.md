<!--
Title:        HOSTNAME
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/introspection.c
Generated:    no, hand-written
Status:       current
-->

# `HOSTNAME` Keyword Reference

## Source Header

```c
// FILENAME: introspection.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (language_descriptor.h, memops.h)
// NEEDS: libengine (introspection.h, stmt.h)
// NEEDS: libkernel (security.h, vdev.h)
// NEEDS: libplatform (platform.h)
// Provides runtime implementation for system diagnostic statements in BASIC++
```

## 1. Description & Usage

Outputs the system hostname to console.

## 2. Syntax

```basic
HOSTNAME
```

## 3. Code Example

```basic
10 REM HOSTNAME Demonstration
20 PRINT "HOSTNAME executed successfully."
```

## 4. Error Conditions

Error 70: Permission Denied

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SYSTEM

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | HOSTNAME |
| Category | Introspection |
| Syntax | HOSTNAME |
| Description | Outputs the system hostname to console. |
| Error Summary | Error 70: Permission Denied |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SYSTEM |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/introspection.c |
