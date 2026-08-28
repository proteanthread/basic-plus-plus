# `FIELD` Random-Access Buffer Field Mapping Statement

## 1. BASIC Usage and Keyword Definition

Allocates sub-string variables into the fixed-length record buffer of a random-access file channel.

### Syntax Signatures:
```basic
FIELD [#]filenum%, width1% AS var1$ [, width2% AS var2$...]
```

### Error Handling & Boundary Conditions:
- **Error 52 (ERR_BAD_FILE_NUMBER)**: Channel not open for random access.
- **Error 54 (ERR_BAD_FILE_MODE)**: Channel not opened in mode R.

### Operational Notes:
- Pointers map directly to the channel record buffer.

---

## 2. Code Examples

```basic
10 OPEN "R", #1, "NAMES.DAT", 64
20 FIELD #1, 20 AS FName$, 20 AS LName$, 24 AS City$
30 GET #1, 1
40 PRINT "Name: "; FName$; " "; LName$
50 CLOSE #1
```
