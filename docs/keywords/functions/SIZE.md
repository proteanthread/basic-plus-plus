# `SIZE` Data Structure Size Function

## 1. BASIC Usage and Function Definition

The `SIZE` function (and alias `LEN` for non-string types or `SIZEOF`) returns the size in bytes occupied by a variable, user-defined type (`TYPE ... END TYPE`), or array element in memory.

### Syntax Signatures:
```basic
bytes% = SIZE(variable_name)
bytes% = SIZE(type_name)
bytes% = SIZE(array_name())
```

### Operational Rules:
- Returns exact memory footprint in bytes:
  - `INTEGER` (%): 8 bytes (or 2/4 in legacy modes)
  - `SINGLE` (!): 4 bytes
  - `DOUBLE` (#): 8 bytes
  - `STRING` ($): Size of string reference descriptor (or length for fixed strings)
  - User-defined `TYPE`: Sum of field sizes and struct alignment padding.

---

## 2. Code Examples

```basic
10 TYPE Particle
20   x AS DOUBLE
30   y AS DOUBLE
40   mass AS DOUBLE
50 END TYPE
60 DIM P AS Particle
70 PRINT "Size of Particle structure: "; SIZE(P); " bytes (expected 24)"
```
