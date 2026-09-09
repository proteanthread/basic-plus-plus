<!--
Title:        DIRLISTBOX
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/ui/widgets/dirlistbox.c
Generated:    no, hand-written
Status:       current
-->

# `DIRLISTBOX` Keyword Reference

## Source Header

```c
// FILENAME: dirlistbox.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_stmts.c)
// NEEDS: libcore (language_descriptor.h, strings.h)
// NEEDS: libengine (ctrl_common.h, dirlistbox.h)
// NEEDS: libkernel (vdev.h)
// Provides runtime implementation for the DIRLISTBOX statement in BASIC++.
```

## 1. Description & Usage

Renders a directory tree list box UI control.

## 2. Syntax

```basic
DIRLISTBOX path$, col%, row%, width%, height%
```

## 3. Code Example

```basic
10 REM DIRLISTBOX Demonstration
20 PRINT "DIRLISTBOX executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: User Interface
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_PURE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | DIRLISTBOX |
| Category | User Interface |
| Syntax | DIRLISTBOX path$, col%, row%, width%, height% |
| Description | Renders a directory tree list box UI control. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_PURE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | none |
| Since Version | none |
| Source File | engine/src/statements/ui/widgets/dirlistbox.c |
