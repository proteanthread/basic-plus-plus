# Tutorial: How to Embed BASIC++

> **Purpose**: A step-by-step guide to adding BASIC++ as an embedded scripting engine inside a third-party C/C++ application.

---

## Step 1: Complete Embedding Program
Below is a complete, compile-ready C program showing the configuration, booting, statement execution, and shutdown sequence:
```c
#include "bpp_boot.h"
#include "bpp_vm.h"
#include "bpp_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    BootConfig config;
    memset(&config, 0, sizeof(config));
    config.is_repl = false;
    config.prog_mem = 64 * 1024 * 1024;
    config.var_mem = 64 * 1024 * 1024;
    config.str_mem = 64 * 1024 * 1024;
    config.scratch_mem = 64 * 1024 * 1024;

    BootContext *boot = boot_execute(&config);
    if (!boot) {
        return 1;
    }
    VMContext *vm = boot->vm;
    
    BppError err = vm_execute_line(vm, "PRINT \"Hello from C Host!\"");
    if (err.code != 0) {
        printf("Error: %s\n", err.message);
    }
    
    boot_shutdown(boot);
    return 0;
}
```
