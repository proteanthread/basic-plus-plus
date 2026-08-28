# `desktop` Flagship Desktop Target Edition Specification

## 1. Architectural Purpose & Overview

The `desktop` edition (`baspp` / `baspp.exe`) is the full-featured, flagship desktop development and runtime environment of BASIC++ v6.5.2. It provides complete compatibility with GW-BASIC, BASICA, and QBASIC, combining classic terminal interaction with high-performance modern graphics and audio capabilities.

### Key Architectural Specifications:
- **Default Memory Allocation**: 640 MB RAM (`671088640L` bytes).
- **Prompt & Status**: `>` prompt with classic `Ok` status output.
- **Dynamic Graphics/Audio**: On-demand delay-loading of `SDL2.dll` / `libSDL2.so` for `SCREEN`, `CIRCLE`, `LINE`, `PAINT`, `PLAY`, and `SOUND`.
- **TUI Multiplexer**: Integrated multi-window text-mode IDE and terminal multiplexer.
- **Virtual Devices**: Full virtual device bus (`vdev`, `vcon`, `vnet`, `vmem`).

---

## 2. Startup & Execution Modes

```bash
# Interactive Desktop REPL
baspp

# Execute BASIC program directly
baspp myprog.bas

# Immediate execution
baspp -c "PRINT 42 * 10"
```
