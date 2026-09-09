<!--
Title:        Web_And_Backend_Guide
Tier:         1
Applies to:   BASIC++ v6.5.2, hosted and WASM targets
Authority:    CMakeLists.txt lines 131-137, engine/include/bpp_api.h,
              engine/include/basicpp.h, engine/src/runtime/sock_engine.c
Generated:    no, hand-written
Status:       current
-->

# Web and Backend Guide

For JavaScript and back-end developers. What BASIC++ can do on the web, what
it cannot do on a server, and the one build target that makes the first
interesting.

---

## 1. The honest summary

**On the web: real.** There is a WebAssembly target, it exports a proper API,
and BASIC++ runs in a browser tab with no server at all.

**On the server: not yet.** The socket layer is a simulation. `sock_open()`
claims a slot in a static array and returns its index; no operating-system
socket is created anywhere. You cannot listen on a port. See
`Networking_Reference` for the evidence.

So the serverless page is buildable today and the API service is not. That is
an unusual shape, and it happens to suit the web case well: a page that runs
entirely client-side needs no sockets.

---

## 2. Running BASIC++ in a browser

The `wap` target, from the root `CMakeLists.txt`:

```cmake
if(EMSCRIPTEN)
    add_executable(wap engine/src/core/bpp_api.c ${HEADLESS_OBJECTS})
    target_compile_definitions(wap PRIVATE BASIC_LITE_BUILD
                               BASIC_HEADLESS_BUILD NO_SDL2 WASM_BUILD)
    set_target_properties(wap PROPERTIES LINK_FLAGS
        "-s WASM=1 -s EXPORTED_FUNCTIONS='[...]'
         -s EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'")
endif()
```

Eight functions are exported to JavaScript:

```
_bpp_init          _bpp_shutdown
_bpp_exec_string   _bpp_eval_expr
_bpp_get_var_num   _bpp_set_var_num
_bpp_get_var_str   _bpp_set_var_str
```

With `ccall` and `cwrap` available, which is what you need to call them
idiomatically.

### Building it

```
emcmake cmake -S . -B build_wasm
cmake --build build_wasm --target wap
```

The target is guarded by `if(EMSCRIPTEN)`, so it simply does not exist in a
normal build. That is why you may not have noticed it.

### Using it from JavaScript

```javascript
const bppInit    = Module.cwrap('bpp_init',        'number', ['number']);
const bppExec    = Module.cwrap('bpp_exec_string', 'number', ['number','string']);
const bppSetNum  = Module.cwrap('bpp_set_var_num', 'number', ['number','string','number']);
const bppGetNum  = Module.cwrap('bpp_get_var_num', 'number', ['number','string']);
const bppShutdown= Module.cwrap('bpp_shutdown',    null,     ['number']);

const ctx = bppInit(16 * 1024 * 1024);
bppSetNum(ctx, 'N', 10);
bppExec(ctx, 'TOTAL = 0 : FOR I = 1 TO N : TOTAL = TOTAL + I : NEXT I');
console.log(bppGetNum(ctx, 'TOTAL'));   // 55
bppShutdown(ctx);
```

That is a complete BASIC interpreter in a page, with no server, no build step
for the user, and no network.

### What it is good for

- Teaching BASIC in a browser with nothing to install.
- Running vintage programs on a web page.
- Embedding a scripting language in a web app where you want deterministic
  execution and no `eval`.
- Sandboxed user scripting: the WASM build has no filesystem and no network,
  which for once is a feature.

### What is not there

No canvas binding, so `SCREEN`, `PSET` and `LINE` have nowhere to draw. No
DOM access. No shell. The build defines `NO_SDL2` and `BASIC_HEADLESS_BUILD`,
so it is text-in, text-out through the exported functions. Wiring `PRINT`
output to a page element is work you would do yourself, and it is not large.

---

## 3. A caution: there are two C APIs

You will meet both and they are not the same.

| Header | Variable accessors | Notes |
|---|---|---|
| `engine/include/basicpp.h` | `bpp_get_number`, `bpp_set_number`, `bpp_get_string`, `bpp_set_string` | `bpp_get_string` returns `const char *`, engine-owned |
| `engine/include/bpp_api.h` | `bpp_get_var_num`, `bpp_set_var_num`, `bpp_get_var_str`, `bpp_set_var_str` | `bpp_get_var_str(ctx, name, out_buf, buf_size)` returns `bool`, caller supplies the buffer |

Both declare `bpp_init`, `bpp_shutdown`, `bpp_exec_string`, `bpp_eval_expr`,
`BppEngineContext` and `BppValue`.

The WASM target is built from `bpp_api.c` and exports the `bpp_api.h` names,
so **use `bpp_api.h` names in browser code**. For a native host, `basicpp.h`
is the documented single-header entry point and carries the message bus, IPC
and logging that `bpp_api.h` does not.

This duplication is a finding, recorded rather than resolved here: two public
headers covering the same subsystem with different naming conventions is a
maintenance hazard, and one of them should eventually become the other's
subset.

---

## 4. Back-end work: what is missing

If you are evaluating BASIC++ for a service, this is the list.

| You need | State |
|---|---|
| Listening sockets | Simulated. `sock_bind` records a port in a struct |
| TLS | Does not exist |
| HTTP server | Does not exist |
| Request routing | Does not exist |
| Database drivers | Do not exist |
| Connection pooling | Not applicable without sockets |
| Preemptive threads | Cooperative only: `TASK`, `YIELD` |

What does exist and is useful: `TASK` cooperative concurrency with
`PRIORITY`, `SUSPEND` and `MUTEX`; `PIPE`, `MSGSEND`, `MSGRECV$`, `PUBLISH`
and `SUBSCRIBE` for in-process messaging; the full file and directory surface;
`CRYPTO.HASH$` and `CRYPTO.HMAC$`; six-level logging with host sinks.

### The pattern that does work

Embed BASIC++ inside a host that owns the network. Your C, Go or Rust service
handles sockets, TLS and routing; BASIC++ handles the logic your users can
change without recompiling your service:

```c
BppEngineContext *ctx = bpp_init(32 * 1024 * 1024);
bpp_set_string(ctx, "REQUEST$", request_body);
bpp_exec_file(ctx, "handlers/order.bas");
const char *reply = bpp_get_string(ctx, "RESPONSE$");
```

This is the sound use of BASIC++ on a server today: a deterministic,
sandboxable, capability-gated scripting layer with no garbage collector,
inside something else that does the networking. See `C_Programmers_Guide`.

---

## 5. Coming from JavaScript

| JavaScript | BASIC++ | Note |
|---|---|---|
| `let` / `var` | `DIM`, `VAR`, `LET` | `LET` is optional as in JS `var` |
| `const` | `CONST` | |
| `function` | `FUNCTION`, `SUB` | `SUB` returns nothing |
| `Array` | array, or `SET` | `SET` has real set algebra |
| `Object` | `MAP`, or a `GROUP` | See `Set_Based_Object_Model` |
| `class` | `GROUP` and its members | There is no class; behaviour attaches to the group |
| `try` / `catch` | `TRY` / `HANDLER` | `RAISE` to throw |
| `import` | `IMPORT`, `MODULE` | |
| Template literals | `PRINT USING`, `FORMAT$` | |
| `JSON.parse` | `NET.UNPACK`, map statements | Also `interop_jsonrpc.h` on the C side |
| `async` / `await` | `TASK`, `YIELD` | Cooperative, not promise-based |
| `setInterval` | `EVERY`, `ON TIMER` | |
| `===` | `=` | BASIC has no type-coercing equality to distinguish from |
| Truthy or falsy | -1 and 0 | True is -1. Vintage programs rely on it |

The largest conceptual difference is the object model, and it is worth the
twenty minutes: `Set_Based_Object_Model` section 10 states why sets are
preferred over objects, and what that costs.

## See also

- `Networking_Reference` for what the network layer really does
- `C_Programmers_Guide` for embedding
- `Set_Based_Object_Model` for the object model
- `Systems_Administration_Guide` for scripting and automation
