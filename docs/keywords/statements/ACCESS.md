# `ACCESS` File Access Mode Specification

## 1. BASIC Usage and Keyword Definition

Specifies file access permissions (READ, WRITE, READ WRITE) when opening a file channel.

### Syntax Signatures:
```basic
OPEN "data.dat" FOR mode ACCESS access_type AS #filenum
```

### Error Handling & Boundary Conditions:
- **Error 54 (ERR_BAD_FILE_MODE)**: Invalid access mode for open mode.
- **Error 70 (ERR_PERMISSION_DENIED)**: OS access permission denied.

### Operational Notes:
- Supported across GW-BASIC, QBASIC, and BASIC++ dialect modes.

---

## 2. Code Examples

```basic
10 OPEN "LOG.TXT" FOR INPUT ACCESS READ AS #1
20 LINE INPUT #1, L$
30 PRINT L$
40 CLOSE #1
```
