# `embedding` C/C++ Host Application Embedding API

## 1. Architectural Purpose & Overview

The `embedding` interface allows host C/C++ applications to instantiate, execute, and control the BASIC++ virtual machine as an embedded scripting engine.

### Key Architectural Invariants:
- **Encapsulated State**: Multiple independent `VMContext` instances can execute concurrently across threads.
- **Zero Host Leakage**: All interpreter memory is contained within the instance memory pool.

---

## 2. Technical Embedding Example (C17)

```c
#include "vm/vm.h"
#include "boot/boot.h"

int main(int argc, char **argv) {
    VMContext *vm = vm_create(64 * 1024 * 1024); // 64 MB pool
    vm_execute_line(vm, "PRINT \"Hello from embedded BASIC++ VM!\"");
    vm_destroy(vm);
    return 0;
}
```
