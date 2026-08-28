# `HOSTNAME$` Host System Computer Name Function

## 1. BASIC Usage and Function Definition

The `HOSTNAME$` function returns the network computer name / hostname of the local machine from `libplatform`.

### Syntax Signatures:
```basic
host$ = HOSTNAME$
host$ = HOSTNAME$()
```

### Operational Rules:
- Returns network hostname string (e.g. `"workstation-01"`, `"debian-server"`).

---

## 2. Code Examples

```basic
10 PRINT "Host Machine Name: "; HOSTNAME$
```
