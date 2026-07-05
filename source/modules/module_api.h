/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: module_api.h
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    C-level modular expansions providing hardware wrappers and runtime libraries.
 *
 * 2. WHAT TO EXPECT:
 *    Modules register customized functions at boot to dynamically extend vocabulary.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Module naming, registered commands list, setup/shutdown details.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Module lifecycle dispatcher, keyword override bindings.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    Ensure mod_init does not fail. Verify that linkage matches build profiles.
 * ===================================================================== */

 // ---
 // BASIC++ Interpreter - module_api.h
 // ---
 //
 // External Module API Definition.
 //
 // PURPOSE:
 // This header is designed to be included by external C developers
 // who wish to compile native modules (.dll on Windows, .so on Linux)
 // for the BASIC++ interpreter.
 //
 // HOW TO WRITE EXTERNAL EXTENSIONS:
 // 1. Use the unified `BPP_EXPORT` macro to define the entry point `bpp_module_init`.
 // 2. Platform-specific dynamic loading (wlink option eliminate / GCC -flto) is abstracted internally.
 //    Avoid platform-specific dynamic loading functions (like raw Windows LoadLibrary or Linux dlopen) to
 //    prevent Shared Library Divergence.
 // 3. Never allocate memory using raw standard `malloc`, `free`, or `realloc`. To prevent Memory Corruption
 //    from raw allocations, always use the interpreter-managed allocators `rt_malloc(rt, size)` and `rt_free(rt, ptr)`
 //    passed in via the RuntimeState pointer.
 // 4. Do not attempt to bypass safety checks by passing raw host pointers to BASIC variables.
 // 5. Do not modify core instruction definitions or bytecode structures (to keep VM engine execution deterministic).
 //
 // HOW TO WRITE EXTERNAL FUNCTIONS:
 // 1. Prefix all function names with your module name (e.g. `MYMOD_ADD`) to avoid Symbol Collisions.
 // 2. Register functions inside your module's `init` callback using the Function Registry.
 // 3. Use soft runtime arguments signature matching (e.g., "SD" -> String, Double). The runtime engine validates
 //    types dynamically when executing calls, trapping signature mismatches without crashing the interpreter.
 //
 // HOW TO WRITE EXTERNAL MODULES:
 // 1. Statically declare a metadata block using the `ModuleInfo` structure.
 // 2. Use capability bitfields (`CAP_MATH`, `CAP_FILE`, etc.) to specify needed system access.
 // 3. The loading pipeline (Validation -> Capability Verification -> Sandbox Allocation -> Registration -> Activation)
 //    will reject or isolate the module if it demands capabilities forbidden by the active security Sandbox mode.
 // 4. Implement cleanup callbacks to ensure full resource cleanup during dynamic reloading (`UNLOAD MODULE` / `LOAD MODULE`).
 //
 // HOW TO WRITE EXTERNAL PLUGINS (VIRTUAL DEVICES):
 // 1. Set your `mod_class` to `MOD_DEVICE`.
 // 2. Map virtual address space blocks (e.g., historical console or hardware buffers) inside the virtual device.
 // 3. Interface with the VM using the PEEK and POKE virtual device wrappers in `vdev.c`. Standard BASIC programs
 //    will interact with your device driver via safe virtualization boundaries, leaving the host OS protected.
 //
 // ---

#ifndef BASICPP_MODULE_API_H
#define BASICPP_MODULE_API_H

#ifdef _WIN32
#define BPP_EXPORT __declspec(dllexport)
#else
#define BPP_EXPORT
#endif

// Provide minimal types if not included with core headers
#ifndef BASICPP_MODULE_H
#include "../module.h"
#endif

 // bpp_module_init - Entry point for dynamically loaded modules.
 //
 // This function must be defined and exported by your shared library.
 // It is called immediately after LoadLibrary/dlopen.
 //
 // You must use this function to register your ModuleInfo struct.
 // Example:
 //   BPP_EXPORT int bpp_module_init(void) {
 //       return module_register(&my_module_info);
 //   }
BPP_EXPORT int bpp_module_init(void);

 // bpp_module_cleanup - Cleanup point for dynamically loaded modules.
 //
 // This function is called immediately before FreeLibrary/dlclose.
 // Use it to free any internal allocations or OS resources.
BPP_EXPORT void bpp_module_cleanup(void);

#endif // BASICPP_MODULE_API_H
