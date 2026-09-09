<!--
Title:        Scope
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/scope.c, engine/include/scope/scope.h
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Variable Scoping & Lexical Blocks Architecture

The authoritative specification for variable scoping, procedure frames, lexical block boundaries, module namespaces, and execution hooks in BASIC++ v6.5.2.

---

## 1. Variable Scoping Hierarchy

BASIC++ provides a structured variable scoping hierarchy supporting global variables, procedure frames, and lexical isolation:

- **`GLOBAL var1 [, var2...]`**: Declares variables in the root global scope. Accessible from any subroutine, function, or block.
- **`SHARED var1 [, var2...]`**: Within a `SUB` or `FUNCTION`, imports outer program variables into the local procedure frame.
- **`LOCAL var1 [, var2...]`**: Explicitly defines procedure-local variables allocated on the stack frame and shadowed from callers.
- **`STATIC var1 [, var2...]`**: Declares procedure-local variables whose values persist across subsequent invocations.

---

## 2. Lexical Scope Blocks

The `SCOPE` statement manages lexical scope frames, namespaces, and execution hooks (implemented in `engine/src/statements/introspection/scope.c`):

### A. Block Scoping
- **`SCOPE BEGIN`**: Pushes a new lexical scope frame onto the VM scope stack.
- **`SCOPE END`**: Pops the current lexical scope frame and releases frame-local variables.

### B. Module Namespaces
- **`SCOPE MODULE name$`**: Enters a named module namespace, isolating internal identifiers.

### C. Keyword Enablement and Protection
- **`SCOPE DISABLE keyword`**: Disables the specified keyword within the active lexical scope.
- **`SCOPE ENABLE keyword`**: Re-enables a previously disabled keyword.
- **`SCOPE PRIVATE symbol`**: Marks a variable or procedure symbol as private to the enclosing scope.

### D. Execution Hooks
- **`SCOPE HOOK BEFORE cmd GOSUB line|label`**: Installs a pre-execution hook called before `cmd` runs.
- **`SCOPE HOOK AFTER cmd GOSUB line|label`**: Installs a post-execution hook called after `cmd` runs.
- **`SCOPE HOOK CLEAR`**: Removes all registered execution hooks.

---

## 3. Example: Lexical Block Scoping

```basic
10 X = 100
20 PRINT "Outer X: "; X
30 SCOPE BEGIN
40   X = 42
50   PRINT "Inner X: "; X
60 SCOPE END
70 PRINT "Restored Outer X: "; X
80 END
```
