<!--
Title:        VPATH
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/vfs.c
Generated:    no, hand-written
Status:       current
-->

# `VPATH` Keyword Reference

## Source Header

```c
// FILENAME: vfs.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community  --  All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (error.c, spec.c)
// NEEDED BY: libengine (context.c, control.c, data.c, events_internal.h)
// NEEDED BY: libengine (exec_internal.h, vm_internal.h)
// NEEDS: libcore (hal.h, memops.h, memops.c, runtime_snprintf.h, runtime_snpr
// NEEDS: libcore (strops.h, strops.c, vfs.h)
// NEEDS: libkernel (vdev.h, vdev.c)
// NEEDS: libplatform (platform.h)
// Provides core logic and interface definitions for vfs within BASIC++.
//
// ---- Includes ----
```

## 1. Description & Usage

Configures or displays the active search path list used for file resolution and includes.

## 2. Syntax

```basic
VPATH [search_path$]
```

## 3. Code Example

```basic
10 REM VPATH Demonstration
20 PRINT "VPATH executed successfully."
```

## 4. Error Conditions

None

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Virtual Filesystems
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_SAFE

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | VPATH |
| Category | Virtual Filesystems |
| Syntax | VPATH [search_path$] |
| Description | Configures or displays the active search path list used for file resolution and includes. |
| Error Summary | None |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_SAFE |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/vfs.c |
