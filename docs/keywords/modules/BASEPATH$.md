# `BASEPATH$` Canonical Directory Path Function

## 1. BASIC Usage and Function Definition

The `BASEPATH$` function returns the canonical, absolute base directory path where the currently executing BASIC++ script or module is stored on disk.

### Syntax Signatures:
```basic
dir$ = BASEPATH$
dir$ = BASEPATH$()
dir$ = BASEPATH$(relative_file$)
```

### Operational Rules:
- Resolves symbolic links and relative path segments (`.` and `..`) to form an absolute path.
- Useful for loading sibling data files or assets relative to script location.

---

## 2. Code Examples

```basic
10 SCRIPT_DIR$ = BASEPATH$
20 ASSET_FILE$ = BASEPATH$("assets/sprites.dat")
30 PRINT "Script located in: "; SCRIPT_DIR$
40 PRINT "Asset target path: "; ASSET_FILE$
```
