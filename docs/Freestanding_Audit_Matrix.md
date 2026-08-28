# Master File-by-File Freestanding C17 Audit Matrix

> **Standard Conformance**: ISO/IEC 9899:2018 (C17 Freestanding), ISO/IEC 25010:2023 (Software Quality & Maintainability), ISO/IEC 5055:2021 (Automated Source Code Quality), MISRA C:2012 / SEI CERT C (Safety-Critical & Bounded Operations), EU Cyber Resilience Act (CRA - Regulation EU 2024/2847 - Security by Design), and EU GDPR Article 25 (Privacy by Default).

> **Audit Scope**: Exhaustive 100% file-by-file inventory of all **1082 source and header files** across the 11 functional subsystem layers, micro-libraries, and tool targets.

## Executive Classification Summary

| Classification Tier | File Count | Percentage | Definition & Architectural Action Path |
|---|---|---|---|
| 🟢 **GREEN (Freestanding C17 Ready)** | **492** | **45.5%** | Includes only standard freestanding C17 headers (`stdint.h`, `stdbool.h`, `stddef.h`, `stdarg.h`, `float.h`, `limits.h`). Zero code modifications required. |
| 🟡 **YELLOW (Convertible via Runtime Subsystem)** | **551** | **50.9%** | Uses hosted libc (`string.h`, `ctype.h`, `math.h`, `stdlib.h`). Converts directly to freestanding `runtime/` subsystem headers with >= 15-16 digit precision. |
| 🔴 **RED (Requires HAL Isolation)** | **39** | **3.6%** | Uses OS-specific APIs (`windows.h`, `termios.h`, `sys/socket.h`, `SDL.h`, `FILE *`). Encapsulated behind `hal/` abstraction callback interfaces. |
| **Total Engine & Micro-Library Files** | **1082** | **100.0%** | **Total Source Lines of Code (SLOC): 114,754** |

---

## Subsystem Layer Distribution Matrix

| Functional Subsystem Layer | Total Files | 🟢 Green | 🟡 Yellow | 🔴 Red | Primary Header Dependencies | Target Category Destination |
|---|---|---|---|---|---|---|
| `TinyGL` | 50 | 15 | 20 | 15 | GL/gl.h, GL/glx.h, GL/nglx.h, GL/oscontext.h | `TinyGL/` (Freestanding GL Adapter) |
| `engine` | 55 | 40 | 14 | 1 | ctype.h, ole2.h, oleauto.h, stdio.h | Dedicated Subsystem Directory |
| `functions_microlib` | 1 | 1 | 0 | 0 | None (Pure Freestanding) | `src/eval/functions/<sub-category>/` |
| `libadvanced` | 2 | 0 | 1 | 1 | SDL.h, ctype.h, stdio.h, stdlib.h | Dedicated Subsystem Directory |
| `libboot` | 10 | 3 | 7 | 0 | ctype.h, stdio.h, stdlib.h, string.h | Dedicated Subsystem Directory |
| `libcore` | 4 | 0 | 4 | 0 | ctype.h, stdio.h, stdlib.h, string.h | Dedicated Subsystem Directory |
| `libengine` | 380 | 183 | 195 | 2 | ctype.h, dirent.h, fnmatch.h, math.h | Dedicated Subsystem Directory |
| `libflex` | 3 | 0 | 3 | 0 | ctype.h, stdio.h, stdlib.h, string.h | Dedicated Subsystem Directory |
| `libkernel` | 90 | 21 | 67 | 2 | arpa/inet.h, ctype.h, fcntl.h, math.h | Dedicated Subsystem Directory |
| `libplatform` | 11 | 0 | 1 | 10 | arpa/inet.h, conio.h, direct.h, dirent.h | `lib/platform/` (HAL Hosted Backend) |
| `libstandard` | 34 | 8 | 21 | 5 | conio.h, ctype.h, io.h, poll.h | Dedicated Subsystem Directory |
| `statements_microlib` | 435 | 220 | 212 | 3 | ctype.h, io.h, math.h, stdio.h | `src/statements/<sub-category>/` |
| `tools` | 7 | 1 | 6 | 0 | ctype.h, math.h, stdio.h, stdlib.h | Dedicated Subsystem Directory |

---

## Detailed File-by-File Audit Inventory

### Subsystem: `TinyGL` (50 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🔴 | `TinyGL/BeOS/GLView.h` | 82 | interface/View.h, support/Locker.h, game/WindowScreen.h, game/DirectWindow.h, GL/gl.h, GL/oscontext.h | - | hal/hal.h | Preserve |
| 🔴 | `TinyGL/examples/gears.c` | 299 | math.h, stdlib.h, stdio.h, string.h, GL/glx.h, GL/gl.h | system_os (exit); math_trig (sin cos); conv (atoi); math_alg (sqrt) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/, runtime/conv/num_parse.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `TinyGL/examples/glu.c` | 269 | stdlib.h, math.h, GL/gl.h | math_trig (sin cos tan); math_alg (sqrt) | runtime/memory/alloc.h, runtime/math/, hal/hal.h | Preserve |
| 🟢 | `TinyGL/examples/glu.h` | 32 | - | - | None | Preserve |
| 🔴 | `TinyGL/examples/mech.c` | 1727 | stdlib.h, stdio.h, math.h, fpu_control.h, GL/gl.h, GL/glx.h | stdio_io (printf); memory (free); math_trig (sin cos acos); math_alg (sqrt) | runtime/memory/alloc.h, runtime/math/, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `TinyGL/examples/nanox.c` | 118 | stdio.h, stdlib.h, string.h, microwin/nano-X.h, GL/gl.h, GL/nglx.h | system_os (exit); stdio_io (fprintf) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `TinyGL/examples/spin.c` | 161 | stdlib.h, math.h, stdio.h, GL/glx.h, GL/gl.h | system_os (exit) | runtime/memory/alloc.h, runtime/math/, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `TinyGL/examples/texobj.c` | 196 | math.h, stdlib.h, stdio.h, string.h, GL/glx.h, GL/gl.h | system_os (exit) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h, hal/hal.h | Preserve |
| 🟢 | `TinyGL/examples/ui.h` | 24 | - | - | None | Preserve |
| 🔴 | `TinyGL/examples/x11.c` | 149 | math.h, stdlib.h, stdio.h, string.h, X11/Xutil.h, X11/keysym.h, X11/Xlib.h, GL/glx.h, GL/gl.h | system_os (exit); stdio_io (fprintf) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h, hal/hal.h | Preserve |
| 🟢 | `TinyGL/include/GL/gl.h` | 841 | - | - | None | Preserve |
| 🔴 | `TinyGL/include/GL/glx.h` | 146 | X11/Xutil.h, X11/Xlib.h, GL/gl.h | - | hal/hal.h | Preserve |
| 🔴 | `TinyGL/include/GL/nglx.h` | 37 | microwin/nano-X.h, GL/gl.h | - | hal/hal.h | Preserve |
| 🟢 | `TinyGL/include/GL/oscontext.h` | 47 | - | - | None | Preserve |
| 🔴 | `TinyGL/include/GLView.h` | 82 | interface/View.h, support/Locker.h, game/WindowScreen.h, game/DirectWindow.h, GL/gl.h, GL/oscontext.h | - | hal/hal.h | Preserve |
| 🟡 | `TinyGL/src/api.c` | 678 | stdio.h | - | hal/io_hal.h | Preserve |
| 🟡 | `TinyGL/src/arrays.c` | 218 | assert.h, stdio.h | - | hal/io_hal.h | Preserve |
| 🟢 | `TinyGL/src/clear.c` | 40 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/clip.c` | 450 | - | math_basic (abs); stdio_io (printf) | runtime/math/, hal/io_hal.h | Preserve |
| 🟡 | `TinyGL/src/error.c` | 26 | - | system_os (exit); stdio_io (fprintf) | hal/io_hal.h | Preserve |
| 🟡 | `TinyGL/src/get.c` | 82 | - | stdio_io (fprintf) | hal/io_hal.h | Preserve |
| 🔴 | `TinyGL/src/glx.c` | 422 | X11/extensions/XShm.h, GL/glx.h, sys/ipc.h, sys/shm.h | system_os (exit); stdio_io (fprintf) | hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `TinyGL/src/image_util.c` | 140 | - | math_basic (floor) | runtime/math/ | Preserve |
| 🟢 | `TinyGL/src/init.c` | 199 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/light.c` | 314 | - | math_trig (cos); math_alg (sqrt pow) | runtime/math/ | Preserve |
| 🟡 | `TinyGL/src/list.c` | 265 | - | stdio_io (fprintf) | hal/io_hal.h | Preserve |
| 🟡 | `TinyGL/src/matrix.c` | 251 | - | stdio_io (fprintf); math_trig (sin cos); math_alg (sqrt) | runtime/math/, hal/io_hal.h | Preserve |
| 🟡 | `TinyGL/src/memory.c` | 28 | - | memory (calloc free) | runtime/memory/alloc.h | Preserve |
| 🟡 | `TinyGL/src/misc.c` | 155 | - | stdio_io (fprintf) | hal/io_hal.h | Preserve |
| 🟡 | `TinyGL/src/msghandling.c` | 62 | stdio.h | stdio_io (fprintf) | hal/io_hal.h | Preserve |
| 🟢 | `TinyGL/src/msghandling.h` | 18 | - | - | None | Preserve |
| 🔴 | `TinyGL/src/nglx.c` | 137 | microwin/nano-X.h, GL/gl.h, GL/nglx.h | system_os (exit); stdio_io (fprintf) | hal/io_hal.h, hal/hal.h | Preserve |
| 🟢 | `TinyGL/src/opinfo.h` | 79 | - | - | None | Preserve |
| 🔴 | `TinyGL/src/oscontext.c` | 94 | stdlib.h, assert.h, GL/oscontext.h, GL/gl.h | system_os (exit); stdio_io (fprintf) | runtime/memory/alloc.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟢 | `TinyGL/src/select.c` | 124 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/specbuf.c` | 62 | math.h, stdlib.h | math_alg (pow) | runtime/memory/alloc.h, runtime/math/ | Preserve |
| 🟢 | `TinyGL/src/specbuf.h` | 32 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/texture.c` | 235 | - | string_mem (memcpy) | runtime/string/memops.h | Preserve |
| 🟡 | `TinyGL/src/vertex.c` | 374 | - | string_mem (memcpy) | runtime/string/memops.h | Preserve |
| 🟡 | `TinyGL/src/zbuffer.c` | 521 | stdlib.h, stdio.h, assert.h, string.h | string_mem (memcpy) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `TinyGL/src/zbuffer.h` | 161 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/zdither.c` | 168 | stdlib.h, stdio.h, assert.h | system_os (exit); stdio_io (fprintf) | runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `TinyGL/src/zfeatures.h` | 51 | - | - | None | Preserve |
| 🔴 | `TinyGL/src/zgl.h` | 382 | stdlib.h, stdio.h, math.h, assert.h, string.h, GL/gl.h | stdio_io (fprintf) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `TinyGL/src/zline.c` | 94 | stdlib.h | - | runtime/memory/alloc.h | Preserve |
| 🟢 | `TinyGL/src/zline.h` | 131 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/zmath.c` | 281 | stdlib.h, string.h, math.h | string_mem (memcpy); math_trig (sin cos); math_basic (fabs); math_alg (sqrt) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/ | Preserve |
| 🟢 | `TinyGL/src/zmath.h` | 63 | - | - | None | Preserve |
| 🟡 | `TinyGL/src/ztriangle.c` | 400 | stdlib.h | - | runtime/memory/alloc.h | Preserve |
| 🟢 | `TinyGL/src/ztriangle.h` | 369 | - | - | None | Preserve |

### Subsystem: `engine` (55 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟡 | `engine/include/bootstrap/common_internal.h` | 45 | ctype.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/bpp_api.h` | 158 | - | - | None | Preserve |
| 🟢 | `engine/include/compiler/compiler_ir.h` | 81 | - | - | None | Preserve |
| 🟢 | `engine/include/core/boot.h` | 57 | - | - | None | Preserve |
| 🟢 | `engine/include/core/complex_num.h` | 44 | - | - | None | Preserve |
| 🟢 | `engine/include/core/dialect.h` | 54 | - | - | None | Preserve |
| 🟢 | `engine/include/core/feature_reg.h` | 63 | - | - | None | Preserve |
| 🟢 | `engine/include/core/struct.h` | 63 | - | - | None | Preserve |
| 🟢 | `engine/include/debug/analyzer.h` | 66 | - | - | None | Preserve |
| 🟢 | `engine/include/debug/logger.h` | 50 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bgi.h` | 395 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bgi_autodetect.h` | 35 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bgi_bridge.h` | 31 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bgi_gfx.h` | 57 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bgi_palette.h` | 62 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bgi_text.h` | 34 | - | - | None | Preserve |
| 🟢 | `engine/include/device/bus.h` | 47 | - | - | None | Preserve |
| 🟢 | `engine/include/device/fujinet.h` | 32 | - | - | None | Preserve |
| 🟢 | `engine/include/device/pdf_writer.h` | 58 | - | - | None | Preserve |
| 🟢 | `engine/include/device/usb.h` | 47 | - | - | None | Preserve |
| 🟢 | `engine/include/device/vcon.h` | 56 | - | - | None | Preserve |
| 🟢 | `engine/include/device/vdev.h` | 197 | - | - | None | Preserve |
| 🟢 | `engine/include/device/vprinter.h` | 51 | - | - | None | Preserve |
| 🟢 | `engine/include/docgen/docgen.h` | 45 | - | - | None | Preserve |
| 🔴 | `engine/include/interop/interop_com.h` | 73 | windows.h, ole2.h, oleauto.h | - | hal/hal.h | Preserve |
| 🟢 | `engine/include/interop/interop_core.h` | 37 | - | - | None | Preserve |
| 🟢 | `engine/include/interop/interop_error.h` | 48 | - | - | None | Preserve |
| 🟢 | `engine/include/interop/interop_handle.h` | 52 | - | - | None | Preserve |
| 🟢 | `engine/include/interop/interop_ipc.h` | 38 | - | - | None | Preserve |
| 🟢 | `engine/include/interop/interop_jsonrpc.h` | 34 | - | - | None | Preserve |
| 🟢 | `engine/include/interop/interop_marshal.h` | 94 | - | - | None | Preserve |
| 🟢 | `engine/include/module/arrayext.h` | 31 | - | - | None | Preserve |
| 🟢 | `engine/include/module/module.h` | 70 | - | - | None | Preserve |
| 🟢 | `engine/include/module/regex.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/scope/scope.h` | 89 | - | - | None | Preserve |
| 🟢 | `engine/include/security/security.h` | 67 | - | - | None | Preserve |
| 🟢 | `engine/include/stmt/stmt.h` | 62 | - | - | None | Preserve |
| 🟢 | `engine/include/stmt/stmt_handlers.h` | 397 | - | - | None | Preserve |
| 🟡 | `engine/include/types/config.h` | 211 | strings.h | - | runtime/string/strops.h | Preserve |
| 🟢 | `engine/include/types/errors.h` | 69 | - | - | None | Preserve |
| 🟢 | `engine/include/types/types.h` | 122 | - | - | None | Preserve |
| 🟢 | `engine/include/types/version.h` | 29 | - | - | None | Preserve |
| 🟡 | `engine/lib/collections/map.c` | 47 | stdlib.h, string.h | memory (calloc free) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/lib/memory/mem_arena.c` | 65 | stdlib.h, string.h | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/lib/vdev/vdev_core.c` | 74 | string.h, ctype.h, stdio.h, stdlib.h | memory (calloc free); ctype (toupper) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/compiler/compiler_ir.c` | 76 | stdlib.h | memory (calloc free) | runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/functions/varptr.c` | 118 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/interop/interop_com.c` | 206 | stdlib.h, string.h, stdio.h | memory (calloc free) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/interop/interop_com_register.c` | 52 | stdio.h | string_str (strlen); format (snprintf) | runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/interop/interop_core.c` | 30 | - | - | None | Preserve |
| 🟡 | `engine/src/interop/interop_error.c` | 42 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/interop/interop_handle.c` | 173 | stdlib.h, string.h | memory (calloc free); string_mem (memcpy) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/interop/interop_ipc.c` | 102 | stdlib.h, stdio.h, string.h | stdio_io (fprintf fgets fflush); string_str (strncmp strncpy); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/interop/interop_jsonrpc.c` | 99 | string.h, stdio.h, stdlib.h | string_mem (memset); format (snprintf); conv (atoi); string_str (strlen strncmp strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/interop/interop_marshal.c` | 200 | stdlib.h, string.h, stdio.h | string_mem (memcpy memset); format (snprintf); memory (calloc free); conv (strtoll strtod) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |

### Subsystem: `functions_microlib` (1 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟢 | `engine/include/functions/varptr.h` | 22 | - | - | None | Preserve |

### Subsystem: `libadvanced` (2 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🔴 | `engine/include/device/gfx_internal.h` | 104 | SDL.h | - | hal/hal.h | Preserve |
| 🟡 | `engine/include/device/gfx_primitives_internal.h` | 69 | ctype.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |

### Subsystem: `libboot` (10 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟢 | `engine/src/bootstrap/common/common.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/bootstrap/common/common_boot.c` | 183 | - | memory (calloc free); system_os (getenv); conv (strtod); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | Preserve |
| 🟢 | `engine/src/bootstrap/common/common_reg_funcs.c` | 255 | - | - | None | Preserve |
| 🟢 | `engine/src/bootstrap/common/common_reg_stmts.c` | 292 | - | - | None | Preserve |
| 🟡 | `engine/src/bootstrap/desktop/desktop.c` | 307 | stdio.h, stdlib.h, string.h, ctype.h | stdio_io (printf fprintf fgets fflush); conv (atoi atof); string_str (strlen strcmp strncmp strncat); ctype (isdigit isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/bootstrap/embedded/embedded.c` | 51 | stdio.h, stdlib.h | - | runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/bootstrap/headless/headless.c` | 65 | stdio.h, stdlib.h, string.h | string_str (strcmp strncmp); stdio_io (fprintf) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/bootstrap/iot/iot.c` | 199 | stdio.h, stdlib.h, string.h, ctype.h | stdio_io (printf fprintf fgets fflush); conv (atof); string_str (strlen strcmp strncmp strncat); ctype (isdigit isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/bootstrap/mobile/mobile.c` | 54 | stdio.h, stdlib.h | - | runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/bootstrap/server/server.c` | 155 | stdio.h, stdlib.h, string.h | stdio_io (printf fprintf fgets fflush); conv (atof); string_str (strlen strcmp strncmp strncat) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |

### Subsystem: `libcore` (4 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟡 | `engine/src/docgen/docgen.c` | 271 | stdio.h, stdlib.h, string.h, ctype.h | stdio_io (fprintf fopen fclose fputs remove); format (snprintf); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/category.c` | 188 | string.h, stdio.h, ctype.h, stdlib.h | string_str (strcpy strncpy strstr strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/help.c` | 455 | string.h, stdio.h, ctype.h | stdio_io (fopen fclose fgets); format (snprintf); string_mem (memcpy memset); string_str (strcmp strcpy strncpy strncat strstr strrchr strcasecmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/help_data.h` | 576 | - | stdio_io (remove); math_trig (sin cos tan asin acos atan2 sinh cosh tanh sec csc cot); math_basic (floor); math_alg (sqrt exp log10 log2 hypot) | runtime/math/, hal/io_hal.h | Preserve |

### Subsystem: `libengine` (380 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟢 | `engine/include/eval/ast.h` | 139 | - | - | None | Preserve |
| 🟡 | `engine/include/eval/ast_internal.h` | 121 | ctype.h, math.h, stdlib.h, string.h | string_str (strlen); ctype (isdigit) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/ | Preserve |
| 🟢 | `engine/include/eval/builtins_internal.h` | 30 | - | - | None | Preserve |
| 🟡 | `engine/include/eval/dispatch_internal.h` | 44 | ctype.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/eval/eval.h` | 54 | - | - | None | Preserve |
| 🟡 | `engine/include/eval/eval_expr_internal.h` | 55 | ctype.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/include/eval/eval_internal.h` | 182 | string.h, ctype.h, math.h, time.h, stdlib.h, stdio.h | string_str (strlen strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/eval/functions/bits/and.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/bitcount.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/eqv.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/imp.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/not.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/or.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/readbit.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/resetbit.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/setbit.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/shl.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/shr.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/togglebit.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/bits/xor.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/datetime/pds_datetime.h` | 51 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/filesystem/eof_fn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/filesystem/fid.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/filesystem/freefile.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/filesystem/loc_fn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/filesystem/lof.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/filesystem/typ.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/abs.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/acos.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/angle.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/asin.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/atan2.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/atn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/ceil.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/clamp.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/comp.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/complex_fn.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/cos.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/cosh.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/cot.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/cross.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/csc.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/degrees.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/det.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/dot.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/eps.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/exp.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/financial.h` | 37 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/fix.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/floor.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/fpt.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/hypot.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/inf.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/int.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/lbound.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/lerp.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/log.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/log10.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/log2.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/mag.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/max.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/maxnum.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/min.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/mod.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/pdif.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/pi.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/radians.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/remainder.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/rnd.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/round.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/sec.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/sgn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/sin.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/sinh.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/sqr.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/tan.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/tanh.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/truncate.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/math/ubound.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/ascii_fn.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/ath.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/bin.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/chr.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/cvt.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/ert.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/hex.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/index_fn.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/instr.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/lcase.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/left.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/len.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/ltrim.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/mbf.h` | 32 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/mid.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/num.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/oct.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/pack.h` | 35 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/pick.h` | 26 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/rad.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/right.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/rtrim.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/seg.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/shuffle.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/space.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/spc.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/str.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/str_math.h` | 25 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/string.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/tab.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/tek.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/trim.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/ucase.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/unpack.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/ups.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/val.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/verify_fn.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/string/xlate.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/category.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/clock_num.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/clock_str.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/command_fn.h` | 31 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/csrlin.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/date.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/environ.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/erl.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/err_fn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/fre.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/inkey.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/inp.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/lpos.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/magtape.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/moddir.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/pds_sys.h` | 33 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/peek.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/pos.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/spec_fn.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/sys_fn.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/ticks.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/time.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/time_fn.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/system/timer.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/ui/point_fn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/ui/vbdos_filebox.h` | 30 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/functions/ui/vbdos_fn.h` | 30 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/microplex.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/stack.h` | 33 | - | - | None | Preserve |
| 🟢 | `engine/include/eval/type.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/arrays.h` | 71 | - | - | None | Preserve |
| 🟡 | `engine/include/runtime/arrays_internal.h` | 97 | ctype.h, stdio.h, stdlib.h, string.h | string_mem (memcpy); string_str (strlen); memory (calloc); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/runtime/collections.h` | 35 | - | - | None | Preserve |
| 🟡 | `engine/include/runtime/file.h` | 135 | stdio.h | - | hal/io_hal.h | Preserve |
| 🟡 | `engine/include/runtime/file_internal.h` | 105 | ctype.h, stdio.h, stdlib.h, string.h | string_str (strlen); stdio_io (fread fwrite fgets fputs) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/runtime/funcreg.h` | 79 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/gemini.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/keyword_props.h` | 46 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/map.h` | 56 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/metadata.h` | 96 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/micro_lib_metadata.h` | 113 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/microplex.h` | 39 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/mux.h` | 61 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/num_format.h` | 62 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/override.h` | 37 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/session.h` | 49 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/spec.h` | 60 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/state.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/strings.h` | 72 | - | - | None | Preserve |
| 🟡 | `engine/include/runtime/task.h` | 54 | - | system_os (exit) | None | Preserve |
| 🟡 | `engine/include/runtime/using.h` | 66 | stdio.h | - | hal/io_hal.h | Preserve |
| 🟢 | `engine/include/runtime/variables.h` | 69 | - | - | None | Preserve |
| 🟡 | `engine/include/runtime/variables_internal.h` | 91 | ctype.h, math.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/runtime/vfs.h` | 52 | - | - | None | Preserve |
| 🟢 | `engine/include/runtime/vnet.h` | 44 | - | - | None | Preserve |
| 🟢 | `engine/src/eval/ast.c` | 34 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/ast/ast_create.c` | 348 | math.h, stdio.h, stdlib.h, string.h | string_str (strncpy); memory (calloc free); math_alg (pow) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/ast/ast_eval_expr.c` | 772 | math.h, stdio.h, stdlib.h, string.h | string_str (strlen strstr); string_mem (memset); math_basic (fabs floor ceil); format (snprintf); memory (malloc free); math_alg (sqrt pow exp log); math_trig (sin cos tan atan) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/math/, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/ast/ast_eval_stmt.c` | 545 | stdio.h, stdlib.h, string.h | string_str (strlen strcmp strncasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/eval/ast/ast_parser.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/ast/parser/ast_parse_block.c` | 553 | ctype.h, stdio.h, stdlib.h, string.h | string_mem (memcpy); string_str (strncasecmp); ctype (isalpha isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/ast/parser/ast_parse_expr.c` | 383 | ctype.h, stdio.h, stdlib.h, string.h | string_str (strlen strcat strncat strcasecmp); string_mem (memcpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/ast/parser/ast_parse_stmt.c` | 470 | ctype.h, stdio.h, stdlib.h, string.h | string_str (strlen strcat strncat strchr); string_mem (memcpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/builtins/conversion_fn.c` | 274 | math.h, string.h | math_basic (round); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | Preserve |
| 🟡 | `engine/src/eval/builtins/math_fn.c` | 282 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/builtins/string_fn.c` | 221 | string.h | string_str (strcmp strncmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/builtins/sys_fn.c` | 243 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/dispatch.c` | 35 | - | memory (calloc); stdio_io (fopen fclose fread fseek ftell) | runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/dispatch/dispatch_call.c` | 86 | - | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/eval/dispatch/dispatch_check.c` | 323 | - | string_str (strcmp strncmp strncpy strcat); ctype (toupper) | runtime/string/strops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/eval/dispatch/dispatch_special.c` | 265 | - | string_str (strcmp); string_mem (memcpy) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/eval.c` | 213 | - | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/eval/eval_builtins.c` | 65 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/eval/expression/eval_array.c` | 180 | - | string_str (strlen strncpy); format (snprintf) | runtime/string/strops.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/eval/expression/eval_ident.c` | 591 | - | string_mem (memcpy memset); format (snprintf); memory (calloc free); string_str (strlen strcmp strcpy strncpy strchr strcasecmp); ctype (isdigit toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/eval/expression/eval_new.c` | 99 | - | string_mem (memcpy memset); format (snprintf) | runtime/string/memops.h, runtime/format/snprintf.h | Preserve |
| 🟢 | `engine/src/eval/expression/eval_op.c` | 160 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/bits/and.c` | 59 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/bitcount.c` | 53 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/eqv.c` | 49 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/imp.c` | 49 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/not.c` | 48 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/or.c` | 59 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/readbit.c` | 46 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/resetbit.c` | 46 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/setbit.c` | 46 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/shl.c` | 46 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/shr.c` | 46 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/togglebit.c` | 46 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/bits/xor.c` | 49 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/datetime/pds_datetime.c` | 280 | math.h, stdio.h, stdlib.h, string.h, time.h, ctype.h | time (time localtime); format (sscanf); math_basic (floor); string_str (strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/eval/functions/filesystem/eof_fn.c` | 42 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/filesystem/fid.c` | 76 | string.h | string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟢 | `engine/src/eval/functions/filesystem/freefile.c` | 52 | - | - | None | Preserve |
| 🟢 | `engine/src/eval/functions/filesystem/loc_fn.c` | 42 | - | - | None | Preserve |
| 🟢 | `engine/src/eval/functions/filesystem/lof.c` | 42 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/filesystem/typ.c` | 50 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/eval/functions/math/abs.c` | 50 | math.h, string.h | string_str (strcmp); math_basic (fabs abs); math_alg (hypot) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/abs.c` |
| 🟡 | `engine/src/eval/functions/math/acos.c` | 53 | math.h, string.h | math_trig (acos); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/acos.c` |
| 🟡 | `engine/src/eval/functions/math/angle.c` | 62 | math.h, string.h | math_trig (atan2); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/geometry/angle.c` |
| 🟡 | `engine/src/eval/functions/math/asin.c` | 53 | math.h, string.h | math_trig (asin); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/asin.c` |
| 🟡 | `engine/src/eval/functions/math/atan2.c` | 46 | math.h, string.h | math_trig (atan2); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/atan2.c` |
| 🟡 | `engine/src/eval/functions/math/atn.c` | 46 | math.h, string.h | math_trig (atan); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/atn.c` |
| 🟡 | `engine/src/eval/functions/math/ceil.c` | 47 | math.h, string.h | math_basic (ceil); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/ceil.c` |
| 🟡 | `engine/src/eval/functions/math/clamp.c` | 52 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/interpolation/clamp.c` |
| 🟡 | `engine/src/eval/functions/math/comp.c` | 53 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/basic/comp.c` |
| 🟡 | `engine/src/eval/functions/math/complex_fn.c` | 214 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/basic/complex_fn.c` |
| 🟡 | `engine/src/eval/functions/math/cos.c` | 46 | math.h, string.h | math_trig (cos); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/cos.c` |
| 🟡 | `engine/src/eval/functions/math/cosh.c` | 47 | math.h, string.h | math_trig (cosh); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/cosh.c` |
| 🟡 | `engine/src/eval/functions/math/cot.c` | 53 | math.h, string.h | math_trig (tan cot); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/cot.c` |
| 🟡 | `engine/src/eval/functions/math/cross.c` | 44 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/linear_algebra/cross.c` |
| 🟡 | `engine/src/eval/functions/math/csc.c` | 53 | math.h, string.h | math_trig (sin csc); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/csc.c` |
| 🟡 | `engine/src/eval/functions/math/degrees.c` | 54 | math.h, string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/geometry/degrees.c` |
| 🟡 | `engine/src/eval/functions/math/det.c` | 103 | string.h | - | runtime/string/memops.h | `engine/src/eval/functions/math/linear_algebra/det.c` |
| 🟡 | `engine/src/eval/functions/math/dot.c` | 70 | string.h | - | runtime/string/memops.h | `engine/src/eval/functions/math/linear_algebra/dot.c` |
| 🟡 | `engine/src/eval/functions/math/eps.c` | 53 | math.h, string.h | math_basic (fabs); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/constants/eps.c` |
| 🟡 | `engine/src/eval/functions/math/exp.c` | 47 | math.h, string.h | string_str (strcmp); math_alg (exp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/exp.c` |
| 🟡 | `engine/src/eval/functions/math/financial.c` | 286 | string.h, math.h | math_basic (fabs); math_alg (pow log) | runtime/string/memops.h, runtime/math/ | `engine/src/eval/functions/math/basic/financial.c` |
| 🟡 | `engine/src/eval/functions/math/fix.c` | 46 | math.h, string.h | math_basic (floor ceil); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/fix.c` |
| 🟡 | `engine/src/eval/functions/math/floor.c` | 46 | math.h, string.h | math_basic (floor); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/floor.c` |
| 🟡 | `engine/src/eval/functions/math/fpt.c` | 70 | math.h, string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/fpt.c` |
| 🟡 | `engine/src/eval/functions/math/hypot.c` | 56 | math.h, string.h | string_str (strcmp); math_alg (sqrt hypot) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/hypot.c` |
| 🟡 | `engine/src/eval/functions/math/inf.c` | 53 | math.h, string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/constants/inf.c` |
| 🟡 | `engine/src/eval/functions/math/int.c` | 46 | math.h, string.h | math_basic (floor); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/int.c` |
| 🟢 | `engine/src/eval/functions/math/lbound.c` | 36 | - | - | None | `engine/src/eval/functions/math/arrays/lbound.c` |
| 🟡 | `engine/src/eval/functions/math/lerp.c` | 49 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/interpolation/lerp.c` |
| 🟡 | `engine/src/eval/functions/math/log.c` | 52 | math.h, string.h | string_str (strcmp); math_alg (log) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/log.c` |
| 🟡 | `engine/src/eval/functions/math/log10.c` | 53 | math.h, string.h | string_str (strcmp); math_alg (log10) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/log10.c` |
| 🟡 | `engine/src/eval/functions/math/log2.c` | 52 | math.h, string.h | string_str (strcmp); math_alg (log2) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/log2.c` |
| 🟡 | `engine/src/eval/functions/math/mag.c` | 61 | math.h, string.h | math_basic (fabs); math_alg (hypot) | runtime/string/memops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/mag.c` |
| 🟡 | `engine/src/eval/functions/math/max.c` | 60 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/interpolation/max.c` |
| 🟡 | `engine/src/eval/functions/math/maxnum.c` | 49 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/constants/maxnum.c` |
| 🟡 | `engine/src/eval/functions/math/min.c` | 60 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/interpolation/min.c` |
| 🟡 | `engine/src/eval/functions/math/mod.c` | 54 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/basic/mod.c` |
| 🟡 | `engine/src/eval/functions/math/pdif.c` | 47 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/basic/pdif.c` |
| 🟡 | `engine/src/eval/functions/math/pi.c` | 51 | math.h, string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/constants/pi.c` |
| 🟡 | `engine/src/eval/functions/math/radians.c` | 54 | math.h, string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/geometry/radians.c` |
| 🟡 | `engine/src/eval/functions/math/remainder.c` | 56 | math.h, string.h | math_basic (floor); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/remainder.c` |
| 🟡 | `engine/src/eval/functions/math/rnd.c` | 55 | stdlib.h, string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | `engine/src/eval/functions/math/basic/rnd.c` |
| 🟡 | `engine/src/eval/functions/math/round.c` | 62 | math.h, string.h | string_str (strcmp); math_basic (round); math_alg (pow) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/round.c` |
| 🟡 | `engine/src/eval/functions/math/sec.c` | 53 | math.h, string.h | math_trig (cos sec); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/sec.c` |
| 🟡 | `engine/src/eval/functions/math/sgn.c` | 48 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/math/basic/sgn.c` |
| 🟡 | `engine/src/eval/functions/math/sin.c` | 46 | math.h, string.h | math_trig (sin); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/sin.c` |
| 🟡 | `engine/src/eval/functions/math/sinh.c` | 47 | math.h, string.h | math_trig (sinh); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/sinh.c` |
| 🟡 | `engine/src/eval/functions/math/sqr.c` | 63 | math.h, string.h | string_str (strcmp); math_trig (sin cos atan2); math_alg (sqrt hypot) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/algebra/sqr.c` |
| 🟡 | `engine/src/eval/functions/math/tan.c` | 46 | math.h, string.h | math_trig (tan); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/tan.c` |
| 🟡 | `engine/src/eval/functions/math/tanh.c` | 47 | math.h, string.h | math_trig (tanh); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/trig/tanh.c` |
| 🟡 | `engine/src/eval/functions/math/truncate.c` | 60 | math.h, string.h | string_str (strcmp); math_basic (trunc); math_alg (pow) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/ | `engine/src/eval/functions/math/basic/truncate.c` |
| 🟢 | `engine/src/eval/functions/math/ubound.c` | 36 | - | - | None | `engine/src/eval/functions/math/arrays/ubound.c` |
| 🟡 | `engine/src/eval/functions/string/ascii_fn.c` | 55 | string.h | - | runtime/string/memops.h | `engine/src/eval/functions/string/format/ascii_fn.c` |
| 🟡 | `engine/src/eval/functions/string/ath.c` | 130 | string.h, stdlib.h, stdio.h, ctype.h | format (snprintf); memory (malloc free); string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | `engine/src/eval/functions/string/format/ath.c` |
| 🟡 | `engine/src/eval/functions/string/bin.c` | 69 | stdio.h | - | hal/io_hal.h | `engine/src/eval/functions/string/conversion/bin.c` |
| 🟡 | `engine/src/eval/functions/string/chr.c` | 54 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/conversion/chr.c` |
| 🟡 | `engine/src/eval/functions/string/cvt.c` | 330 | string.h, stdlib.h, ctype.h | string_mem (memcpy); string_str (strcmp); memory (malloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/cvt.c` |
| 🟡 | `engine/src/eval/functions/string/ert.c` | 95 | string.h | string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/format/ert.c` |
| 🟡 | `engine/src/eval/functions/string/hex.c` | 101 | stdio.h, string.h, ctype.h, stdlib.h | format (snprintf); memory (malloc free); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | `engine/src/eval/functions/string/conversion/hex.c` |
| 🟡 | `engine/src/eval/functions/string/index_fn.c` | 78 | string.h | string_str (strstr) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/format/index_fn.c` |
| 🟡 | `engine/src/eval/functions/string/instr.c` | 86 | string.h | string_str (strcmp strstr) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/search/instr.c` |
| 🟡 | `engine/src/eval/functions/string/lcase.c` | 62 | ctype.h, stdlib.h, string.h | memory (calloc free); ctype (tolower) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/manipulation/lcase.c` |
| 🟡 | `engine/src/eval/functions/string/left.c` | 64 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/manipulation/left.c` |
| 🟡 | `engine/src/eval/functions/string/len.c` | 48 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/format/len.c` |
| 🟡 | `engine/src/eval/functions/string/ltrim.c` | 52 | ctype.h, string.h | ctype (isspace) | runtime/string/memops.h, runtime/ctype/ctype.h | `engine/src/eval/functions/string/manipulation/ltrim.c` |
| 🟡 | `engine/src/eval/functions/string/mbf.c` | 169 | string.h, math.h, stdlib.h | math_basic (fabs); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/ | `engine/src/eval/functions/string/format/mbf.c` |
| 🟡 | `engine/src/eval/functions/string/mid.c` | 88 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/manipulation/mid.c` |
| 🟡 | `engine/src/eval/functions/string/num.c` | 81 | stdio.h, string.h, stdlib.h | format (snprintf); conv (atoi); string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | `engine/src/eval/functions/string/format/num.c` |
| 🟡 | `engine/src/eval/functions/string/oct.c` | 47 | stdio.h, string.h | string_str (strlen); format (snprintf) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | `engine/src/eval/functions/string/conversion/oct.c` |
| 🟡 | `engine/src/eval/functions/string/pack.c` | 54 | stdio.h, stdlib.h, string.h | memory (free) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | `engine/src/eval/functions/string/format/pack.c` |
| 🟡 | `engine/src/eval/functions/string/pick.c` | 314 | string.h, stdlib.h, stdio.h | string_str (strlen strstr strchr) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | `engine/src/eval/functions/string/format/pick.c` |
| 🟡 | `engine/src/eval/functions/string/rad.c` | 76 | string.h | string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/format/rad.c` |
| 🟡 | `engine/src/eval/functions/string/right.c` | 65 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/manipulation/right.c` |
| 🟡 | `engine/src/eval/functions/string/rtrim.c` | 52 | ctype.h, string.h | ctype (isspace) | runtime/string/memops.h, runtime/ctype/ctype.h | `engine/src/eval/functions/string/manipulation/rtrim.c` |
| 🟡 | `engine/src/eval/functions/string/seg.c` | 64 | string.h | - | runtime/string/memops.h | `engine/src/eval/functions/string/format/seg.c` |
| 🟡 | `engine/src/eval/functions/string/shuffle.c` | 87 | stdlib.h, string.h | memory (calloc free); string_mem (memcpy) | runtime/string/memops.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/shuffle.c` |
| 🟡 | `engine/src/eval/functions/string/space.c` | 61 | stdlib.h, string.h | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/space.c` |
| 🟡 | `engine/src/eval/functions/string/spc.c` | 61 | stdlib.h, string.h | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/spc.c` |
| 🟡 | `engine/src/eval/functions/string/str.c` | 50 | string.h | string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/conversion/str.c` |
| 🟡 | `engine/src/eval/functions/string/str_math.c` | 475 | stdio.h, stdlib.h, string.h, ctype.h | string_str (strlen strchr); memory (malloc calloc free); string_mem (memcpy); format (snprintf); ctype (isdigit isspace); conv (atof) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | `engine/src/eval/functions/string/format/str_math.c` |
| 🟡 | `engine/src/eval/functions/string/string.c` | 71 | stdlib.h, string.h | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/string.c` |
| 🟡 | `engine/src/eval/functions/string/tab.c` | 82 | stdlib.h, string.h | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/tab.c` |
| 🟡 | `engine/src/eval/functions/string/tek.c` | 98 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/format/tek.c` |
| 🟡 | `engine/src/eval/functions/string/trim.c` | 56 | ctype.h, string.h | ctype (isspace) | runtime/string/memops.h, runtime/ctype/ctype.h | `engine/src/eval/functions/string/manipulation/trim.c` |
| 🟡 | `engine/src/eval/functions/string/ucase.c` | 62 | ctype.h, stdlib.h, string.h | memory (calloc free); ctype (toupper) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/manipulation/ucase.c` |
| 🟡 | `engine/src/eval/functions/string/unpack.c` | 72 | stdio.h, stdlib.h, string.h | string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | `engine/src/eval/functions/string/format/unpack.c` |
| 🟡 | `engine/src/eval/functions/string/ups.c` | 65 | string.h, ctype.h, stdlib.h | memory (malloc free); ctype (toupper) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/ups.c` |
| 🟡 | `engine/src/eval/functions/string/val.c` | 48 | stdlib.h, string.h | string_str (strcmp); conv (strtod) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | `engine/src/eval/functions/string/conversion/val.c` |
| 🟡 | `engine/src/eval/functions/string/verify_fn.c` | 71 | string.h | string_str (strlen strcmp strchr) | runtime/string/memops.h, runtime/string/strops.h | `engine/src/eval/functions/string/format/verify_fn.c` |
| 🟡 | `engine/src/eval/functions/string/xlate.c` | 66 | string.h, stdlib.h | memory (malloc free) | runtime/string/memops.h, runtime/memory/alloc.h | `engine/src/eval/functions/string/format/xlate.c` |
| 🟡 | `engine/src/eval/functions/system/category.c` | 102 | string.h, stdlib.h, ctype.h | string_str (strlen strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/clock_num.c` | 60 | time.h, string.h | time (time clock) | runtime/string/memops.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/clock_str.c` | 57 | stdio.h, time.h, string.h | time (time); format (snprintf); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/command_fn.c` | 53 | string.h | string_str (strlen strncpy); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟢 | `engine/src/eval/functions/system/csrlin.c` | 47 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/system/date.c` | 119 | ctype.h, stdio.h, string.h, time.h | time (time strftime mktime); format (snprintf); string_str (strlen strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/environ.c` | 50 | string.h | string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟢 | `engine/src/eval/functions/system/erl.c` | 43 | - | - | None | Preserve |
| 🟢 | `engine/src/eval/functions/system/err_fn.c` | 43 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/system/fre.c` | 53 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/inkey.c` | 54 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/inp.c` | 67 | string.h | - | runtime/string/memops.h | Preserve |
| 🟢 | `engine/src/eval/functions/system/lpos.c` | 41 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/system/magtape.c` | 41 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/moddir.c` | 47 | string.h | string_str (strlen); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🔴 | `engine/src/eval/functions/system/pds_sys.c` | 173 | stdio.h, stdlib.h, string.h, windows.h, dirent.h, fnmatch.h | string_str (strlen strncpy); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/peek.c` | 88 | string.h | - | runtime/string/memops.h | Preserve |
| 🟢 | `engine/src/eval/functions/system/pos.c` | 47 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/system/spec_fn.c` | 35 | string.h | - | runtime/string/memops.h | Preserve |
| 🔴 | `engine/src/eval/functions/system/sys_fn.c` | 86 | string.h, stdlib.h, windows.h, unistd.h | system_os (system) | runtime/string/memops.h, runtime/memory/alloc.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/ticks.c` | 62 | stdio.h, string.h | string_str (strlen); format (snprintf) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/time.c` | 90 | stdio.h, string.h, time.h | time (time); format (snprintf); string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/time_fn.c` | 78 | time.h, string.h | time (time clock localtime) | runtime/string/memops.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/eval/functions/system/timer.c` | 61 | math.h, string.h, time.h | time (time); string_str (strcmp); math_basic (fmod floor) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/, runtime/time/calendar.h | Preserve |
| 🟢 | `engine/src/eval/functions/ui/point_fn.c` | 48 | - | - | None | Preserve |
| 🟡 | `engine/src/eval/functions/ui/vbdos_filebox.c` | 95 | string.h, stdio.h | string_str (strlen strncpy); format (snprintf) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/functions/ui/vbdos_fn.c` | 134 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strlen strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/helpers.c` | 299 | string.h, ctype.h, math.h, stdio.h | string_mem (memcpy memset); format (snprintf); string_str (strlen); math_basic (fabs ceil round); math_alg (pow log10) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/math/, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/eval/microplex.c` | 53 | stdlib.h, string.h | memory (malloc free); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/eval/ops.c` | 498 | string.h, math.h | string_str (strlen strcmp); string_mem (memcpy memset); math_basic (floor); format (snprintf); memory (malloc free); math_alg (sqrt pow exp log hypot); math_trig (sin cos atan2) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/math/, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/eval/rpn.c` | 478 | - | string_mem (memcpy memset); format (snprintf); string_str (strlen strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h | Preserve |
| 🟢 | `engine/src/eval/stack.c` | 44 | - | - | None | Preserve |
| 🟢 | `engine/src/eval/type.c` | 31 | - | - | None | Preserve |
| 🟡 | `engine/src/module/arrayext.c` | 173 | stdio.h, stdlib.h, string.h | string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/module/mathext.c` | 94 | math.h | math_alg (sqrt pow) | runtime/math/ | Preserve |
| 🟡 | `engine/src/module/module.c` | 193 | stdio.h, stdlib.h, string.h, ctype.h | string_str (strlen); stdio_io (fopen fclose fgets); string_mem (memset); format (snprintf); ctype (isdigit isspace toupper); conv (atof) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/module/regex.c` | 94 | string.h, stdlib.h | string_mem (memset); string_str (strlen); memory (free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟢 | `engine/src/module/regex.h` | 24 | - | - | None | Preserve |
| 🟡 | `engine/src/parser/parser.c` | 90 | stdlib.h, string.h | memory (calloc realloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/runtime/array_sort.c` | 105 | stdlib.h, string.h | string_mem (memset); string_str (strcmp); sort (qsort) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/sort/qsort.h | Preserve |
| 🟢 | `engine/src/runtime/arrays.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/runtime/arrays/arr_access.c` | 197 | - | string_mem (memcpy); string_str (strcmp strncpy strcasecmp); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/runtime/arrays/arr_core.c` | 317 | - | string_mem (memcpy memset); string_str (strlen strcmp); memory (calloc realloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/runtime/arrays/arr_persist.c` | 314 | - | stdio_io (fread fwrite); string_mem (memcpy); memory (calloc free); string_str (strlen strcmp strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/crypto.c` | 315 | string.h, stdio.h, stdlib.h | string_mem (memcpy memset); format (snprintf); memory (calloc free); string_str (strlen strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/runtime/file.c` | 14 | - | - | None | Preserve |
| 🟡 | `engine/src/runtime/file/file_channel.c` | 352 | - | stdio_io (fopen fclose remove); string_mem (memcpy memset); memory (calloc free); string_str (strcmp strncpy strchr strcasecmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/file/file_io.c` | 356 | - | memory (malloc); stdio_io (fread fwrite fflush); format (vsnprintf); string_mem (memcpy memchr); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/file/file_record.c` | 161 | - | stdio_io (fseek ftell feof) | hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/file/file_txn.c` | 251 | - | memory (calloc free); stdio_io (fopen fclose fread fwrite fseek fflush remove); format (snprintf); string_mem (memcpy memset); string_str (strcmp strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/funcreg.c` | 102 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/runtime/gemini.c` | 31 | stdlib.h, stdio.h | memory (calloc); format (snprintf) | runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/keyword_props.c` | 97 | string.h, ctype.h, stdio.h, stdlib.h | string_str (strncpy strcasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/map.c` | 190 | stdlib.h, string.h | string_mem (memcpy memset); string_str (strlen strcasecmp); memory (calloc realloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/runtime/map_serialize.c` | 659 | stdlib.h, string.h, ctype.h, stdio.h | string_str (strlen strncmp); memory (calloc realloc free); string_mem (memcpy); format (snprintf); ctype (isdigit isspace); conv (strtod) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/metadata.c` | 590 | string.h, ctype.h, stdio.h | string_mem (memcpy); format (snprintf); string_str (strlen strstr strcasecmp strncasecmp); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/micro_lib_metadata.c` | 205 | ctype.h, stdio.h, stdlib.h, string.h | string_mem (memcpy memset); format (snprintf); memory (malloc calloc realloc free); string_str (strcmp strcpy); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/microplex.c` | 53 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/runtime/mux.c` | 204 | stdlib.h, string.h, stdio.h | memory (calloc); string_mem (memcpy memset) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/num_format.c` | 130 | stdio.h, math.h, string.h | math_basic (fabs); format (snprintf) | runtime/string/memops.h, runtime/math/, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/override.c` | 90 | string.h, ctype.h, stdio.h, stdlib.h | string_str (strncpy strcasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/print_using.c` | 236 | stdio.h, stdlib.h, string.h, ctype.h | stdio_io (fprintf fflush); string_mem (memset); string_str (strncpy strncasecmp); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/session.c` | 85 | string.h, time.h, stdlib.h | string_mem (memset); string_str (strncpy); time (time) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/runtime/spec.c` | 325 | stdio.h, string.h, stdlib.h, ctype.h | stdio_io (fopen fclose fgets); format (snprintf); string_mem (memcpy memset memmove); string_str (strlen strncmp strcpy strchr strrchr strcasecmp); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/state.c` | 232 | stdio.h, stdlib.h, string.h | memory (calloc free); stdio_io (fopen fclose fread fwrite); format (snprintf); string_mem (memcpy memset memcmp); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/string_ext.c` | 506 | string.h, stdio.h, stdlib.h | string_mem (memcpy memset); format (snprintf); memory (calloc realloc free); string_str (strlen strstr) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/strings.c` | 423 | string.h, stdlib.h | string_mem (memcpy); string_str (strlen); memory (malloc calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/runtime/task.c` | 159 | stdio.h, stdlib.h, string.h | string_mem (memset); format (snprintf) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/using_engine.c` | 467 | stdio.h, stdlib.h, string.h, ctype.h, math.h | string_mem (memcpy memset); format (snprintf); conv (strtol strtod); string_str (strlen strcmp strncmp strcpy strncpy strcat strstr strchr) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/variables.c` | 77 | - | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/runtime/variables/var_lookup.c` | 464 | - | string_mem (memcpy); format (snprintf); memory (calloc free); string_str (strlen strcmp strncpy strchr); ctype (isalpha toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟢 | `engine/src/runtime/variables/var_magic.c` | 24 | - | - | None | Preserve |
| 🟡 | `engine/src/runtime/variables/var_scope.c` | 491 | - | string_str (strlen strcmp strncmp strncpy strchr); stdio_io (fread fwrite); memory (calloc free); string_mem (memset); math_basic (fabs); format (snprintf); ctype (isalpha toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/vfs.c` | 219 | stdio.h, stdlib.h, string.h | string_mem (memcpy); format (snprintf); memory (calloc free); string_str (strlen strcpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/runtime/vnet.c` | 346 | stdio.h, stdlib.h, string.h | string_mem (memset); string_str (strlen strcmp strcpy strncpy); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/scope/scope.c` | 226 | string.h, ctype.h, stdlib.h | string_str (strncpy strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |

### Subsystem: `libflex` (3 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟡 | `engine/src/statements/dialect/alias.c` | 267 | string.h, ctype.h, stdio.h, stdlib.h | stdio_io (fprintf fopen fclose); string_mem (memcpy memset); string_str (strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/override.c` | 133 | string.h, ctype.h, stdio.h, stdlib.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/scope.c` | 250 | string.h, ctype.h, stdio.h, stdlib.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |

### Subsystem: `libkernel` (90 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟢 | `engine/include/bios/bios.h` | 236 | - | - | None | Preserve |
| 🟢 | `engine/include/bios/bios_at.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/bios/bios_cpu8086.h` | 48 | - | - | None | Preserve |
| 🟢 | `engine/include/bios/bios_hal.h` | 68 | - | - | None | Preserve |
| 🟢 | `engine/include/bios/bios_jr.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/bios/bios_pc.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/bios/bios_xt.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/lexer/lexer.h` | 609 | - | - | None | Preserve |
| 🟢 | `engine/include/lexer/lexer_internal.h` | 58 | - | - | None | Preserve |
| 🟢 | `engine/include/memory/memory.h` | 111 | - | - | None | Preserve |
| 🟢 | `engine/include/memory/segmented_mem.h` | 37 | - | - | None | Preserve |
| 🟡 | `engine/include/vm/events_internal.h` | 60 | ctype.h, stdio.h, stdlib.h, string.h, time.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/include/vm/exec_control_internal.h` | 47 | ctype.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/include/vm/exec_internal.h` | 102 | ctype.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟢 | `engine/include/vm/host.h` | 42 | - | - | None | Preserve |
| 🟡 | `engine/include/vm/vm.h` | 450 | stdio.h | time (time) | runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/include/vm/vm_internal.h` | 344 | stdlib.h, string.h, time.h, ctype.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/bios/bios.c` | 328 | stdlib.h, string.h | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/bios/bios_at.c` | 97 | string.h, time.h | time (time localtime_s localtime_r) | runtime/string/memops.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/bios/bios_cpu8086.c` | 83 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/bios/bios_hal_vm.c` | 72 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/bios/bios_int10.c` | 82 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/bios/bios_int13.c` | 67 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/bios/bios_int16.c` | 68 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/bios/bios_int1a.c` | 82 | time.h | time (time localtime_s localtime_r) | runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/bios/bios_jr.c` | 59 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/bios/bios_pc.c` | 418 | string.h, time.h | string_mem (memset); string_str (strlen); time (time localtime_s localtime_r) | runtime/string/memops.h, runtime/string/strops.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/bios/bios_xt.c` | 108 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/core/bpp_api.c` | 274 | stdlib.h, string.h | string_mem (memcpy memset); string_str (strlen strdup); memory (free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/core/complex_num.c` | 106 | math.h | math_trig (sin cos atan2 sinh cosh); math_alg (sqrt exp log) | runtime/math/ | Preserve |
| 🟡 | `engine/src/core/dialect.c` | 63 | stdio.h, stdlib.h, string.h | memory (calloc free); format (snprintf) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/core/feature_reg.c` | 129 | string.h, ctype.h, stdio.h | string_mem (memset); string_str (strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/core/struct.c` | 188 | string.h, ctype.h, stdlib.h, stdio.h | string_mem (memcpy memset); format (snprintf); string_str (strlen strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/debug/analyzer.c` | 215 | stdio.h, stdlib.h, string.h, ctype.h | string_mem (memset); format (snprintf); string_str (strncpy strstr strncasecmp); ctype (isdigit isspace toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/src/debug/dap_server.c` | 104 | stdio.h, stdlib.h, string.h, winsock2.h, ws2tcpip.h, sys/socket.h, netinet/in.h, arpa/inet.h, unistd.h, fcntl.h | string_str (strlen); format (snprintf) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/debug/logger.c` | 180 | stdio.h, stdlib.h, string.h, time.h | time (time); format (snprintf); string_mem (memset); stdio_io (fprintf fopen fclose fwrite fflush) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/src/device/bgi/aalib/aalib.c` | 133 | stdlib.h, string.h, stdio.h, windows.h, sys/ioctl.h, unistd.h | stdio_io (printf fwrite fflush putchar); string_mem (memset); memory (calloc free) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟢 | `engine/src/device/bgi/aalib/include/aalib.h` | 51 | - | - | None | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_autodetect.c` | 72 | stdio.h, string.h | string_mem (memset); format (snprintf) | runtime/string/memops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_core.c` | 449 | string.h, stdlib.h | memory (calloc free); string_mem (memcpy memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_font.c` | 253 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_gfx.c` | 217 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_modes.c` | 122 | string.h, stdio.h | string_mem (memset); format (snprintf) | runtime/string/memops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_palette.c` | 131 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/device/bgi/bgi_raster.c` | 395 | stdlib.h, string.h, math.h | string_mem (memcpy); memory (calloc free); math_trig (sin cos); math_basic (abs); math_alg (sqrt) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/ | Preserve |
| 🟡 | `engine/src/device/bgi_bridge.c` | 38 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/device/bgi_text.c` | 177 | stdio.h, string.h | string_mem (memset); stdio_io (printf fflush) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/bus.c` | 276 | stdio.h, stdlib.h, string.h, time.h | time (clock); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/console.c` | 223 | stdio.h, string.h | stdio_io (printf fgets fputs fflush putchar getchar); string_mem (memset); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/fujinet.c` | 528 | stdio.h, stdlib.h, string.h, time.h | string_str (strlen strcmp strncmp strncpy strchr); time (time); memory (calloc free); string_mem (memcpy memset); format (snprintf); stdio_io (fprintf fopen fclose fgets); conv (strtol) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/gfx.c` | 498 | ctype.h, stdio.h, stdlib.h, string.h | sdl (SDL_CreateWindow SDL_CreateRenderer); string_mem (memcpy memset); format (snprintf); memory (calloc free) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/gfx/gfx_audio.c` | 658 | ctype.h, math.h, stdio.h, stdlib.h, string.h | string_str (strlen); memory (malloc free); string_mem (memcpy memset); ctype (isdigit isspace toupper); math_alg (pow); sdl (SDL_OpenAudioDevice); math_trig (sin) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/gfx/gfx_palette.c` | 101 | string.h | string_mem (memcpy) | runtime/string/memops.h | Preserve |
| 🟢 | `engine/src/device/gfx/gfx_primitives.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/device/gfx/gfx_tui.c` | 502 | ctype.h, stdio.h, stdlib.h, string.h | memory (free); stdio_io (printf fflush); sdl (SDL_RenderPresent); string_mem (memcpy memset memmove) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/gfx/primitives/gfx_draw_core.c` | 253 | - | string_mem (memcpy memset); memory (calloc free); math_basic (abs) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/math/ | Preserve |
| 🟡 | `engine/src/device/gfx/primitives/gfx_stmt_retro.c` | 387 | - | string_mem (memset); format (snprintf); string_str (strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/device/gfx/primitives/gfx_stmt_shapes.c` | 380 | - | stdio_io (printf fflush); string_mem (memcpy memset); string_str (strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/mux.c` | 31 | string.h, stdlib.h | - | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/device/pdf_writer.c` | 322 | stdio.h, stdlib.h, string.h | memory (malloc calloc realloc free); stdio_io (fprintf fopen fclose fwrite fputs ftell); format (snprintf); string_mem (memcpy); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/vcon.c` | 307 | stdio.h, stdlib.h, string.h, ctype.h | string_mem (memcpy memset); format (snprintf); memory (calloc free); conv (strtol); ctype (isdigit isalpha) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/vdev.c` | 334 | string.h, ctype.h, stdio.h, stdlib.h | memory (calloc free); stdio_io (fgets); format (vsnprintf); string_mem (memset); ctype (toupper) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/device/vprinter.c` | 277 | stdio.h, stdlib.h, string.h, ctype.h | string_mem (memset); format (snprintf); memory (calloc free); string_str (strlen strncpy strrchr strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/lexer/lexer.c` | 445 | ctype.h, stdlib.h, string.h | string_mem (memcpy memset); string_str (strncasecmp); memory (calloc free); ctype (isdigit isalpha isalnum isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟢 | `engine/src/lexer/lexer_internal.h` | 58 | - | - | None | Preserve |
| 🟡 | `engine/src/lexer/scan_keyword.c` | 512 | ctype.h, stdlib.h, string.h | string_str (strlen strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/lexer/scan_number.c` | 193 | ctype.h, stdlib.h | ctype (isdigit isalnum); conv (strtod) | runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | Preserve |
| 🟡 | `engine/src/lexer/scan_string.c` | 218 | ctype.h, string.h | string_str (strlen); ctype (isalnum isspace toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/memory/mem_system.c` | 423 | stdlib.h, string.h, stdio.h | format (snprintf); memory (malloc calloc realloc free); string_str (strlen strcpy strncpy strstr) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/memory/segmented_mem.c` | 163 | stdlib.h, string.h | memory (calloc realloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/security/security.c` | 353 | stdio.h, string.h, ctype.h | stdio_io (printf); string_mem (memset); string_str (strlen strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/vm/context.c` | 678 | stdlib.h, string.h, time.h, ctype.h | string_mem (memset); string_str (strlen strncpy strcasecmp); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/vm/control.c` | 357 | stdlib.h, string.h, time.h, ctype.h | string_str (strlen strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/vm/data.c` | 128 | stdlib.h, string.h, time.h, ctype.h | string_mem (memset); memory (calloc realloc free); ctype (isspace) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/vm/error.c` | 171 | stdlib.h, string.h, time.h, ctype.h | string_str (strlen); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |
| 🟢 | `engine/src/vm/events.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/vm/events/events_alarm.c` | 450 | - | string_str (strcmp strncpy strcasecmp); time (time); memory (realloc); string_mem (memset); format (snprintf sscanf); ctype (isdigit toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/time/calendar.h | Preserve |
| 🟡 | `engine/src/vm/events/events_poll.c` | 415 | - | time (time); format (snprintf); string_str (strcmp strncpy) | runtime/string/strops.h, runtime/format/snprintf.h, runtime/time/calendar.h | Preserve |
| 🟢 | `engine/src/vm/events/events_trap.c` | 143 | - | - | None | Preserve |
| 🟢 | `engine/src/vm/exec.c` | 22 | - | - | None | Preserve |
| 🟡 | `engine/src/vm/exec/control/exec_line.c` | 161 | - | string_mem (memcpy memset); string_str (strlen strstr strcasecmp); memory (malloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟢 | `engine/src/vm/exec/control/exec_postfix.c` | 51 | - | - | None | Preserve |
| 🟡 | `engine/src/vm/exec/control/exec_stmt.c` | 734 | - | string_mem (memcpy memset memchr); format (snprintf); memory (calloc free); string_str (strlen strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟢 | `engine/src/vm/exec/exec_control.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/vm/exec/exec_dispatch.c` | 717 | ctype.h, stdio.h, stdlib.h, string.h | stdio_io (fopen fclose fgets); format (snprintf); string_mem (memcpy memset memmove); string_str (strlen strncpy strcasecmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/vm/exec/exec_interrupt.c` | 301 | stdio.h, stdlib.h, string.h | stdio_io (fgets); format (snprintf); string_mem (memset); string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/vm/host.c` | 114 | stdio.h, string.h | string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/vm/math.c` | 15 | - | - | None | Preserve |
| 🟡 | `engine/src/vm/stack.c` | 531 | stdlib.h, string.h | string_mem (memset); string_str (strlen strncpy strcasecmp); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/vm/vm_internal.h` | 344 | stdlib.h, string.h, time.h, ctype.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/time/calendar.h | Preserve |

### Subsystem: `libplatform` (11 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟡 | `engine/include/platform/platform.h` | 202 | stdio.h, time.h, strings.h | - | runtime/string/strops.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_clipboard.c` | 127 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | string_mem (memcpy); string_str (strlen strcpy); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_console.c` | 531 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | system_os (sleep usleep); stdio_io (printf fflush getchar) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_dl.c` | 99 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | - | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_fs.c` | 448 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | stdio_io (printf remove rename); format (snprintf); memory (calloc free); string_str (strcmp strrchr strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_net.c` | 286 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | string_mem (memset); format (snprintf); string_str (strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_regex.c` | 199 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | string_mem (memcpy); string_str (strlen); memory (calloc realloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_sys.c` | 520 | stdio.h, stdlib.h, signal.h, string.h, time.h, math.h, mmsystem.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | string_str (strncpy); memory (malloc free); string_mem (memcpy); stdio_io (fwrite fflush); system_os (system exit getenv setenv); math_trig (sin) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/math/, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_thread.c` | 176 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | memory (calloc free) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/plat_time.c` | 165 | stdio.h, stdlib.h, string.h, time.h, signal.h, direct.h, netdb.h, dlfcn.h, errno.h, pthread.h, ncurses.h, dos.h, sys/stat.h, sys/timeb.h, sys/time.h, winsock2.h, ws2tcpip.h, windows.h, conio.h, io.h, unistd.h, termios.h, sys/select.h, sys/ioctl.h, sys/socket.h, sys/types.h, arpa/inet.h, dirent.h, fcntl.h | time (time localtime_s localtime_r); system_os (sleep usleep) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🔴 | `engine/lib/platform/platform_core.c` | 104 | stdio.h, stdlib.h, string.h, time.h, sys/stat.h, sys/timeb.h, windows.h, sys/time.h, unistd.h | system_os (sleep usleep); stdio_io (fprintf) | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |

### Subsystem: `libstandard` (34 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟡 | `engine/include/editor/edit_internal.h` | 181 | stdio.h, stdlib.h, string.h, time.h | - | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/editor/editor.h` | 35 | - | - | None | Preserve |
| 🟢 | `engine/include/editor/editor_core.h` | 105 | - | - | None | Preserve |
| 🔴 | `engine/include/editor/edlin_internal.h` | 97 | ctype.h, stdio.h, stdlib.h, string.h, windows.h, sys/ioctl.h, termios.h, unistd.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `engine/include/editor/vi_internal.h` | 134 | stdio.h, stdlib.h, string.h, time.h | - | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/include/editor/ws_internal.h` | 140 | stdio.h, stdlib.h, string.h, time.h | - | runtime/string/memops.h, runtime/memory/alloc.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/lib/editor/editor_buffer.c` | 98 | stdio.h, stdlib.h, string.h | memory (calloc realloc free); stdio_io (fprintf) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/lib/editor/editor_render.c` | 134 | stdio.h, stdlib.h, string.h | string_str (strlen strcpy strcat); string_mem (memcpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/lib/editor/editor_selection.c` | 138 | stdlib.h, string.h | memory (calloc); string_mem (memcpy memmove) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟢 | `engine/lib/editor/editor_term.c` | 23 | - | - | None | Preserve |
| 🟢 | `engine/src/editor/edit.c` | 22 | - | - | None | Preserve |
| 🟡 | `engine/src/editor/edit/edit_buf.c` | 378 | - | memory (calloc realloc free); stdio_io (printf fprintf fopen fclose fgets fflush getchar); format (snprintf); string_mem (memmove); string_str (strlen strcmp strncmp strcpy strncpy strcat strstr) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/edit/edit_cmd.c` | 225 | - | string_mem (memmove); string_str (strcpy); memory (free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/editor/edit/edit_menu.c` | 105 | - | format (snprintf); memory (free); string_str (strcpy) | runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/editor/edit/edit_render.c` | 295 | - | time (time strftime); format (snprintf); string_mem (memcpy); string_str (strlen strcpy strcat) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, runtime/time/calendar.h | Preserve |
| 🔴 | `engine/src/editor/edit/edit_term.c` | 290 | poll.h, termios.h, unistd.h, sys/ioctl.h, windows.h, conio.h | system_os (sleep); format (vsnprintf); string_str (strlen strcpy); stdio_io (printf fflush) | runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/editor/editor.c` | 142 | stdio.h, stdlib.h, string.h, ctype.h | string_mem (memset); conv (atof); string_str (strlen strcmp strncmp); ctype (isdigit isspace toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/editor_manager.c` | 32 | string.h | string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟢 | `engine/src/editor/edlin.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/editor/edlin/edlin_buf.c` | 141 | - | stdio_io (fprintf fopen fclose fgets); format (snprintf); memory (calloc realloc free); string_str (strlen strcpy) | runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/edlin/edlin_cmd.c` | 356 | - | stdio_io (fprintf fopen fclose fgets fflush); format (snprintf); memory (calloc); conv (strtol); string_str (strlen strcmp strcpy strstr strcspn) | runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/edlin/edlin_exec.c` | 198 | - | stdio_io (printf fprintf fopen fclose fflush getchar); conv (strtol); string_mem (memset); string_str (strcmp strcspn); ctype (isdigit tolower) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/src/editor/standalone_runner.h` | 76 | stdio.h, stdlib.h, string.h, io.h, unistd.h | system_os (system); format (snprintf); string_str (strstr); stdio_io (printf) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟢 | `engine/src/editor/tui_multiplexer.c` | 20 | - | - | None | Preserve |
| 🟢 | `engine/src/editor/vi.c` | 21 | - | - | None | Preserve |
| 🟡 | `engine/src/editor/vi/vi_buf.c` | 228 | - | stdio_io (printf fprintf fopen fclose fgets fflush getchar); format (snprintf); memory (calloc realloc free); string_str (strlen strcmp strncmp strcpy strncpy) | runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/vi/vi_cmd.c` | 268 | - | stdio_io (printf fflush); string_mem (memmove); string_str (strcmp strncmp strcpy strncpy strcat) | runtime/string/memops.h, runtime/string/strops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/vi/vi_render.c` | 200 | - | time (time strftime); format (snprintf); string_mem (memcpy); string_str (strlen strcpy strcat); stdio_io (printf fflush) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/src/editor/vi/vi_term.c` | 232 | poll.h, termios.h, unistd.h, sys/ioctl.h, windows.h, conio.h | system_os (sleep) | hal/hal.h | Preserve |
| 🟢 | `engine/src/editor/ws.c` | 21 | - | - | None | Preserve |
| 🟡 | `engine/src/editor/ws/ws_buf.c` | 321 | - | memory (calloc realloc free); stdio_io (printf fprintf fopen fclose fgets fflush getchar); format (snprintf); string_mem (memmove); string_str (strlen strcmp strncmp strcpy strncpy strcat) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/editor/ws/ws_cmd.c` | 231 | - | memory (free); string_mem (memmove) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/editor/ws/ws_render.c` | 189 | - | time (time strftime); format (snprintf); string_mem (memcpy); string_str (strlen strcpy strcat); stdio_io (fflush) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/src/editor/ws/ws_term.c` | 233 | poll.h, termios.h, unistd.h, sys/ioctl.h, windows.h, conio.h | system_os (sleep) | hal/hal.h | Preserve |

### Subsystem: `statements_microlib` (435 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟢 | `engine/include/statements/bgi/picture.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/bgi/viewport.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/cause.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/continue.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/declare.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/doevents.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/extend.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/external.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/handler.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/retry.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/scale.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/sleep.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/suspend.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/unless.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/void.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/wait.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/control/when.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/end.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/for.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/gosub.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/goto.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/if.h` | 32 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/input.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/line_input.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/map.h` | 59 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/next.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/print.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/randomize.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/rem.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/return.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/select.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/stop.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/wend.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/core/while.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/db/isam.h` | 41 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/debug/check.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/debug/test.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/debug/verify.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/alias.h` | 26 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/category.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/help.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/introspection.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/keyword.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/module_stmt.h` | 16 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/override.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/remove.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/scope.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/dialect/selftest.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/key.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/on_com.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/on_error.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/on_key.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/on_timer.h` | 30 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/resume.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/try.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/event/whenever.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/extended/bitmux.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/append.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/assign.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/backspace.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/bload.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/brun.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/bsave.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/chdir.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/close.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/create.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/destroy.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/field.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/files.h` | 30 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/find.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/get.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/input_file.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/kill.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/lock.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/mkdir.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/modify.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/name.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/open.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/prefix.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/print_file.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/put.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/record.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/rewind.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/rmdir.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/seek.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/text.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/vdim.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/filesystem/write_file.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/bgi.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/circle.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/cls.h` | 18 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/color.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/compat.h` | 32 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/draw.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/line.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/paint.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/palette.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/preset.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/pset.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/screen.h` | 34 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/stmt_home.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/stmt_plot.h` | 28 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/view.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/graphics/window.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/ask.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/echo.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/enter.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/form.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/image.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/linput.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/lprint.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/margin.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/page.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/io/zone.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/do.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/endloop.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/exit_loop.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/for.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/loop.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/next.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/repeat.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/until.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/wend.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/loops/while.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/matrices/mat_input.h` | 20 | - | - | None | Preserve |
| 🟡 | `engine/include/statements/matrices/mat_internal.h` | 66 | ctype.h, math.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/statements/matrices/mat_ops.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/matrices/mat_print.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/matrices/mat_read.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/matrices/mat_write.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/call.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/class.h` | 32 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/def.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/enum.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/function.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/param.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/shared.h` | 24 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/sub.h` | 31 | - | - | None | Preserve |
| 🟡 | `engine/include/statements/oop/sub_internal.h` | 61 | ctype.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/statements/oop/type.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/oop/with.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/auto.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/chain.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/clear.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/cont.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/delete.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/list.h` | 26 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/llist.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/load.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/merge.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/new.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/reformat.h` | 138 | - | - | None | Preserve |
| 🟡 | `engine/include/statements/program/reformat_internal.h` | 128 | ctype.h, stdio.h, stdlib.h, string.h | ctype (isspace); format (snprintf) | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/include/statements/program/renum.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/run.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/program/save.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/sound/beep.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/sound/play.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/sound/sound.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/sound/voice.h` | 32 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/bios.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/debug.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/def_seg.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/def_usr.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/defseg.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/interrupt.h` | 35 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/invoke.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/joystick.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/mouse.h` | 30 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/mux.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/out.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/pause.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/pen.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/perform.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/poke.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/security.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/session_stmts.h` | 33 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/shell.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/sys.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/system.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/task.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/txn.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/system/version.h` | 23 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/ui/msgbox.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/ui/vbdos_controls.h` | 41 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/ui/vbdos_widgets.h` | 33 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/array_ext.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/arrayfill.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/change.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/clr.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/common.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/complex.h` | 29 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/const.h` | 20 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/data.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/def.h` | 42 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/defdbl.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/defint.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/deflng.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/defsng.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/defstr.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/dim.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/erase.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/exchange.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/global.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/incr.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/let.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/lset.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/mid_stmt.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/mux.h` | 27 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/option.h` | 22 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/public.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/randomize.h` | 19 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/read.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/redim.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/restore.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/rset.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/share.h` | 21 | - | - | None | Preserve |
| 🟢 | `engine/include/statements/variables/swap.h` | 21 | - | - | None | Preserve |
| 🟡 | `engine/src/statements/bgi/picture.c` | 72 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/bgi/viewport.c` | 51 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/cause.c` | 56 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/control/continue.c` | 31 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/declare.c` | 93 | string.h | string_str (_strnicmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/control/doevents.c` | 41 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/extend.c` | 48 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/external.c` | 61 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/handler.c` | 64 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/retry.c` | 31 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/scale.c` | 48 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/sleep.c` | 55 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/suspend.c` | 160 | string.h, ctype.h | string_mem (memset); format (snprintf); string_str (strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/control/unless.c` | 157 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟢 | `engine/src/statements/control/void.c` | 43 | - | - | None | Preserve |
| 🟡 | `engine/src/statements/control/wait.c` | 105 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/control/when.c` | 158 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/core/end.c` | 119 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/core/gosub.c` | 122 | string.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/core/goto.c` | 101 | string.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/core/if.c` | 242 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/core/input.c` | 282 | stdio.h, string.h, stdlib.h, ctype.h | string_mem (memcpy memset); conv (strtod); string_str (strlen strncpy); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/line_input.c` | 196 | string.h, stdio.h | stdio_io (fgets); string_mem (memcpy memset); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/map.c` | 384 | string.h, ctype.h, stdio.h, stdlib.h | string_str (strlen strncpy); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/print.c` | 351 | stdio.h, string.h, math.h | string_mem (memset); format (snprintf); string_str (strlen strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/math/, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/randomize.c` | 222 | stdio.h, stdlib.h, string.h, time.h, ctype.h | time (time); conv (atol); string_mem (memcpy memset); string_str (strlen strncmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, runtime/time/calendar.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/rem.c` | 91 | stdio.h, string.h | stdio_io (fopen fclose fgets); format (snprintf); string_mem (memset); string_str (strlen strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/return.c` | 44 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/core/select.c` | 309 | stdio.h, stdlib.h, string.h, ctype.h | string_str (strcmp strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/core/stop.c` | 42 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/db/isam.c` | 396 | string.h, stdlib.h, stdio.h | string_mem (memcpy memset); format (snprintf); string_str (strncpy strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/debug/check.c` | 83 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/debug/test.c` | 76 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/debug/verify.c` | 135 | stdio.h, stdlib.h, string.h | stdio_io (fopen fclose fgets); format (snprintf sscanf); string_mem (memset); string_str (strlen strchr strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/introspection.c` | 72 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/dialect/keyword.c` | 161 | string.h, ctype.h, stdio.h, stdlib.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/dialect/remove.c` | 430 | string.h, stdlib.h | string_mem (memcpy memset); string_str (strcpy strstr); memory (calloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/statements/dialect/selftest.c` | 145 | string.h | string_str (strcmp strncmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/event/key.c` | 162 | string.h, stdio.h | string_mem (memset) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/event/on_com.c` | 36 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/event/on_error.c` | 70 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/event/on_key.c` | 117 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/event/on_timer.c` | 218 | string.h, math.h | string_mem (memcpy memset) | runtime/string/memops.h, runtime/math/ | Preserve |
| 🟡 | `engine/src/statements/event/resume.c` | 106 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/event/try.c` | 216 | string.h, stdlib.h, ctype.h | string_str (strlen strncpy); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/statements/event/whenever.c` | 159 | string.h, ctype.h, strings.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/append.c` | 203 | string.h, stdio.h, stdlib.h, ctype.h | string_str (strlen strstr); stdio_io (fprintf fopen fclose fgets remove rename); memory (calloc free); string_mem (memset); format (snprintf); ctype (isdigit isspace); conv (atof) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/assign.c` | 293 | string.h, stdio.h | string_mem (memcpy memset) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/backspace.c` | 66 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/bload.c` | 34 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/brun.c` | 34 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/bsave.c` | 36 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/chdir.c` | 50 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/close.c` | 81 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/create.c` | 106 | stdio.h, string.h | string_mem (memset); stdio_io (fopen fclose) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/statements/filesystem/destroy.c` | 28 | - | - | None | Preserve |
| 🟡 | `engine/src/statements/filesystem/field.c` | 114 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/files.c` | 306 | string.h, stdio.h, ctype.h | stdio_io (rewind fflush remove); format (snprintf); string_mem (memset); string_str (strlen strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/find.c` | 83 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/get.c` | 206 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/input_file.c` | 202 | string.h, stdlib.h, ctype.h | string_mem (memcpy memset); conv (strtod); string_str (strlen); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/kill.c` | 60 | stdio.h, string.h | string_mem (memset); stdio_io (remove) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🔴 | `engine/src/statements/filesystem/lock.c` | 181 | string.h, io.h, sys/stat.h, unistd.h | string_str (strlen strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/mkdir.c` | 50 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/modify.c` | 72 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/name.c` | 143 | string.h, stdio.h | stdio_io (rename); string_mem (memcpy memset); string_str (strchr strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/open.c` | 201 | string.h, ctype.h, stdio.h | string_mem (memcpy memset); format (snprintf); string_str (strcpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/prefix.c` | 29 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/print_file.c` | 257 | string.h, stdio.h | string_mem (memcpy memset); format (snprintf); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/put.c` | 191 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/record.c` | 73 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/rewind.c` | 61 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/rmdir.c` | 50 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/seek.c` | 36 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/filesystem/text.c` | 72 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟢 | `engine/src/statements/filesystem/vdim.c` | 28 | - | - | None | Preserve |
| 🟡 | `engine/src/statements/filesystem/write_file.c` | 90 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/bgi.c` | 420 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/circle.c` | 35 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/cls.c` | 77 | string.h, stdio.h | string_mem (memset) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/graphics/color.c` | 167 | string.h, stdio.h | string_mem (memset) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/graphics/compat.c` | 98 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/draw.c` | 58 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/line.c` | 42 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/paint.c` | 35 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/palette.c` | 36 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/preset.c` | 33 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/pset.c` | 35 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/screen.c` | 141 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/stmt_home.c` | 76 | string.h, stdio.h | string_mem (memset) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/graphics/stmt_plot.c` | 93 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/view.c` | 36 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/graphics/window.c` | 74 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/io/ask.c` | 145 | string.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/io/defseg.c` | 50 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/io/echo.c` | 84 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/io/enter.c` | 190 | string.h | string_str (strlen); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/io/form.c` | 40 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/io/image.c` | 40 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/io/linput.c` | 135 | string.h | string_str (strlen); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/io/lprint.c` | 99 | string.h, stdio.h | string_mem (memset); format (snprintf) | runtime/string/memops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/io/lset.c` | 156 | string.h, stdio.h, stdlib.h | memory (calloc free); string_mem (memcpy memset) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/io/margin.c` | 90 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/io/page.c` | 151 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/io/zone.c` | 90 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/loops/do.c` | 213 | string.h, ctype.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/loops/endloop.c` | 165 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/loops/exit_loop.c` | 165 | string.h, ctype.h | string_mem (memcpy memset); string_str (strcmp); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/loops/for.c` | 227 | string.h | string_str (strncasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/loops/loop.c` | 157 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/loops/next.c` | 149 | string.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/loops/repeat.c` | 37 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/loops/until.c` | 63 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/loops/wend.c` | 51 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/loops/while.c` | 177 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_input.c` | 213 | string.h, stdlib.h, ctype.h | string_mem (memcpy memset); conv (strtod); string_str (strlen); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_ops.c` | 414 | - | string_str (strncasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_ops/mat_arith.c` | 351 | - | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_ops/mat_special.c` | 100 | - | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_ops/mat_transform.c` | 247 | - | memory (calloc free); string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_print.c` | 169 | string.h, stdio.h | string_mem (memcpy memset); format (snprintf); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/matrices/mat_read.c` | 238 | string.h, stdlib.h, ctype.h | string_mem (memcpy memset); conv (strtod); string_str (strlen); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | Preserve |
| 🟢 | `engine/src/statements/matrices/mat_write.c` | 28 | - | - | None | Preserve |
| 🟡 | `engine/src/statements/oop/call.c` | 62 | string.h | string_str (strncasecmp); string_mem (memset memchr) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/oop/class.c` | 289 | string.h | string_str (strncpy strcasecmp strncasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/oop/def.c` | 192 | string.h, ctype.h | string_mem (memcpy memset); format (snprintf); string_str (strlen); ctype (isdigit isalpha toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/oop/enum.c` | 37 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/oop/function.c` | 108 | string.h | string_str (strcmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/oop/module.c` | 314 | stdio.h, string.h, ctype.h | string_str (strcasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/oop/param.c` | 82 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/oop/shared.c` | 155 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/oop/sub.c` | 126 | - | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/oop/sub/sub_exec.c` | 285 | - | string_mem (memcpy memset); format (snprintf); string_str (strncpy strchr strcasecmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/oop/sub/sub_invoke.c` | 285 | - | string_str (strncmp strchr strcasecmp strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/oop/sub/sub_lookup.c` | 401 | - | string_mem (memcpy); format (snprintf); string_str (strlen strncmp strcpy strncpy strchr strcasecmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/oop/type.c` | 160 | string.h | string_str (strncpy strcasecmp strncasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/oop/with.c` | 71 | string.h | string_str (strlen); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/program/auto.c` | 86 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/program/chain.c` | 162 | string.h, ctype.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/program/clear.c` | 47 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/program/cont.c` | 38 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/program/delete.c` | 107 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/program/list.c` | 149 | string.h, stdio.h | stdio_io (fopen fclose fputs); format (snprintf); string_mem (memset); string_str (strlen strstr strchr strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/program/llist.c` | 74 | string.h, stdio.h | string_mem (memset); format (snprintf) | runtime/string/memops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/program/load.c` | 178 | string.h, stdio.h, stdlib.h, ctype.h | stdio_io (fopen fclose fgets); conv (atof); string_mem (memset); string_str (strlen strstr); ctype (isdigit isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/program/merge.c` | 93 | string.h, stdio.h, stdlib.h, ctype.h | stdio_io (fopen fclose fgets); conv (atof); string_mem (memset); string_str (strlen); ctype (isdigit isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/program/new.c` | 50 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/program/reformat.c` | 151 | stdio.h, stdlib.h, string.h | string_mem (memset); format (snprintf); string_str (strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/program/reformat/reformat_analyze.c` | 285 | - | string_mem (memcpy memset); format (snprintf); conv (strtod); string_str (strlen strcmp strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, runtime/conv/num_parse.h | Preserve |
| 🟡 | `engine/src/statements/program/reformat/reformat_indent.c` | 313 | - | string_mem (memset); format (snprintf); string_str (strcmp strcasecmp); ctype (toupper tolower) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/program/reformat/reformat_report.c` | 203 | - | stdio_io (fprintf fopen fclose); format (snprintf); string_str (strlen strcmp strncat strrchr) | runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/statements/program/reformat_engine.c` | 16 | - | - | None | Preserve |
| 🟡 | `engine/src/statements/program/renum.c` | 205 | string.h, stdlib.h, ctype.h | string_mem (memcpy memset); format (snprintf); memory (calloc free); string_str (strlen strdup strncasecmp); ctype (isdigit isalnum) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/program/run.c` | 114 | string.h | string_str (strncpy); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/program/save.c` | 73 | stdio.h, string.h | string_mem (memset); stdio_io (fprintf fopen fclose) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/sound/beep.c` | 121 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/sound/play.c` | 289 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/sound/sound.c` | 132 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/sound/voice.c` | 156 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/bios.c` | 113 | stdio.h, string.h | string_str (strcasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/system/debug.c` | 286 | string.h, stdlib.h | string_mem (memset); format (snprintf); string_str (strlen strcpy strcasecmp strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/system/def_seg.c` | 67 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/def_usr.c` | 34 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/defseg.c` | 74 | string.h | string_str (strncasecmp); string_mem (memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🔴 | `engine/src/statements/system/interrupt.c` | 225 | string.h, stdio.h, time.h, windows.h, sys/time.h | time (time localtime); format (snprintf); string_mem (memcpy memset); string_str (strlen); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/format/snprintf.h, runtime/time/calendar.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/statements/system/invoke.c` | 52 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/joystick.c` | 44 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/mouse.c` | 72 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/mux.c` | 71 | stdio.h, stdlib.h, string.h | string_mem (memset) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/system/out.c` | 85 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/pause.c` | 45 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/pen.c` | 37 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/perform.c` | 29 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/poke.c` | 108 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/security.c` | 237 | stdio.h, string.h, ctype.h | ctype (toupper); string_mem (memcpy memset) | runtime/string/memops.h, runtime/ctype/ctype.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/system/session_stmts.c` | 176 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/system/shell.c` | 54 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/system/sys.c` | 43 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🔴 | `engine/src/statements/system/system.c` | 226 | stdio.h, stdlib.h, string.h, windows.h, unistd.h | string_mem (memcpy memset); system_os (system); string_str (strcmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, hal/io_hal.h, hal/hal.h | Preserve |
| 🟡 | `engine/src/statements/system/task.c` | 129 | stdio.h, string.h | string_mem (memcpy memset) | runtime/string/memops.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/system/txn.c` | 112 | string.h, stdio.h, stdlib.h | string_mem (memcpy memset) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/system/version.c` | 158 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strlen strcasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/ui/msgbox.c` | 104 | stdio.h, string.h | string_mem (memset); format (snprintf); string_str (strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/ui/vbdos_controls.c` | 376 | string.h, stdio.h | string_mem (memset); format (snprintf); string_str (strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/ui/vbdos_widgets.c` | 182 | string.h, stdio.h | string_mem (memset); format (snprintf); string_str (strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/variables/array_ext.c` | 134 | stdio.h, stdlib.h, string.h, ctype.h | string_mem (memcpy memset); format (snprintf); string_str (strncpy) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/variables/arrayfill.c` | 99 | string.h | string_mem (memcpy) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/change.c` | 209 | string.h, stdlib.h, ctype.h | string_mem (memcpy memset); string_str (strncasecmp); memory (malloc free) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/statements/variables/clr.c` | 75 | string.h | string_mem (memcpy) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/common.c` | 73 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/complex.c` | 94 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/const.c` | 77 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/data.c` | 37 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/def.c` | 471 | string.h, stdio.h, stdlib.h, ctype.h | string_mem (memcpy memset); string_str (strncpy strcasecmp); memory (calloc); ctype (toupper) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/statements/variables/defdbl.c` | 67 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/variables/defint.c` | 67 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/variables/deflng.c` | 67 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/variables/defsng.c` | 67 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/variables/defstr.c` | 67 | string.h, ctype.h | ctype (toupper); string_mem (memset) | runtime/string/memops.h, runtime/ctype/ctype.h | Preserve |
| 🟡 | `engine/src/statements/variables/dim.c` | 205 | string.h | string_str (strncasecmp); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/variables/erase.c` | 56 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/exchange.c` | 29 | string.h | - | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/global.c` | 74 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/incr.c` | 148 | string.h, stdlib.h | string_mem (memcpy) | runtime/string/memops.h, runtime/memory/alloc.h | Preserve |
| 🟡 | `engine/src/statements/variables/let.c` | 425 | string.h, stdlib.h | string_mem (memcpy memset); format (snprintf); memory (malloc free); string_str (strlen strncpy strchr strncasecmp) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/statements/variables/lset.c` | 109 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/mid_stmt.c` | 36 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/mux.c` | 55 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/option.c` | 63 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/public.c` | 115 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/randomize.c` | 34 | string.h | string_mem (memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/read.c` | 180 | string.h, stdlib.h, ctype.h | string_mem (memcpy memset); conv (strtoull strtod); string_str (strlen); ctype (isspace) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/conv/num_parse.h | Preserve |
| 🟡 | `engine/src/statements/variables/redim.c` | 51 | string.h | string_str (strncasecmp); string_mem (memcpy) | runtime/string/memops.h, runtime/string/strops.h | Preserve |
| 🟡 | `engine/src/statements/variables/restore.c` | 107 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/rset.c` | 114 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/share.c` | 77 | string.h | string_mem (memcpy memset) | runtime/string/memops.h | Preserve |
| 🟡 | `engine/src/statements/variables/swap.c` | 152 | string.h | string_str (strlen); string_mem (memcpy memset) | runtime/string/memops.h, runtime/string/strops.h | Preserve |

### Subsystem: `tools` (7 files)

| Score | File Path | SLOC | System Headers | Functions Detected | Required Replacement Module | Proposed Granular Destination |
|---|---|---|---|---|---|---|
| 🟡 | `engine/include/tools/bppc_internal.h` | 52 | ctype.h, stdio.h, stdlib.h, string.h | - | runtime/string/memops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
| 🟢 | `engine/src/tools/bppc.c` | 21 | - | - | None | Preserve |
| 🟡 | `engine/src/tools/bppc/bppc_bytecode.c` | 234 | - | string_str (strlen strncpy strrchr); memory (calloc realloc free); string_mem (memcpy memset); format (snprintf); stdio_io (printf fprintf fopen fclose fread fwrite); system_os (system); sort (qsort) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, runtime/sort/qsort.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/tools/bppc/bppc_detok.c` | 158 | - | string_mem (memcpy); format (snprintf); memory (calloc); string_str (strlen) | runtime/string/memops.h, runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h | Preserve |
| 🟡 | `engine/src/tools/bppc/bppc_main.c` | 215 | - | system_os (system); format (snprintf); memory (calloc free); string_str (strlen strcmp strncpy strcasecmp); stdio_io (printf fprintf fopen fclose fread fseek ftell remove) | runtime/string/strops.h, runtime/memory/alloc.h, runtime/format/snprintf.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/tools/bppc/bppc_transpile.c` | 218 | stdio.h, stdlib.h, string.h, math.h | string_str (strlen strcmp strncmp strncpy strstr strchr strtok); stdio_io (printf fprintf fopen fclose); memory (calloc free); string_mem (memcpy); format (snprintf); ctype (isdigit isalpha isalnum isspace toupper); conv (strtol) | runtime/string/memops.h, runtime/string/strops.h, runtime/ctype/ctype.h, runtime/memory/alloc.h, runtime/math/, runtime/format/snprintf.h, runtime/conv/num_parse.h, hal/io_hal.h | Preserve |
| 🟡 | `engine/src/tools/detok.c` | 189 | stdio.h, stdlib.h, string.h | stdio_io (printf fprintf fopen fclose rewind) | runtime/string/memops.h, runtime/memory/alloc.h, hal/io_hal.h | Preserve |
