<!--
Title:        UMOUNT
Tier:         3
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/vfs.c
Generated:    no, hand-written
Status:       current
-->

# `UMOUNT` Keyword Reference

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

Unmounts the specified virtual drive letter and releases associated filesystem resources.

## 2. Syntax

```basic
UMOUNT drive_letter$
```

## 3. Code Example

```basic
10 REM UMOUNT Demonstration
20 PRINT "UMOUNT executed successfully."
```

## 4. Error Conditions

Error 76: Path Not Found

## 5. Compatibility & Lineage

- **Lineage**: BASIC++ Standard
- **Since Version**: 6.0.0
- **Category**: Virtual Filesystems
- **Subsystem**: SUBSYSTEM_PLATFORM
- **Safety Level**: SAFETY_IO

---

## LanguageDescriptor (LangDesc) Quick Reference

| Field | Value |
|---|---|
| Name | UMOUNT |
| Category | Virtual Filesystems |
| Syntax | UMOUNT drive_letter$ |
| Description | Unmounts the specified virtual drive letter and releases associated filesystem resources. |
| Error Summary | Error 76: Path Not Found |
| Subsystem | SUBSYSTEM_PLATFORM |
| Safety Level | SAFETY_IO |
| Feature Type | FEATURE_STATEMENT |
| Delimiter Mask | none |
| Compatibility | BASIC++ Standard |
| Since Version | 6.0.0 |
| Source File | engine/src/runtime/vfs.c |
