<!--
Title:        EDIT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/editor/editor_manager.c
Generated:    no, hand-written
Status:       current
-->

# `EDIT` Keyword Reference

## Source Header

```c
// FILENAME: editor_manager.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libstandard (editor.h, editor.c)
// Implements visual text editor subsystem components for editor_manager.
//
// ---- Includes ----
```

## 1. Description & Usage

Launches the built-in full-screen TUI text editor or positions cursor at specified line.

## 2. Syntax

```basic
EDIT [filename$ | line_number]
```

## 3. Code Example

```basic
10 REM EDIT Demonstration
20 PRINT "EDIT executed successfully."
```

## 4. Error Conditions

Error 53: File Not Found

## 5. Compatibility & Lineage

- **Lineage**: GW-BASIC, QBASIC, BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Program Management & Editing
- **Subsystem**: SUBSYSTEM_ENGINE
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | EDIT |
| Category | Program Management & Editing |
| Syntax | EDIT [filename$ \| line_number] |
| Description | Launches the built-in full-screen TUI text editor or positions cursor at specified line. |
| Error Summary | Error 53: File Not Found |
| Subsystem | SUBSYSTEM_ENGINE |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | GW-BASIC, QBASIC, BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/editor/editor_manager.c |
