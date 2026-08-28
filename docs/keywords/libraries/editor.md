# `editor` Multi-Window TUI Editor Subsystem (`libstandard`)

## 1. Architectural Purpose & Overview

The `editor` subsystem (`engine/src/editor/editor.c`) provides the integrated QBASIC/Turbo C style full-screen text-mode program editor and terminal multiplexer.

### Key Architectural Invariants:
- **Syntax Highlighting**: On-the-fly keyword and comment colorizing via `vcon`.
- **Multi-Window Splitting**: Horizontal/vertical split panes and immediate watch windows.

---

## 2. Technical API Signatures (C17)

```c
void editor_launch(VMContext *vm, const char *file_path);
```
