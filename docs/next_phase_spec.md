# BASIC++ v6.x.x — Next Phase Specification

> **Version**: Phase 11 (Foundation → I/O → Advanced)
> **Status**: SPECIFICATION — Pending Implementation
> **Supersedes**: Previous `next_phase_spec.md` (5-item spec)

---

## Executive Summary

This specification defines the next major development phase for BASIC++, organized into three sub-phases plus cross-cutting work streams. It covers **13 new feature areas**, **5 existing partially-implemented features**, a comprehensive **API documentation package** for third-party embedding, and a **HELP/CATALOG parity catch-up** to close the gap between implemented keywords (~371) and documented ones (~160).

### Design Decisions (from /grill-me interview)

| Decision | Resolution |
|----------|------------|
| Goal | Both new features AND API documentation, updated continuously |
| Embedding model | C library (`libbasicpp`) + standalone executable with IPC |
| Doc format | Per-subsystem API docs in `docs/api/` + inline `.h` comments + tutorials |
| Doc grouping | By abstraction layer AND complexity (simple → complex) |
| Feature phasing | Three sub-phases: 11a (Foundation), 11b (I/O), 11c (Advanced) |
| Spec strategy | Single authoritative spec (merged with old `next_phase_spec.md`) |
| v5 migration | Proactive: port v5 code as features are implemented, archive to `v5/` |
| HELP/CATALOG | Full catch-up audit + regeneration from keyword scan |
| Status tracking | `IMPLEMENTATION_STATUS.md` + code comments + CATALOG metadata |
| Hardware abstraction | Configurable: `BPP_SDL2`, `BPP_HEADLESS`, `BPP_FRAMEBUFFER_ONLY` build flags |
| Keyword naming | Dual syntax: GW-BASIC classic + modern dot-access (interchangeable) |
| Compiler/transpiler | Full sync: every new feature includes `bppc` + `trans` updates |
| Execution order | All sub-phases in parallel via sub-agents |

---

## Phase 11a — Foundation (Language Core)

### 11a.1: Dialect Configuration Layer

**Status**: ⚠️ `src/dialect/` is EMPTY — core architectural requirement

**Goal**: Implement the dialect configuration system that allows users to declare their own dialects using shared VM and virtual device abstractions.

**Key deliverables**:
- Populate `src/dialect/` with the dialect configuration engine
- Port and supersede `source/dialect/dialect.c` (35 KB) from v5
- Port `source/config/` (4 files: `override.c`, `parser_config.c`, `scope.c`, `scope_stack.c`)
- Support loading dialect definitions from `.ini`, `.json`, `.yaml` files
- Integrate with existing `src/core/dialect.c` (15 KB) and `include/bpp_dialect.h`
- Enable keyword aliasing, syntax mode switching, and compatibility layers

**Syntax (classic)**:
```basic
DIALECT "gwbasic"          ' Switch to GW-BASIC compatibility mode
DIALECT "qbasic"           ' Switch to QBasic mode
DIALECT LOAD "custom.json" ' Load user-defined dialect
DIALECT INFO               ' Display current dialect configuration
```

**Syntax (modern)**:
```basic
dialect.set("gwbasic")
dialect.load("custom.json")
print dialect.info()
```

**v5 code to port**: `source/config/` (4 files), `source/dialect/dialect.c` (35 KB)
**Archive to v5/**: After porting, move originals to `v5/`

---

### 11a.2: Bit Manipulation Functions

**Status**: ❌ Not implemented

**Goal**: Add bitwise shift and individual bit manipulation functions.

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_SHL(n, bits)` | `bits.shl(n, bits)` | Shift left |
| `_SHR(n, bits)` | `bits.shr(n, bits)` | Shift right |
| `_READBIT(n, pos)` | `bits.read(n, pos)` | Read single bit |
| `_SETBIT(n, pos)` | `bits.set(n, pos)` | Set bit to 1 |
| `_RESETBIT(n, pos)` | `bits.reset(n, pos)` | Clear bit to 0 |
| `_TOGGLEBIT(n, pos)` | `bits.toggle(n, pos)` | Toggle bit |
| `_BITCOUNT(n)` | `bits.count(n)` | Count set bits (popcount) |

**Implementation**: Add to `src/expression/eval.c` as built-in functions. Register in `help_data.h`.

---

### 11a.3: Extended Math Functions

**Status**: ⚠️ `mod_mathext.c` (4 KB) exists but is likely partial

**Goal**: Fill gaps in mathematical functions vs. QB64/FreeBASIC.

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_ACOS(x)` | `math.acos(x)` | Inverse cosine |
| `_ASIN(x)` | `math.asin(x)` | Inverse sine |
| `_ATAN2(y, x)` | `math.atan2(y, x)` | Two-argument arctangent |
| `_ACOSH(x)` | `math.acosh(x)` | Hyperbolic inverse cosine |
| `_ASINH(x)` | `math.asinh(x)` | Hyperbolic inverse sine |
| `_ATANH(x)` | `math.atanh(x)` | Hyperbolic inverse tangent |
| `_CEIL(x)` | `math.ceil(x)` | Ceiling |
| `_HYPOT(x, y)` | `math.hypot(x, y)` | Hypotenuse |
| `_PI` | `math.pi` | Pi constant |
| `_D2R(deg)` | `math.d2r(deg)` | Degrees to radians |
| `_R2D(rad)` | `math.r2d(rad)` | Radians to degrees |
| `_D2G(deg)` | `math.d2g(deg)` | Degrees to gradians |
| `_G2D(grad)` | `math.g2d(grad)` | Gradians to degrees |

**Implementation**: Extend `src/module/mod_mathext.c`. All backed by C17 `<math.h>`.

---

### 11a.4: ENUM Support

**Status**: ❌ Not implemented

**Goal**: Named constant enumerations for cleaner code.

**Syntax**:
```basic
' Classic syntax:
ENUM Color
    Red = 1
    Green = 2
    Blue = 4
END ENUM

DIM c AS Color
c = Color.Green

' Modern syntax:
enum Color { Red = 1, Green = 2, Blue = 4 }
```

**Implementation**: New statement file `src/statements/stmt_enum.c`. Integrate with type system in `bpp_types.h`. ENUM values are compile-time integer constants stored in the variable table.

---

### 11a.5: WITH Blocks

**Status**: ❌ Not implemented

**Goal**: Shorthand for accessing struct/UDT members.

**Syntax**:
```basic
TYPE Point
    x AS SINGLE
    y AS SINGLE
END TYPE

DIM p AS Point
WITH p
    .x = 10.0
    .y = 20.0
    PRINT .x; .y
END WITH
```

**Implementation**: New statement file `src/statements/stmt_with.c`. Pushes a context onto the VM stack that resolves `.member` references against the WITH target.

---

## Phase 11b — I/O & Devices

### 11b.1: Sound System

**Status**: ⚠️ Only `stmt_beep.c` (3.3 KB) exists

**Goal**: Full sound system matching GW-BASIC SOUND/PLAY plus extended audio.

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `SOUND freq, dur` | `sound.tone(freq, dur)` | Play tone (GW-BASIC compatible) |
| `PLAY "MML"` | `music.play("MML")` | Play MML music string |
| `_SNDOPEN(file$)` | `sound.open(file)` | Open audio file |
| `_SNDPLAY(handle)` | `sound.play(handle)` | Play loaded audio |
| `_SNDLOOP(handle)` | `sound.loop(handle)` | Loop audio |
| `_SNDSTOP(handle)` | `sound.stop(handle)` | Stop playback |
| `_SNDPAUSE(handle)` | `sound.pause(handle)` | Pause playback |
| `_SNDVOL(handle, vol)` | `sound.volume(handle, vol)` | Set volume (0.0–1.0) |
| `_SNDLEN(handle)` | `sound.length(handle)` | Get duration |
| `_SNDGETPOS(handle)` | `sound.position(handle)` | Get playback position |
| `NOISE type, dur` | `sound.noise(type, dur)` | Generate noise |

**Virtual device**: New `src/device/vdev_sound.c`
**Build flags**: `BPP_SDL2` uses SDL2_mixer; `BPP_HEADLESS` uses null audio; `BPP_FRAMEBUFFER_ONLY` uses PC speaker emulation
**v5 code to port**: `source/sound/parser_sound.c` (9 KB)

---

### 11b.2: Mouse/Joystick Input

**Status**: ❌ Not implemented

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `MOUSE(0)..MOUSE(6)` | `mouse.x`, `mouse.y`, `mouse.button` | Mouse state (GW-BASIC func) |
| `_MOUSEINPUT` | `mouse.poll()` | Poll mouse queue |
| `_MOUSEWHEEL` | `mouse.wheel` | Scroll wheel delta |
| `_MOUSEHIDE`/`_MOUSESHOW` | `mouse.hide()`/`mouse.show()` | Cursor visibility |
| `STICK(n)` | `joystick.axis(n)` | Joystick axis (GW-BASIC) |
| `STRIG(n)` | `joystick.button(n)` | Joystick button (GW-BASIC) |
| `_DEVICES` | `input.device_count()` | Connected device count |
| `_DEVICE$(n)` | `input.device_name(n)` | Device name |

**Virtual device**: New `src/device/vdev_input.c`

---

### 11b.3: Clipboard Access

**Status**: ❌ Not implemented (but v5 had OS clipboard in editors)

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_CLIPBOARD$` | `clipboard.text()` | Read clipboard |
| `_CLIPBOARD$ = s$` | `clipboard.set(s)` | Write to clipboard |

**Implementation**: Platform-specific in `src/platform/platform.c` (Windows API / SDL2 / X11), exposed via `vdev_vcon.c`.

---

### 11b.4: Window Management

**Status**: ❌ Not implemented

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_TITLE s$` | `window.title = s` | Set window title |
| `_SCREENMOVE x, y` | `window.move(x, y)` | Move window |
| `_FULLSCREEN` | `window.fullscreen()` | Toggle fullscreen |
| `_SCREENX`/`_SCREENY` | `window.x`/`window.y` | Window position |
| `_DESKTOPWIDTH` | `screen.width` | Desktop resolution |
| `_DESKTOPHEIGHT` | `screen.height` | Desktop resolution |
| `_RESIZE ON/OFF` | `window.resizable = true` | Allow resize |
| `_ICON file$` | `window.icon = file` | Set window icon |

**Implementation**: Via `vdev_gfx.c` SDL2 backend. Headless builds ignore these.

---

## Phase 11c — Advanced Runtime

### 11c.1: TCP/IP Networking

**Status**: ⚠️ `vnet.c` (7 KB) exists but likely minimal

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `OPEN "TCP:host:port"` | `net.connect(host, port)` | TCP client connection |
| `_OPENHOST(port)` | `net.listen(port)` | TCP server listener |
| `_OPENCONNECTION(host_h)` | `net.accept(handle)` | Accept incoming connection |
| `_CONNECTED(handle)` | `net.connected(handle)` | Check connection status |
| `_CONNECTIONADDRESS$(h)` | `net.address(handle)` | Remote address |
| `NWRITE h, data$` | `net.send(handle, data)` | Send data |
| `NREAD$(h, len)` | `net.receive(handle, len)` | Receive data |

**Virtual device**: Extend `src/runtime/vnet.c` and `src/device/vdev_fujinet.c`
**v5 code to port**: `source/io/builtins_net.c`, `vdev_net.c`, `parser_net.c`

---

### 11c.2: Image File Loading

**Status**: ❌ Not implemented (BGI is vector/primitive-only)

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_LOADIMAGE(file$)` | `image.load(file)` | Load image file (BMP/PNG) |
| `_FREEIMAGE(handle)` | `image.free(handle)` | Release image |
| `_PUTIMAGE (x,y), handle` | `image.draw(handle, x, y)` | Draw image to screen |
| `_NEWIMAGE(w, h, mode)` | `image.create(w, h, mode)` | Create blank image |
| `_COPYIMAGE(handle)` | `image.copy(handle)` | Duplicate image |
| `_WIDTH(handle)` | `image.width(handle)` | Image width |
| `_HEIGHT(handle)` | `image.height(handle)` | Image height |

**Implementation**: Integrate with BGI framebuffer in `src/device/bgi/`. BMP loading is simple (no external deps); PNG optional via `stb_image.h` (public domain, single-header).

---

### 11c.3: Compression

**Status**: ❌ Not implemented

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_DEFLATE$(data$)` | `compress.deflate(data)` | Compress string |
| `_INFLATE$(data$, orig_size)` | `compress.inflate(data, size)` | Decompress string |

**Implementation**: Minimal zlib-compatible implementation or optional linkage to zlib. Can use a public-domain miniz.c (single file, C89-compatible).

---

### 11c.4: Session State Save/Load

**Status**: ❌ Not implemented (PC-BASIC has this)

**Keywords**:
| Classic | Modern | Description |
|---------|--------|-------------|
| `_STATESAVE file$` | `session.save(file)` | Save entire interpreter state |
| `_STATELOAD file$` | `session.load(file)` | Restore interpreter state |
| `_STATEINFO$(file$)` | `session.info(file)` | Query state file metadata |

**Implementation**: Serialize VM state, variable table, program text, and open file handles to a binary format. New file `src/runtime/state.c`.
**v5 code to consult**: `source/virtual/vm_snapshot.c` (2.4 KB)

---

## Existing Features — Status Update

These were in the previous `next_phase_spec.md` and are already partially implemented:

| Feature | v6 Status | Files | Notes |
|---------|-----------|-------|-------|
| Software Graphics Framebuffer | ✅ **Largely done** | `vdev_gfx.c` (77 KB), `bgi/` (4 files, 50 KB) | Functional but mode coverage expanding |
| Structured Datatypes (TYPE) | ✅ **Implemented** | `stmt_struct.c` (12 KB), `core/struct.c` (6 KB) | Working with dot-access |
| Event Trapping | ⚠️ **Partial** | `stmt_event.c` (5 KB) | ON TIMER/KEY/PLAY needs testing |
| Matrix/Array Math | ✅ **Extensive** | `stmt_mat.c` (50 KB) | 50 KB of ECMA-116 MAT operations |
| Dynamic Plugin Loader | ⚠️ **Partial** | `module.c` (10 KB) | Core loader exists, capability verification TBD |

---

## Cross-Cutting Work Streams

### C1: API Documentation Package

Create `docs/api/` with per-subsystem docs organized by abstraction layer:

**Tier 1 — Embedding & Lifecycle** (Start here for new developers)
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_boot.h` | `docs/api/boot.md` | Initialization, shutdown, entry points |
| `bpp_vm.h` | `docs/api/vm.md` | VM lifecycle, execution, line dispatch |
| `bpp_config.h` | `docs/api/config.md` | Configuration, options, dialect selection |
| `bpp_version.h` | `docs/api/version.md` | Version constants |

**Tier 2 — Language Core**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_lexer.h` | `docs/api/lexer.md` | Tokenizer, keyword table, token types |
| `bpp_eval.h` | `docs/api/eval.md` | Expression evaluation, operator dispatch |
| `bpp_stmt.h` | `docs/api/stmt.md` | Statement dispatch, registration |
| `bpp_types.h` | `docs/api/types.md` | Type system, value representation |
| `bpp_variables.h` | `docs/api/variables.md` | Variable storage, scoping |
| `bpp_arrays.h` | `docs/api/arrays.md` | Array management, DIM, REDIM |
| `bpp_strings.h` | `docs/api/strings.md` | String pool, string operations |

**Tier 3 — Runtime Extensions**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_file.h` | `docs/api/file.md` | File I/O, channels, modes |
| `bpp_funcreg.h` | `docs/api/funcreg.md` | Function registry, DEF FN |
| `bpp_map.h` | `docs/api/map.md` | Map/dictionary data structure |
| `bpp_struct.h` | `docs/api/struct.md` | User-defined types (TYPE) |
| `bpp_task.h` | `docs/api/task.md` | Cooperative multitasking |
| `bpp_spec.h` | `docs/api/spec.md` | Language specification queries |
| `bpp_metadata.h` | `docs/api/metadata.md` | Directives, pragmas |

**Tier 4 — Virtual Devices & I/O**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_vdev.h` | `docs/api/vdev.md` | Virtual device framework |
| `bpp_vcon.h` | `docs/api/vcon.md` | Virtual console |
| `bpp_vfs.h` | `docs/api/vfs.md` | Virtual filesystem |
| `bpp_vnet.h` | `docs/api/vnet.md` | Virtual networking |
| `bpp_bus.h` | `docs/api/bus.md` | Virtual bus |
| `bpp_usb.h` | `docs/api/usb.md` | USB device abstraction |
| `bpp_fujinet.h` | `docs/api/fujinet.md` | FujiNet retro networking |
| `bpp_gemini.h` | `docs/api/gemini.md` | Gemini protocol |

**Tier 5 — Graphics**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_bgi.h` | `docs/api/bgi.md` | BGI Graphics Interface |
| `bpp_mock_bios.h` | `docs/api/mock_bios.md` | Mock BIOS for heritage hardware |

**Tier 6 — Security & Modules**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_security.h` | `docs/api/security.md` | Security sandbox, capability system |
| `bpp_module.h` | `docs/api/module.md` | Module loading, validation pipeline |
| `mod_arrayext.h` | `docs/api/mod_arrayext.md` | Array extension module |

**Tier 7 — Platform & Memory**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_platform.h` | `docs/api/platform.md` | OS abstraction layer |
| `bpp_memory.h` | `docs/api/memory.md` | Memory management |
| `bpp_segmented_mem.h` | `docs/api/segmented_mem.md` | Segmented memory (RAMBANK) |

**Tier 8 — Dialect & Configuration**
| Header | API Doc | Topic |
|--------|---------|-------|
| `bpp_dialect.h` | `docs/api/dialect.md` | Dialect system |
| `bpp_logger.h` | `docs/api/logger.md` | Logging infrastructure |
| `bpp_editor.h` | `docs/api/editor.md` | Editor integration |
| `custom_dialect_static.h` | `docs/api/custom_dialect.md` | Static dialect definitions |

Each doc includes: Overview, Initialization, Public Functions (signature + description + example), Callbacks/Hooks, Error Handling, Embedding Example, and Tutorial.

**Master index**: `docs/api/INDEX.md`

---

### C2: HELP/CATALOG Parity Catch-Up

**Problem**: ~371 implemented keywords, ~160 in HELP/CATALOG.

**Approach**:
1. Scan `eval.c` and all `stmt_*.c` files for every registered keyword
2. Cross-reference against `help_data.h`, `help.TXT`, and `catalog.TXT`
3. Generate a gap report
4. Register missing keywords in `help_data.h` (online HELP)
5. Regenerate `help.TXT` and `catalog.TXT` from the engine
6. Add CATALOG metadata showing implementation status

**Tracking**: Keywords not yet implemented are tracked in `IMPLEMENTATION_STATUS.md` with ❌ status.

---

### C3: Implementation Status Tracker

New file: `docs/IMPLEMENTATION_STATUS.md`

Master tracking document with columns:
- **Keyword** — The keyword/function name
- **Category** — Arithmetic, Flow, I/O, Graphics, etc.
- **Implemented** — ✅/❌
- **Tested** — ✅/⚠️/❌
- **HELP Registered** — ✅/❌
- **CATALOG Listed** — ✅/❌
- **API Documented** — ✅/❌
- **Compiled (bppc)** — ✅/❌
- **Transpiled (trans)** — ✅/❌

---

### C4: Compiler/Transpiler Synchronization

Every new keyword/feature added in Phases 11a/b/c must also update:
1. `src/tools/bppc.c` — bytecode emission rule
2. `source/codegen/trans_c17.c` — C17 transpiler backend
3. `source/codegen/trans_py3.c` — Python 3 transpiler backend
4. `source/standalone/trans_cli.c` — transpiler CLI

**v5 code to port**: `source/codegen/` (17 files) should be progressively migrated to `src/tools/` or `src/codegen/` as backends are updated.

---

## Build Configuration

New build flags for the configurable hardware abstraction:

```cmake
option(BPP_SDL2 "Enable SDL2 backend for graphics, sound, and input" ON)
option(BPP_HEADLESS "Build headless (no graphics/sound)" OFF)
option(BPP_FRAMEBUFFER_ONLY "Software framebuffer only, no SDL2 window" OFF)
```

These are mutually exclusive. `baspp` defaults to `BPP_SDL2`. `blite`/`bscript` default to `BPP_HEADLESS`.

---

## v5 Migration Schedule

| Sub-Phase | v5 Files to Port | Archive After |
|-----------|-----------------|---------------|
| 11a | `source/dialect/dialect.c`, `source/config/` (4 files) | Move to `v5/` |
| 11b | `source/sound/parser_sound.c` | Move to `v5/` |
| 11c | `source/io/builtins_net.c`, `vdev_net.c`, `parser_net.c`, `source/virtual/vm_snapshot.c` | Move to `v5/` |
| C2 | `source/help/` (17 files) — reference for help data | Move to `v5/` |

---

## Verification Plan

Each sub-phase must pass before proceeding:

1. **Build**: `baspp.exe` compiles with 0 warnings, 0 errors (MSVC + GCC)
2. **SELFTEST**: `baspp.exe -c "SELFTEST"` passes all system tests
3. **Regression**: All existing tests in `tests/basic/` pass
4. **New tests**: Each new feature has a test file in `tests/basic/`
5. **Help parity**: New keywords registered in HELP/CATALOG
6. **Doc parity**: User guide + API doc + help text for each feature
7. **Compiler sync**: `bppc` can compile programs using new keywords
8. **Transpiler sync**: `trans` can transpile programs using new keywords
