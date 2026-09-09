<!--
Title:        C_Programmers_Guide
Tier:         1
Applies to:   BASIC++ v6.5.2, all targets
Authority:    engine/include/basicpp.h, engine/include/interop/,
              engine/include/basicpp_sys.h, engine/CMakeLists.txt
Generated:    no, hand-written
Status:       current
-->

# BASIC++ for C Programmers

You already know C. This tells you what BASIC++ is in your terms, how to embed
it, and where the interesting parts are. It is an orientation document: it
links rather than duplicates.

---

## 1. What BASIC++ is, from where you are standing

A BASIC interpreter, compiler and transpiler written in **freestanding C17
with zero libc**. That last part is the whole design, and it is what makes the
project unusual rather than nostalgic.

Freestanding means the engine does not call `malloc`, `printf`, `strlen` or
anything else from a hosted C library. It supplies its own: `runtime_memset`,
`runtime_snprintf`, `str_*`, and an arena allocator. Platform services arrive
through a HAL, so the same engine source compiles for Linux, Windows,
FreeDOS, WASM and a microcontroller.

Consequences you will care about:

- No garbage collector. Containers are reference-counted with explicit
  `add_ref` and `release`. Execution is deterministic and there is no
  collection pause.
- It links into your program as a library. `basicpp.dll`, `libbasicpp.so`, or
  a static archive.
- It builds under C17 with `-Wall -Wextra -Wpedantic -Werror`, or `/W4 /WX`
  under MSVC. Warnings are errors throughout.

---

## 2. Embedding it: the whole API in one page

`engine/include/basicpp.h` is a single header. It is `extern "C"` guarded, so
C++ hosts work unchanged.

**Note that there are two public headers**, and you should know which you are
using. `basicpp.h` is the fuller one and is documented here. `bpp_api.h`
declares an overlapping API with different variable accessors:
`bpp_get_var_num`, `bpp_set_var_num`, `bpp_get_var_str`, `bpp_set_var_str`,
where `bpp_get_var_str(ctx, name, out_buf, buf_size)` writes into a
caller-supplied buffer and returns `bool` rather than returning an
engine-owned pointer. Both declare `bpp_init`, `bpp_shutdown`,
`bpp_exec_string`, `bpp_eval_expr`, `BppEngineContext` and `BppValue`.

The WASM target is built from `bpp_api.c` and exports the `bpp_api.h` names,
so browser code uses those. For a native host prefer `basicpp.h`, which also
carries the message bus, IPC and logging that `bpp_api.h` does not. The
duplication is a known maintenance hazard and is recorded rather than
resolved.

```c
#include "basicpp.h"

int main(void) {
    BppEngineContext *ctx = bpp_init(64 * 1024 * 1024);   // 64 MB arena

    bpp_set_number(ctx, "N", 10);
    bpp_exec_string(ctx, "FOR I = 1 TO N : PRINT I : NEXT I");

    BppValue v = bpp_eval_expr(ctx, "N * 2");
    printf("%g\n", v.as.number);
    bpp_value_free(ctx, &v);

    bpp_shutdown(ctx);
    return 0;
}
```

### Lifecycle

| Function | Purpose |
|---|---|
| `bpp_init(size_t ram_bytes)` | Create a context with an arena of the given size |
| `bpp_shutdown(ctx)` | Destroy it |
| `bpp_reset(ctx)` | Clear program and variables, keep the context |
| `bpp_version()` | Version string |

`BppEngineContext` is opaque. You never see its layout, which means the ABI
survives engine changes.

### Execution

| Function | Purpose |
|---|---|
| `bpp_exec_string(ctx, src)` | Run a program from memory |
| `bpp_exec_file(ctx, path)` | Run a program from disk |
| `bpp_eval_expr(ctx, expr)` | Evaluate one expression, return a `BppValue` |
| `bpp_value_free(ctx, &val)` | Release a returned value |

### Variables

`bpp_set_number`, `bpp_set_string`, `bpp_get_number`, `bpp_get_string`. This
is how you push configuration in and pull results out without parsing text.

### Zero-copy array views

```c
BPP_API int bpp_array_bind_view(BppEngineContext *ctx, const char *arr_name,
                                int elem_type, void *data_ptr, size_t count);
```

Bind a C array directly as a BASIC++ array. No copy, no marshalling. This is
the function to reach for when the host owns a large buffer and you want BASIC
code to operate on it in place.

### Message bus and IPC

`bpp_bus_publish`, `bpp_bus_subscribe`, `bpp_bus_unsubscribe`, `bpp_ipc_send`,
`bpp_ipc_recv`. A topic-based callback bus for host and script to exchange
events rather than polling.

### Logging

`bpp_log_add_sink` installs a `BppLogSinkFn` and routes engine diagnostics
into your host's logging. Six levels from `BPP_LOG_TRACE` to `BPP_LOG_FATAL`.

---

## 3. What the API tells you that the source does not

Worth stating explicitly, because it is a fair question and the answer is not
"nothing".

An implementation tells you what happens today. An interface tells you what
will keep happening. Specifically, `basicpp.h` fixes:

- **The ABI.** `BppEngineContext` is opaque and `BPP_API` handles the
  visibility and dllexport attributes. You can upgrade the engine under a
  compiled host.
- **Ownership.** `bpp_value_free` exists, which tells you `BppValue` from
  `bpp_eval_expr` is yours to release. `bpp_get_string` returns `const char *`
  with no free function, which tells you it is engine-owned and borrowed.
- **What is supported.** A function in the header is a promise. A function
  found by grepping the sources is an implementation detail that may be static
  next month.

Where `basicpp.h` is currently silent, and you should not infer: thread
safety per context, re-entrancy of callbacks invoked from inside a running
program, and behaviour when a callback itself calls back into the engine.
Treat one context as single-threaded until that is documented.

---

## 4. Extending the language from C

Adding a keyword is a full vertical: lexer, parser, evaluator, handler,
`LanguageDescriptor` registration, CMake target, tests, documentation. Every
keyword is one file and one `OBJECT` target.

Registration is declarative:

```c
REGISTER_STATEMENT(&my_desc, my_handler);
REGISTER_FUNCTION(&my_desc, my_evaluator);
```

`engine/include/runtime/language_descriptor.h` is the metadata contract, and
it is mandatory: name, category, syntax, description, errors, subsystem,
safety level, feature type and delimiter mask. The documentation for your
keyword is generated from it.

See `Extension_Guide` and the tutorials under `docs/api/tutorials/`.

---

## 5. Interop internals

`engine/include/interop/` holds the FFI machinery:

| Header | Purpose |
|---|---|
| `interop_core.h` | Subsystem init and shutdown |
| `interop_handle.h` | Generation-counted handle table |
| `interop_error.h` | Thread-local error state |
| `interop_marshal.h` | Type marshalling across the boundary |
| `interop_com.h` | COM vtables in pure C17 |
| `interop_ipc.h` | Inter-process channels |
| `interop_jsonrpc.h` | JSON-RPC, hand-written recursive descent |

The handle table is worth understanding if you cross the boundary often. It
hands out `uint64_t` handles rather than raw pointers, with retain, release
and validity checks, so a stale handle from foreign code is detected rather
than dereferenced.

**A note on these headers:** they use Doxygen `@brief`, `@param` and `@return`
tags, which `PROJECT_RULES.md` section 6 prohibits. That is a source defect,
recorded here rather than silently corrected.

---

## 6. Bare metal

`engine/include/basicpp_sys.h` is the freestanding systems layer: MMIO
accessors at 8, 16, 32 and 64 bits, x86 port I/O, a monotonic arena
allocator, a `SysConsole` hook struct, and freestanding `memcpy`, `memset`,
`strlen` and `itoa`.

Two limitations you must know before trusting it on non-x86 hardware:

- Port I/O exists only for x86 and x86_64, and **silently compiles to a no-op
  returning 0** elsewhere.
- `sys_memory_barrier()` is a compiler barrier, not a CPU fence. It does not
  order MMIO against DMA on a weakly ordered architecture.

See `Systems_Programming`.

---

## 7. Building

```
cmake -S . -B build_linux
cmake --build build_linux
```

Or `tools/build_linux.sh` and `tools/build_win.ps1`. Targets: `baspp`
(desktop), `bpp` (REPL), `bs` (server), `bppc` (compiler), `trans`
(transpiler), `detok`, `iot`.

Quality gates are enforced: files under 400 lines with a 250-line target,
functions under 60 lines, the six-line header on every file, `//` comments
only, three-tier include ordering. See `PROJECT_RULES.md`.

---

## 8. Where to go next

| You want | Read |
|---|---|
| Embedding in depth | `C_Host_Embedding_API` |
| Adding keywords, modules, devices | `Extension_Guide` |
| The object model | `Set_Based_Object_Model` |
| Bare metal, PEEK/POKE/INP/OUT | `Systems_Programming` |
| Porting to a new platform | `Porting_And_Portability_Guide` |
| The engine's internal layout | `Internals_And_Architecture` |
| Per-symbol API reference | `docs/api/` |
