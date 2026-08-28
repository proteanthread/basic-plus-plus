# `VPATH` Virtual Filesystem Search Path Configuration Statement

## 1. BASIC Usage and Keyword Definition

Configures or displays the ordered search paths used by the VFS when locating program and data files.

### Syntax Signatures:
```basic
VPATH [path_string$]
```

### Operational Notes:
- Supports semicolon-delimited directory search lists.

---

## 2. Code Examples

```basic
10 VPATH "/lib;/assets;/data"
```
