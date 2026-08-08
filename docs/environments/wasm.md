# WAP: Web Application BASIC (`wap` / `wap.wasm` / `wap.js`)

## Overview
**WAP (Web Application BASIC)** compiles the complete BASIC++ VM and C SDK into WebAssembly using Emscripten, enabling client-side execution in all modern web browsers, Progressive Web Apps (PWAs), and Node.js.

## Binary Artifacts & Bindings
- **`wap.wasm`**: Compiled WebAssembly bytecode module
- **`wap.js`**: Emscripten JavaScript glue library
- **`bindings/wasm/basicpp.d.ts`**: TypeScript definitions for web applications

## Features & Browser Compatibility
- **Zero Server Overhead**: Evaluates BASIC++ scripts 100% client-side in the browser V8 / SpiderMonkey JS engine
- **Interactive Sandbox**: Powers the live Web Developer Playground in `docs/web/index.html`
- **DOM & Canvas Binding**: Bridge BASIC++ drawing and console output directly to HTML5 `<canvas>` elements

## Web JavaScript Usage Example
```javascript
import { loadWap } from './bindings/wasm/basicpp.js';

const wap = await loadWap();
const ctx = wap.bpp_init(16 * 1024 * 1024);
wap.bpp_set_var_num(ctx, "ANGLE", 45.0);
console.log("Sine Result:", wap.bpp_eval_expr(ctx, "SIN(ANGLE * 3.14159 / 180)"));
wap.bpp_shutdown(ctx);
```
