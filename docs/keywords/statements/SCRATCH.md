<!--
Title:        SCRATCH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/vm/control/vm_control.c
Generated:    no, hand-written
Status:       current
-->

# `SCRATCH` Keyword Reference

## Source Header

```c
// FILENAME: vm_control.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for SCRATCH.
```

## 1. Description & Usage

Erases all program lines, resets symbol tables, and frees allocated variable heap memory.

## 2. Syntax

```basic
SCRATCH
```

## 3. Code Example

```basic
10 REM SCRATCH Demonstration
20 PRINT "SCRATCH executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: Dartmouth BASIC, HP BASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Management
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | SCRATCH |
| Category | Program Management |
| Syntax | SCRATCH |
| Description | Erases all program lines, resets symbol tables, and frees allocated variable heap memory. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | Dartmouth BASIC, HP BASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/vm/control/vm_control.c |
