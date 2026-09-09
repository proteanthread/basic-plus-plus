<!--
Title:        UNLOAD
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/platform/dl/plat_dl.c
Generated:    no, hand-written
Status:       current
-->

# `UNLOAD` Keyword Reference

## Source Header

```c
// FILENAME: plat_dl.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community -- All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe, bpp.exe, bs.exe, libengine
// NEEDS: libcore, libkernel
// Implements component functionality for UNLOAD.
```

## 1. Description & Usage

Unloads a dynamically linked library or loaded module extension from active memory.

## 2. Syntax

```basic
UNLOAD module_name$
```

## 3. Code Example

```basic
10 REM UNLOAD Demonstration
20 PRINT "UNLOAD executed successfully."
```

## 4. Error Conditions

Error 53: File Not Found

## 5. Compatibility & Lineage

- **Lineage**: VB for DOS, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Dynamic Loading
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | UNLOAD |
| Category | Dynamic Loading |
| Syntax | UNLOAD module_name$ |
| Description | Unloads a dynamically linked library or loaded module extension from active memory. |
| Error Summary | Error 53: File Not Found |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | VB for DOS, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/platform/dl/plat_dl.c |
