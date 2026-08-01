# Tutorial: How to Write a Module Plugin

> **Purpose**: Guide to building modular shared library plugins (.dll or .so) that can be loaded dynamically by the VM.

---

## Step 1: Create Module Entry Point
Every plugin must export a validation and activation handler:
```c
#include "bpp_module.h"

#ifdef _WIN32
#define BPP_MODULE_EXPORT __declspec(dllexport)
#else
#define BPP_MODULE_EXPORT
#endif

BPP_MODULE_EXPORT BppModuleInfo bpp_module_init(void) {
    BppModuleInfo info = {0};
    info.name = "MyMathPlugin";
    info.version = "1.0.0";
    return info;
}

BPP_MODULE_EXPORT void bpp_module_activate(struct VMContext *vm) {
    (void)vm;
}
```
