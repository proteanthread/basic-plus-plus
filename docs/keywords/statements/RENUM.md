# `RENUM` Program Line Renumbering Command

## 1. BASIC Usage and Keyword Definition

Renumbers program lines and automatically updates all GOTO, GOSUB, and ON...GOTO/GOSUB line references.

### Syntax Signatures:
```basic
RENUM [new_start%] [, [old_start%] [, [increment%]]]
```

### Error Handling & Boundary Conditions:
- **Error 8 (ERR_UNDEFINED_LINE_NUMBER)**: Unresolved target line reference.

### Operational Notes:
- Essential program maintenance utility.

---

## 2. Code Examples

```basic
RENUM         : REM Renumbers all lines to 10, 20, 30...
RENUM 100, 10 : REM Starts at 100 with step of 10
```
