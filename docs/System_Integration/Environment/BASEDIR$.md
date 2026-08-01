# BASEDIR$

## 1. Syntax & Parameters

**Syntax:**
`BASEDIR$`

**Parameters:**
None. `BASEDIR$` is a parameterless system string function.

## 2. Description & Usage

The `BASEDIR$` function returns the parent directory path of the current working directory as a string (e.g. if the current directory is `C:\Users\rtdos\GitHub\basic-plus-plus`, it returns `"C:\Users\rtdos\GitHub"`).

This is useful when you want to get the directory location containing the current project directory.

## 3. Code Examples

**Example 1: Basic String Assignment**
```basic
D$ = BASEDIR$
PRINT "Parent directory path: " + D$
```

## 4. Cross-References / See Also
- `BASEPATH$`, `BASENAME$`, `PWD`
