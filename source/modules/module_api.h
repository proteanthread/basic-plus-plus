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
 // 1. Include this header in your C project.
 // 2. Define `bpp_module_init` (which must be exported, e.g. __declspec(dllexport) on Windows).
 // 3. Inside `bpp_module_init`, you MUST call `module_register` with a `ModuleInfo` structure.
 // 4. To register custom functions, call `funcreg_register_ext` from your `init` callback.
 // 5. Define `bpp_module_cleanup` if you need to free resources when the module is unloaded.
 //
 // HOW TO ACCESS EXTERNAL MODULES IN BASIC++:
 // - Load them: `LOAD MODULE "my_extension.dll"`
 // - Execute them: `EXEC MODULE "my_extension.dll"` (triggers their logic)
 // - Unload them: `UNLOAD MODULE "my_extension.dll"` (frees memory/prevents leaks)
 //
 // SECURITY:
 // Modules specify their capability needs. The interpreter dynamically
 // matches these against the host environment. If your module requires 
 // CAP_FILE but the interpreter is running in RESTRICTED mode, your 
 // module's `init` callback may be denied or downgraded dynamically.
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
