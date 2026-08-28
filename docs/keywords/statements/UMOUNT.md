# `UMOUNT` Unmount VFS Filesystem Device Statement

## 1. BASIC Usage and Keyword Definition

Unmounts a previously mounted virtual filesystem device or directory path.

### Syntax Signatures:
```basic
UMOUNT mount_path$
```

### Error Handling & Boundary Conditions:
- **Error 76 (ERR_PATH_NOT_FOUND)**: Mount path not found.

### Operational Notes:
- Flushes and closes all open handles on the mount before unmounting.

---

## 2. Code Examples

```basic
10 UMOUNT "/games"
```
