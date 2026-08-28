# `wasm` WebAssembly Runtime Profile

## 1. Architectural Purpose & Overview

The `wasm` runtime profile targets browser-based and web runtime execution of BASIC++ v6.5.2 compiled via Emscripten / WebAssembly (`wasm32-unknown-emscripten`).

### Key Architectural Specifications:
- **Canvas Virtualization**: Bridges BGI graphics and text modes to HTML5 `<canvas>` via WebGL / 2D context.
- **Audio Virtualization**: Bridges `PLAY` and `SOUND` statements to the Web Audio API.
- **Async Execution Loop**: Implements an asynchronous VM scheduler loop to yield control back to the browser's event loop (`emscripten_set_main_loop`).
- **Virtual File System (MEMFS/IDBFS)**: Maps local file I/O to in-memory virtual files and IndexedDB for persistent storage.

---

## 2. Compilation & Embedding

```bash
emcmake cmake -B build_wasm -DCMAKE_BUILD_TYPE=Release
cmake --build build_wasm --target baspp_wasm
```

```html
<canvas id="basic-canvas" width="640" height="400"></canvas>
<script src="baspp.js"></script>
```
