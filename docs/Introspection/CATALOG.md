# CATALOG Statement Reference

The `CATALOG` statement lists the master inventory of all built-in keywords, statements, and mathematical/string functions in BASIC++ v6.5.2, organized by functional category.

## Syntax

```basic
CATALOG
CATALOG "category_name"
```

## Parameters

- **`"category_name"`** *(Optional)* — A string expression filtering the catalogue output to a specific functional category (e.g. `CATALOG "Strings"`, `CATALOG "Math"`, `CATALOG "Graphics & Sound"`).
- If invoked without arguments, `CATALOG` prints all registered categories and total keyword counts.

## Description

The `CATALOG` command serves as an interactive environment inventory explorer. It scans the internal help registry (`engine/src/statements/dialect/help_data.h`) and metadata registries, presenting keywords in clean tabular columns.

### Standard Categories:
1. **`Control Flow`** — Branching, looping, subroutines, event traps (`IF`, `FOR`, `DO/LOOP`, `SELECT`, `ON ERROR`, `GOSUB`).
2. **`Variables & Memory`** — Array allocation, variable types, scopes, segment access (`DIM`, `DEF SEG`, `PEEK`, `POKE`, `COMMON`, `TYPE`).
3. **`Strings`** — String analysis, search, substring extraction, formatting (`LEFT$`, `MID$`, `RIGHT$`, `INSTR`, `RTRIM$`, `OCT$`, `HEX$`).
4. **`Math`** — Arithmetic, trigonometry, logarithms, matrix math (`SIN`, `COS`, `LOG`, `EXP`, `MAT`, `HI`, `LO`).
5. **`Input / Output`** — Console streams, printers, serial devices (`PRINT`, `INPUT`, `LOCATE`, `COLOR`, `LPRINT`, `LPOS`, `IOCTL`).
6. **`Graphics & Sound`** — BGI raster graphics, pixel shaders, audio synthesis (`SCREEN`, `PSET`, `LINE`, `CIRCLE`, `SOUND`, `PLAY`).
7. **`Filesystem`** — File handles, random access, directory navigation (`OPEN`, `CLOSE`, `GET`, `PUT`, `KILL`, `CHDIR`, `FILES`).
8. **`Devices & Network`** — Sockets, Gemini protocol, virtual devices (`VDEV`, `MOUNT`, `DEVICES`, `NET`).
9. **`Debug & Testing`** — Tracing, breakpoints, self-test verification (`TRON`, `TROFF`, `DEBUG`, `ASSERT`, `SELFTEST`).
10. **`System & Environ`** — Timers, alarms, environment strings, system control (`TIME$`, `DATE$`, `JIFFIES`, `ENVIRON$`, `SYSTEM`).

---

## Code Examples

### Example 1: Full System Catalogue
```basic
CATALOG
REM Outputs summary of all categories and available keywords
```

### Example 2: Filtering by Category
```basic
CATALOG "Graphics & Sound"
REM Lists all drawing primitives, color settings, and audio playback statements
```

---

## Engine Implementation (`help.c`)

In `engine/src/statements/system/help.c`, `stmt_catalog_handler` parses optional category filters and iterates through the `g_help_entries` database in `help_data.h`:

```c
BppError stmt_catalog_handler(VMContext *vm, LexerContext *lex) {
    VDevContext *vdev = vm_get_vdev(vm);
    BppToken tok = lex_next(lex);

    if (tok.type == TOK_STRING || tok.type == TOK_IDENT) {
        /* Filter and list keywords matching the category */
        catalog_display_category(vdev, tok.start, tok.length);
    } else {
        /* Display all categories with item counts */
        catalog_display_all(vdev);
    }
    return BPP_OK;
}
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 13 | Type Mismatch (`ERR_TYPE_MISMATCH`) | Argument passed to `CATALOG` is a numeric type |

---

## Cross-References

- **`HELP`** — Queries detailed syntax, parameters, and examples for a single keyword.
- **`INFO`** — Displays system health, memory usage metrics, and build info.
- **`How_To_Use_Help.md`** — Comprehensive guide to the interactive help subsystem.

---

## Proposed Expansion or Changes

1. **JSON Catalogue Export**: Support `CATALOG --json` or `CATALOG$()` to export the entire language catalogue as structured JSON data for IDEs and tooling.
2. **Columnar Screen Formatting**: Dynamically calculate terminal width (`WIDTH`) to format keywords in 3, 4, or 5 columns automatically.
