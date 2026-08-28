# `version` Version Identification & Release Metadata (`libkernel`)

## 1. Architectural Purpose & Overview

The `version` subsystem (`engine/src/types/version.c`) provides version stamping and release metadata for BASIC++ v6.5.2.

### Version Constants:
- **Major / Minor / Patch**: `6.5.2`
- **Dialect Target**: Unified Master Dialect (GW-BASIC, QBASIC, Super BASIC, ECMA-116 compatible).
- **Banner Messages**:
  - `baspp`: `BASIC++ Standard Edition v6.5.2\n640 MB RAM Available.\n\nOk\n`
  - `bpp`: `BASIC++ Lite Edition v6.5.2\n384 MB RAM Available.\n\nReady.\n`

---

## 2. Technical API Signatures (C17)

```c
const char *version_string(void);
const char *version_codename(void);
int version_major(void);
int version_minor(void);
int version_patch(void);
```
