# `CLR` / `CLEAR` Clear Program Variables Statement

## 1. BASIC Usage and Keyword Definition

The `CLR` statement (and alias `CLEAR`) clears all variables from active memory, resets array allocations, releases reference-counted string pools, and resets runtime execution states without removing the stored program lines.

### Syntax Signatures:
```basic
CLR
CLEAR [, [stack_size&] [, [basic_data_size&]]]
```

### Operational Rules:
- **Variable Reset**: Clears all scalars and deallocates arrays (`var_clear_all()`).
- **String Memory**: Releases reference-counted strings.
- **File Buffers**: Resets I/O channel states and closes open files if parameters are passed.
- **Sound/Graphics**: Resets active audio and sprite channels.

---

## 2. Code Examples

```basic
10 A = 100 : B$ = "Active data"
20 PRINT "Before CLR: A="; A; " B$="; B$
30 CLR
40 PRINT "After CLR : A="; A; " B$='"; B$; "'"
```
