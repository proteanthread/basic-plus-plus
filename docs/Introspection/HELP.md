# HELP Statement Reference

The `HELP` statement provides interactive, on-demand documentation and keyword syntax introspection directly within the BASIC++ environment.

## Syntax

```basic
HELP
HELP topic_or_keyword$
HELP "category_name"
```

## Parameters

- **`topic_or_keyword$`** *(Optional)* — A string or unquoted identifier specifying the keyword, statement, function, or library to look up (e.g. `HELP PRINT`, `HELP "OPEN"`, `HELP LPOS`).
- If called without arguments, `HELP` displays the master interactive help index, general syntax conventions, and categorized topic directories.

## Description

The `HELP` facility queries the built-in system metadata blocks (`MicroLibMetadata`) and static keyword documentation registries (`engine/src/statements/dialect/help_data.h`). It formats and prints syntax definitions, parameter descriptions, usage notes, error codes, and category classifications directly to the active virtual console (`VCon`).

Unlike historical dialects where documentation required external paper manuals, BASIC++ includes a complete, offline searchable reference engine that operates identically across desktop (`baspp`), lite REPL (`bpp`), and embedded builds.

```basic
> HELP PEEK
======================================================================
KEYWORD:      PEEK
CATEGORY:     Memory & Segment
SYNTAX:       byte% = PEEK(address%)
DESCRIPTION:  Reads a single byte (0-255) from the active segment.
ERROR CODES:  Error 5: Illegal Function Call, Error 13: Type Mismatch
======================================================================
Ok
```

---

## Code Examples

### Example 1: General Help Directory
```basic
HELP
REM Displays master help categories and search instructions
```

### Example 2: Inspecting Specific Keyword Syntax
```basic
HELP "CIRCLE"
REM Prints parameter list and graphics coordinate usage for CIRCLE
```

### Example 3: Filter by Category
```basic
HELP "Variables & Memory"
REM Displays all keywords associated with variable allocation and memory
```

---

## Engine Implementation (`help.c` & `help_data.h`)

In `engine/src/statements/system/help.c`, the `HELP` handler inspects arguments:

1. **Keyword Table Lookup**: Searches `g_help_entries[]` in `help_data.h` using case-insensitive string comparisons (`platform_strcasecmp`).
2. **Micro-Library Registry Lookup**: Queries dynamically registered statements and modules via `microlib_get_meta()`.
3. **Console Routing**: Text is emitted using `vdev_printf(vm_get_vdev(vm), ...)` to guarantee terminal formatting consistency across Windows console, Linux TTY, and graphical SDL2 windows.

```c
BppError stmt_help_handler(VMContext *vm, LexerContext *lex) {
    BppToken tok = lex_next(lex);
    VDevContext *vdev = vm_get_vdev(vm);
    if (tok.type == TOK_EOF || tok.type == TOK_EOL) {
        /* Display Master Help Index */
        vdev_printf(vdev, "BASIC++ Interactive Help System v%s\n", VERSION_STRING);
        vdev_printf(vdev, "Type HELP <keyword> or CATALOG for full listings.\n");
        return BPP_OK;
    }
    /* Search keyword registry */
    ...
}
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument passed to `HELP` evaluates to a numeric type |

---

## Cross-References

- **`CATALOG`** — Displays categorized keyword listings and functional group overviews.
- **`INFO`** — Displays system metrics, memory pool consumption, and runtime version.
- **`How_To_Use_Help.md`** — Top-level user guide for interactive help and introspection.

---

## Proposed Expansion or Changes

1. **Fuzzy Search & Spell Checking**: When an exact keyword is not found, suggest close matches (e.g. `HELP PRNT` suggests `PRINT`, `LPRINT`).
2. **Interactive Paged Help**: For terminal windows with limited row heights, support automatic spacebar pagination (`-- More --`).
