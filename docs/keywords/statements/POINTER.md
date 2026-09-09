<!--
Title:        POINTER
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/variable/var.c
Generated:    no, hand-written
Status:       current
-->

# `POINTER` Keyword Reference

## Source Header

```c
// FILENAME: var.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for POINTER.
```

## 1. Description & Usage

Declares a typed memory pointer variable for systems and bare-metal programming.

## 2. Syntax

```basic
POINTER ptr_name AS type
```

## 3. Code Example

```basic
10 REM POINTER Demonstration
20 PRINT "POINTER executed successfully."
```

## 4. Error Conditions

Error 2: Syntax Error

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Systems Standard
- **Since Version**: 6.5.0
- **Category**: Hardware & Systems
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_UNSAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | POINTER |
| Category | Hardware & Systems |
| Syntax | POINTER ptr_name AS type |
| Description | Declares a typed memory pointer variable for systems and bare-metal programming. |
| Error Summary | Error 2: Syntax Error |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_UNSAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Systems Standard |
| Since Version | 6.5.0 |
| Source File | engine/src/variable/var.c |
