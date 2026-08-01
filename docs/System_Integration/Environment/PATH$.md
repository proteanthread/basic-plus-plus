# PATH$

## 1. Syntax & Parameters

**Syntax:**
`PATH$`
`PATH$(category$)`

**Parameters:**
- **category$** (optional): A string specifying the path category: `"WORKING"`, `"DATA"`, `"EXEC"`, or `"PROGRAM"`.

## 2. Description & Usage

Gets or sets virtual filesystem search paths.
- As a parameterless function `PATH$`, it returns the VFS search path string.
- With an argument `PATH$(category$)`, it returns the path for the specified category.
  - `"WORKING"` -> active working directory.
  - `"DATA"` -> data directory.
  - `"EXEC"` / `"PROGRAM"` -> directory where the currently loaded BASIC file resides.
- As a statement/LET target, it can be assigned to set the path:
  - `PATH$ = val$` -> sets main VFS search path list.
  - `PATH$(category$) = val$` -> sets specific category path.

## 3. Code Examples

```basic
PATH$ = "/bin;/data"
PRINT PATH$
PATH$("DATA") = "/my/custom/data"
PRINT PATH$("DATA")
```

## 4. Cross-References / See Also
- `BASEPATH$`, `BASEDIR$`
