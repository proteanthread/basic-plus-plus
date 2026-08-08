# BASIC++ Desktop Environment Edition (`basicpp.dll` / `libbasicpp.so` / `libbasicpp.dylib`)

## Overview
The **Desktop Environment Edition** is the flagship C SDK shared library and interactive workstation build for Windows, Linux, and macOS.

## Binary Artifacts
- **Windows**: `basicpp.dll` & `basicpp.lib` (MSVC / MinGW)
- **Linux**: `libbasicpp.so` (GCC / Clang)
- **macOS**: `libbasicpp.dylib` (Clang)
- **Workstation CLI**: `baspp.exe` / `baspp`

## Hardware & System Requirements
- **Memory Pool**: 640 MB default heap memory (`671088640L` bytes)
- **Graphics & Audio**: Delay-loaded SDL2 (`SDL2.dll` / `libSDL2.so`) for `SCREEN`, `PSET`, `LINE`, `CIRCLE`, `PAINT`, `SOUND`, `PLAY`
- **TUI & Terminals**: Full ANSI/VT100 multi-window terminal multiplexer editor

## C17 Integration Example
```c
#include "bpp_api.h"
#include <stdio.h>

int main(void) {
    BppEngineContext *ctx = bpp_init(671088640);
    bpp_set_var_num(ctx, "X", 100.0);
    bpp_exec_string(ctx, "PRINT \"Desktop Engine Value:\"; X");
    bpp_shutdown(ctx);
    return 0;
}
```

## Python 3 Integration
```python
from basicpp import VM

vm = VM(ram_mb=640)
vm["COUNT"] = 50.0
print("Evaluated:", vm.eval("COUNT * 2"))
vm.close()
```
