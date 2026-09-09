<!--
Title:        STATIC
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/variable/var.c
Generated:    no, hand-written
Status:       current
-->

# `STATIC` Keyword Reference

## Source Header

```c
// FILENAME: var.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for STATIC.
```

## 1. Description & Usage

Declares procedure-local variables whose storage and values persist across successive invocations.

## 2. Syntax

```basic
STATIC var1 [AS type] [, var2 ...]
```

## 3. Code Example

```basic
10 REM STATIC Demonstration
20 PRINT "STATIC executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: QBASIC, VB for DOS, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Variables & Memory
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | STATIC |
| Category | Variables & Memory |
| Syntax | STATIC var1 [AS type] [, var2 ...] |
| Description | Declares procedure-local variables whose storage and values persist across successive invocations. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | QBASIC, VB for DOS, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/variable/var.c |
