# `iot` Lite REPL Target Edition Specification

## 1. Architectural Purpose & Overview

The `iot` target edition (`bpp` / `bpp.exe`) is a headless, lightweight interactive REPL optimized for terminal environments, connected IoT gateways, single-board computers (Raspberry Pi), and remote network access.

### Key Architectural Specifications:
- **Default Memory Allocation**: 384 MB RAM (`402653184L` bytes).
- **Prompt & Status**: `]` prompt with classic `Ready.` status (Apple II / Commodore style).
- **Excluded Subsystems**: Stripped of SDL2 graphics, BGI rasterization, SDL audio, TUI editor multiplexer, and segmented memory (`vmem`).
- **Core Linkage**: Links `HEADLESS_OBJECTS` and `libcore` for fast terminal execution.

---

## 2. Command-Line Usage

```bash
# Launch Lite REPL
bpp

# Run script in Lite mode
bpp sensor_monitor.bas
```
