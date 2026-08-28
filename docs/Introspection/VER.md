# VER Statement Reference

The `VER` statement displays the current BASIC++ engine build version, target architecture, and the active program version tag (set via `VERSION`) to the virtual console.

## Syntax

```basic
VER
```

## Parameters

*(No parameters required.)*

## Description

The `VER` statement outputs version telemetry for the running environment. It displays both:
1. **Host Engine Version**: The compiled version of the BASIC++ interpreter (e.g. `BASIC++ v6.5.2 (64-Bit)`).
2. **Program Version Tag**: The application version string embedded in the currently loaded program via the `VERSION "x.y.z"` statement. If no tag has been set, it displays `"(None)"`.

```
======================================================================
BASIC++ Engine Version:  6.5.2 (Modern / 64-Bit)
Program Version Tag:   1.4.0-Release
======================================================================
Ok
```

---

## Code Examples

### Example 1: Tagging a Program and Querying Version
```basic
10 VERSION "2.1.0-Production"
20 PRINT "Running Application..."
30 VER
```

### Example 2: Interactive Immediate Mode Query
```basic
> VER
BASIC++ Engine Version:  6.5.2 (Modern / 64-Bit)
Program Version Tag:   (None)
Ok
```

---

## Engine Implementation (`version.c`)

In `engine/src/statements/system/version.c`:

```c
BppError stmt_ver_handler(VMContext *vm, LexerContext *lex) {
    VDevContext *vdev = vm_get_vdev(vm);
    const char *prog_ver = mem_get_program_version(vm_get_mem(vm));
    if (!prog_ver || prog_ver[0] == '\0') {
        prog_ver = "(None)";
    }

    vdev_printf(vdev, "BASIC++ Engine Version:  %s (%s)\n",
                VERSION_STRING, BASIC_PROFILE_NAME);
    vdev_printf(vdev, "Program Version Tag:   %s\n", prog_ver);
    return BPP_OK;
}
```

---

## Error Codes

| Error Code | Name | Condition |
|------------|------|-----------|
| 2 | Syntax Error (`ERR_SYNTAX_ERROR`) | Trailing unexpected tokens passed to `VER` |

---

## Cross-References

- **`VERSION`** — Tags the active program with a version string.
- **`INFO`** — Displays complete system metrics and memory telemetry.
- **`HELP`** — Interactive keyword documentation.

---

## Proposed Expansion or Changes

1. **`VER$` Builtin Function**: Expose `ver$ = VER$` returning the combined version string for automated logging and telemetry scripts.
2. **Build Hash Verification**: Output git commit hash and compilation timestamp when invoked with `VER FULL`.
