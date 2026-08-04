# Tutorial: How to Write a Module Plugin

> **Purpose**: Guide to building modular shared library plugins (.dll or .so) that can be loaded dynamically by the VM.

---

## Step 1: Create Module Entry Point
Every plugin must export a validation and activation handler:
```c
#include "module.h"

#ifdef _WIN32
#define BASIC_MODULE_EXPORT __declspec(dllexport)
#else
#define BASIC_MODULE_EXPORT
#endif

BASIC_MODULE_EXPORT BppModuleInfo module_init(void) {
    BppModuleInfo info = {0};
    info.name = "MyMathPlugin";
    info.version = "1.0.0";
    return info;
}

BASIC_MODULE_EXPORT void module_activate(struct VMContext *vm) {
    (void)vm;
}
```
