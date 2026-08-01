# BASENAME$

## 1. Syntax & Parameters

**Syntax:**
`BASENAME$`

**Parameters:**
None. `BASENAME$` is a parameterless system string function.

## 2. Description & Usage

The `BASENAME$` function returns the name (basename) of the current working directory as a string (e.g. if the current directory is `C:\Users\rtdos\GitHub\basic-plus-plus`, it returns `"basic-plus-plus"`).

This is useful when you want to get the specific project or directory name without parsing the entire path.

## 3. Code Examples

**Example 1: Basic String Assignment**
```basic
D$ = BASENAME$
PRINT "Current project directory name: " + D$
```

## 4. Cross-References / See Also
- `BASEPATH$`, `BASEDIR$`, `PWD`
