# `MOUNT` Virtual Filesystem Device Mount Statement

## 1. BASIC Usage and Keyword Definition

Mounts a host folder, ZIP archive, or memory image onto the virtual filesystem (VFS) namespace.

### Syntax Signatures:
```basic
MOUNT device_spec$, mount_path$ [, filesystem_type$]
```

### Error Handling & Boundary Conditions:
- **Error 68 (ERR_DEVICE_UNAVAILABLE)**: Device could not be mounted.

### Operational Notes:
- Extensible VFS architecture.

---

## 2. Code Examples

```basic
10 MOUNT "C:\GAMES", "/games", "host"
20 FILES "/games/"
```
