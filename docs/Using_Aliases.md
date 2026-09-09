<!--
Title:        Using_Aliases
Tier:         2
Applies to:   BASIC++ v6.5.2 (baspp, bpp, bs, iot)
Authority:    engine/src/statements/introspection/alias.c
Generated:    no, hand-written
Status:       current
-->

# BASIC++ v6.5.2 Using Aliases Architecture

The authoritative specification for keyword aliasing, syntax renaming, operator re-mapping, and identifier synonyms in BASIC++ v6.5.2.

---

## 1. The `ALIAS` Statement

The `ALIAS` statement creates an alternative name for an existing keyword, statement, function, or operator (`engine/src/statements/introspection/alias.c`). 

The alias behaves identically to the original keyword: it is an exact syntactic synonym evaluated through the primary parser dispatch table, not a text substitution macro. The original keyword remains fully accessible.

---

## 2. Syntax and Operations

### A. Keyword Aliasing
```basic
ALIAS new_name = original_keyword
```
Creates `new_name` as a synonym for `original_keyword`. Both identifiers are case-insensitive.

### B. Operator Aliasing
```basic
ALIAS OPER new_op = original_op
```
Maps an alternative operator token to an existing operator (e.g., mapping `//` to integer division `\`).

### C. Alias Management
- **`ALIAS LIST`**: Displays all currently active alias mappings on the console.
- **`ALIAS CLEAR`**: Removes all user-defined aliases and restores default keyword bindings.

---

## 3. Protected System Keywords

To prevent unrecoverable VM corruption, core control flow and introspection keywords cannot be aliased or overridden:

- `SCOPE`, `ALIAS`, `KEYWORD`, `OVERRIDE`
- `REM`, `END`, `STOP`, `NEW`, `RUN`, `CLEAR`

Attempting to redefine any protected keyword generates **Error 13: Permission Denied**.

---

## 4. Use Cases

- **Natural Language Localization**: Creating French, Spanish, or German keyword synonyms for educational environments.
- **Dialect Parity**: Mapping vintage dialect synonyms (e.g., `CLS` to `HOME` or `PRINT` to `?`).
- **Shorthand Typing**: Creating concise 1-letter or 2-letter shortcuts for frequently typed interactive commands.

---

## 5. Example: Keyword Synonyms

```basic
10 REM Alias Statement Demo
20 ALIAS DISPLAY = PRINT
30 ALIAS ASK = INPUT
40 DISPLAY "Hello from aliased DISPLAY statement!"
50 ASK "Enter your name: ", UserName$
60 DISPLAY "Welcome, "; UserName$
70 ALIAS CLEAR
```
