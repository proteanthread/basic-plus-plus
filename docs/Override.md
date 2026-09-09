<!--
Title:        Override
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/override.c, engine/src/runtime/override.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Override System Architecture

The authoritative specification for keyword execution interception, statement redirection, and user-defined runtime hooks in BASIC++ v6.5.2.

---

## 1. The `OVERRIDE` Statement

The `OVERRIDE` statement intercepts the execution of a built-in keyword and redirects its control flow to a user-defined subroutine or line number. Unlike `ALIAS` (which introduces synonym identifiers for existing keywords), `OVERRIDE` modifies the runtime behavior of the target keyword itself.

The override mechanism is implemented in `engine/src/statements/introspection/override.c` and `engine/src/runtime/override.c`.

---

## 2. Syntax and Operations

### A. Redirection to Line Number
```basic
OVERRIDE target WITH GOSUB line_num
```
Redirects execution of `target` keyword to `line_num`. When `target` is invoked in the program, the VM performs a `GOSUB` to the specified line number.

### B. Redirection to Named Subroutine
```basic
OVERRIDE target WITH sub_name
```
Redirects execution of `target` keyword to the named `SUB` procedure `sub_name`.

### C. Clearing Overrides
```basic
OVERRIDE CLEAR
```
Restores all overridden keywords to their original built-in engine implementations.

---

## 3. Protected Keywords

To prevent unrecoverable VM lockouts or interpreter corruption, core control flow and introspection keywords are permanently protected and cannot be overridden:

- `SCOPE`, `ALIAS`, `KEYWORD`, `OVERRIDE`
- `REM`, `END`, `STOP`, `NEW`, `RUN`, `CLEAR`

Attempting to override any protected keyword generates **Error 13: Permission Denied**.

---

## 4. Example: Custom Logging Hook

```basic
10 REM Custom PRINT Interception
20 OVERRIDE PRINT WITH GOSUB 5000
30 PRINT "Hello World"
40 OVERRIDE CLEAR
50 PRINT "Restored default PRINT"
60 END
5000 PRINT "[LOG] Custom handler executed"
5010 RETURN
```
