# BASIC++ Editing Commands

**Version 4.1.1**


---

## Table of Contents

- Line Editing (AUTO / DELETE / EDIT / RENUM)
  - AUTO — Automatic Line Numbering
  - DELETE — Remove Program Lines
  - EDIT — Edit a Line
  - RENUM — Renumber Program Lines
- ALIAS — Keyword Remapping
  - Setting Aliases
  - Listing and Managing
  - Saving and Loading
  - Language Packs
- ALIAS$ — Alias Lookup Function
- KEYWORD — Keyword Properties
- OVERRIDE — Runtime Keyword Replacement
- SCOPE — Keyword Access Control
  - Enabling and Disabling
  - Execution Hooks
  - Presets
  - Listing

---

This guide covers all editing, customization, and keyword management commands in BASIC++.

---

## Line Editing (AUTO / DELETE / EDIT / RENUM)

### AUTO — Automatic Line Numbering

`AUTO` begins automatic line numbering for rapid entry:

```
AUTO             ' Start at 10, step 10
AUTO 100         ' Start at 100, step 10
AUTO 100, 5      ' Start at 100, step 5
```

Each time you press Enter, the next line number appears automatically. Press **Ctrl+C** or enter a blank line to exit AUTO mode.

If a line number already exists, AUTO shows it with a `*` prefix to warn you that entering text will overwrite it.

### DELETE — Remove Program Lines

`DELETE` removes one or more lines from the program:

```
DELETE 100       ' Delete line 100
DELETE 100-200   ' Delete lines 100 through 200
```

You can also delete a line by typing its number with no content:

```
100              ' (empty line — deletes line 100)
```

### EDIT — Edit a Line

`EDIT` opens a line for in-place editing:

```
EDIT 100
```

The current content of line 100 is displayed and you can modify it. Press Enter to accept changes.

### RENUM — Renumber Program Lines

`RENUM` renumbers all lines in the program:

```
RENUM            ' Renumber starting at 10, step 10
RENUM 100        ' Start at 100, step 10
RENUM 100, 5     ' Start at 100, step 5
```

`RENUM` automatically updates all `GOTO`, `GOSUB`, `ON...GOTO`, `ON...GOSUB`, `RESTORE`, and `RESUME` references to match the new line numbers.

---

## ALIAS — Keyword Remapping

`ALIAS` lets you remap any BASIC++ keyword to a custom name. This is the foundation for localized/translated BASIC.

### Setting Aliases

```basic
ALIAS "IMPRE" = PRINT      ' Map "IMPRE" to PRINT
ALIAS "SI" = IF            ' Map "SI" to IF
ALIAS "ALLER" = GOTO       ' Map "ALLER" to GOTO
```

After aliasing, both the alias and the original work:

```basic
IMPRE "Bonjour"            ' Prints: Bonjour
PRINT "Hello"              ' Still works
```

### Listing and Managing

| Command | Description |
|---------|-------------|
| `ALIAS LIST` | Show all active aliases |
| `ALIAS CLEAR "IMPRE"` | Remove one alias |
| `ALIAS CLEAR ALL` | Remove all aliases |
| `ALIAS COUNT` | Show number of active aliases |

### Saving and Loading

```basic
ALIAS SAVE "french.als"    ' Save alias set to file
ALIAS LOAD "french.als"    ' Load alias set from file
```

### Language Packs

`ALIAS LANG` loads a built-in language translation:

```basic
ALIAS LANG "SPANISH"       ' Load Spanish keyword aliases
ALIAS LANG "FRENCH"        ' Load French keyword aliases
```

Use `ALIAS LANG` without an argument to list available packs.

---

## ALIAS$ — Alias Lookup Function

`ALIAS$` is a read-only function that looks up mappings:

```basic
PRINT ALIAS$("IMPRE")     ' prints "PRINT"
PRINT ALIAS$("PRINT")     ' prints "IMPRE" (reverse lookup)
```

If no alias exists, `ALIAS$` returns an empty string.

---

## KEYWORD — Keyword Properties

`KEYWORD` modifies or queries built-in keyword behavior:

```basic
KEYWORD PRINT UPPERCASE ON     ' Forces PRINT to uppercase all output
KEYWORD PRINT UPPERCASE OFF    ' Disables forced uppercase
KEYWORD PRINT DESCRIBE         ' Shows description and properties
```

The `KEYWORD` command provides programmatic access to the keyword property system (see `keyword_props.c`).

---

## OVERRIDE — Runtime Keyword Replacement

`OVERRIDE` replaces how a keyword is interpreted at runtime without modifying source code:

```basic
OVERRIDE PRINT "PRINT TAB(7);"
```

Every `PRINT` statement is now prefixed with `TAB(7)`.

| Command | Description |
|---------|-------------|
| `OVERRIDE LIST` | Lists all active overrides |
| `OVERRIDE RESET` | Removes all active overrides |
| `OVERRIDE PRINT RESET` | Removes only the PRINT override |

Overrides apply to **all** uses of the keyword in the program. They are designed for batch processing, formatting, and output transformation without touching source code.

---

## SCOPE — Keyword Access Control

`SCOPE` controls which keywords are available and adds execution hooks.

### Enabling and Disabling

```basic
SCOPE DISABLE GOTO         ' Prevent use of GOTO
SCOPE ENABLE GOTO          ' Re-enable GOTO

SCOPE DISABLE POKE         ' Prevent memory writes
SCOPE DISABLE SHELL        ' Prevent OS access
```

Disabled keywords produce an error if used:

```
?Keyword disabled: GOTO
```

### Execution Hooks

```basic
SCOPE BEFORE PRINT GOSUB 9000    ' Call before every PRINT
SCOPE AFTER INPUT GOSUB 9100     ' Call after every INPUT
SCOPE OVERRIDE PRINT GOSUB 9200  ' Replace PRINT entirely
```

### Presets

| Preset | Description |
|--------|-------------|
| `SCOPE "STANDARD"` | Default keyword set |
| `SCOPE "MINIMAL"` | Bare minimum keywords |
| `SCOPE "CREATIVE"` | Disabled: GOTO, GOSUB (forces structured programming) |
| `SCOPE "SAFE"` | Disabled: SHELL, POKE, KILL |

### Listing

| Command | Description |
|---------|-------------|
| `SCOPE LIST` | Show all scope rules |
| `SCOPE LIST DISABLED` | Show disabled keywords only |
| `SCOPE LIST HOOKS` | Show active hooks only |
