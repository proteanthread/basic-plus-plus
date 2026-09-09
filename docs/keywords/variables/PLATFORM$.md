<!--
Title:        PLATFORM$
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/descriptors/system_var_descriptors.c
Generated:    no, hand-written
Status:       current
-->

# `PLATFORM$` Keyword Reference

## Source Header

```c
// FILENAME: system_var_descriptors.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libflex
// NEEDS: libcore (language_descriptor.h, system_var_descriptors.h)
// Implements authoritative descriptor definitions for built-in system variabl
```

## 1. Description & Usage

Platform execution environment (Win32, POSIX, Embedded, FreeDOS).

## 2. Syntax

```basic
PLATFORM$ or SYS.PLATFORM$
```

## 3. Code Example

```basic
10 FOR I = 1 TO 5
20   PRINT "Iteration: "; I
30 NEXT I
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: System Identification
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | PLATFORM$ |
| Category | System Identification |
| Syntax | PLATFORM$ or SYS.PLATFORM$ |
| Description | Platform execution environment (Win32, POSIX, Embedded, FreeDOS). |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_VARIABLE |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/runtime/descriptors/system_var_descriptors.c |
