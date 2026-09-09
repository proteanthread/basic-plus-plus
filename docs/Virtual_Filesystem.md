<!--
Title:        Virtual_Filesystem
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/runtime/vfs.c, engine/include/runtime/vfs.h
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Virtual Filesystem (VFS) Architecture

The authoritative specification for the Virtual Filesystem (`VFS`), file channels, security sandboxing, path canonicalization, and device nodes in BASIC++ v6.5.2.

---

## 1. Overview and Design Principles

The Virtual Filesystem (`VFS`) provides an abstraction layer between BASIC++ language I/O statements and the underlying platform operating system (`engine/src/runtime/vfs.c`). All high-level file operations (`OPEN`, `CLOSE`, `KILL`, `FILES`, `NAME`, `MKDIR`, `CHDIR`, `RMDIR`) pass through the VFS:

- **Path Canonicalization**: Normalizes directory separators (`/` vs `\`), strips redundant dots, and resolves relative paths deterministically.
- **Security Sandboxing**: Under restricted security levels, enforces directory root containment, preventing directory traversal attacks (`../`).
- **Device Namespace**: Mounts virtual device nodes (`DEV:`, `NET:`, `PRN:`, `COM1:`, `LPT1:`) directly into the unified file path space.

---

## 2. File Channel Management

VFS maintains the open channel descriptor table (`#1` through `#255`):
- Tracks active file access mode (`INPUT`, `OUTPUT`, `APPEND`, `RANDOM`, `BINARY`).
- Manages sector buffers, record lengths, 64-bit byte seek positions, and EOF status.
- Implements concurrent range locking (`LOCK #filenum, range` / `UNLOCK #filenum, range`).

---

## 3. Directory Operations

- **`CHDIR path$`**: Changes the active virtual working directory.
- **`MKDIR path$`**: Creates a new directory in the virtual filesystem.
- **`RMDIR path$`**: Removes an existing empty directory.
- **`FILES [pattern$]`**: Lists files matching the wildcard pattern.
- **`KILL filename$`**: Deletes a file from disk.
- **`NAME old$ AS new$`**: Renames an existing file or moves it within the filesystem.

---

## 4. Example: VFS File and Directory Inspection

```basic
10 REM VFS Directory Operations
20 TestDir$ = "sandbox_data"
30 MKDIR TestDir$
40 CHDIR TestDir$
50 OPEN "log.txt" FOR OUTPUT AS #1
60 PRINT #1, "VFS subsystem operational at "; TIME$
70 CLOSE #1
80 FILES "*.txt"
90 KILL "log.txt"
100 CHDIR ".."
110 RMDIR TestDir$
120 PRINT "VFS cleanup completed."
```
