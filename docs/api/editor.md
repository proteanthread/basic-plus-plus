# C17 API Reference: TUI Editor Multiplexer (`editor/editor.h`)

## 1. Subsystem Overview & Responsibilities

The TUI Editor Subsystem (`editor/editor.h`, implemented in `engine/src/editor/editor.c`) provides an interactive text user interface (TUI) code editing environment and multiplexer plugin architecture for the BASIC++ v6.5.2 engine.

Key architectural responsibilities include:
- **Editor Plugin Interface**: Standardized `BppEditorPlugin` struct allowing pluggable text editor backends.
- **Built-in Editor Personalities**:
  - `WordStar` (`mod_ws_main`): Classical WordStar / Borland Turbo Pascal IDE diamond cursor navigation (`Ctrl+E/S/D/X`, `Ctrl+K+B/K/K`).
  - `Vi / Vim` (`mod_vi_main`): Modal editing with normal, insert, and visual modes (`h/j/k/l`, `dd`, `yy`, `p`, `:w`, `:q`).
  - `EDIT` (`mod_edit_main`): Microsoft MS-DOS / QuickBASIC style pull-down menu TUI editor.
  - `EDLIN` (`mod_edlin_main`): Line-oriented editor matching MS-DOS 1.0–5.0 EDLIN semantics.
- **TUI Multiplexer Lifecycle**: Manages terminal raw mode switching, full-screen buffer redrawing, syntax highlighting, and clean restoration of the standard console upon editor exit.

## 2. Header Inclusion & Prerequisites

```c
#include "editor/editor.h"
#include "vm/vm.h"
```

## 3. Data Structures & Types

```c
/* Common Editor Plugin Interface */
typedef struct {
    const char *name;                                                /* Editor name (e.g. "WS", "VI", "EDIT", "EDLIN") */
    int       (*run_editor)(VMContext *vm, const char *filename);     /* Editor execution entry point */
} BppEditorPlugin;
```

## 4. Function Prototypes & Operational Contracts

### Multiplexer Lifecycle & Execution
```c
/**
 * @brief Initializes the TUI terminal multiplexer and enables raw mode.
 */
void tui_multiplexer_init(void);

/**
 * @brief Shuts down the multiplexer and restores standard terminal console mode.
 */
void tui_multiplexer_shutdown(void);

/**
 * @brief Initializes editor plugin registry and binds built-in editors.
 */
void editor_manager_init(VMContext *vm);

/**
 * @brief Launches a named editor personality to edit a source file.
 * @param vm Pointer to active VMContext.
 * @param editor_name Name of the editor to launch ("EDIT", "VI", "WS", "EDLIN").
 * @param filename File path to open and edit.
 * @return Exit status code (0 = success).
 */
int editor_manager_run(VMContext *vm, const char *editor_name, const char *filename);
```

### Built-in Editor Entry Points
```c
int mod_ws_main(VMContext *vm, const char *filename);    /* WordStar */
int mod_vi_main(VMContext *vm, const char *filename);    /* Vi Modal */
int mod_edit_main(VMContext *vm, const char *filename);  /* QuickBASIC EDIT */
int mod_edlin_main(VMContext *vm, const char *filename); /* EDLIN */
```

## 5. Architectural Invariants

- **Terminal Restoration Guarantee**: The editor multiplexer guarantees that the host terminal's original cursor visibility, ANSI colors, and cooked mode state are completely restored upon exit, even on crash or signal interruption.
- **Excluded in Lite/Batch Targets**: The TUI editor subsystem is included in `baspp` (Standard Edition) and excluded from headless `bpp` and batch runner `bs` to optimize binary footprint.

## 6. Code Example: Launching QuickBASIC EDIT from Host C

```c
#include "editor/editor.h"
#include "core/boot.h"

void edit_script(VMContext *vm, const char *script_file) {
    editor_manager_init(vm);
    editor_manager_run(vm, "EDIT", script_file);
}
```
