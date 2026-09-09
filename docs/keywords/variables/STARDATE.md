<!--
Title:        STARDATE
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/introspection.c
Generated:    no, hand-written
Status:       current
-->

# `STARDATE` Keyword Reference

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

Outputs CP/M days with tenth-day decimal to console.

## 2. Syntax

```basic
STARDATE
```

## 3. Code Example

```basic
10 REM STARDATE Demonstration
20 PRINT "STARDATE executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Introspection
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | STARDATE |
| Category | Introspection |
| Syntax | STARDATE |
| Description | Outputs CP/M days with tenth-day decimal to console. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/introspection/introspection.c |
