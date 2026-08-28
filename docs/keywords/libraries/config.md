# `config` Engine Configuration Headers (`libkernel`)

## 1. Architectural Purpose & Overview

The `config` headers (`engine/include/types/config.h`) declare build-time memory bounds, buffer sizes, channel capacity limits, and system feature gates.

### Key Configuration Constants:
- `DEFAULT_MEM_POOL_STANDARD`: `671088640L` bytes (640 MB).
- `DEFAULT_MEM_POOL_LITE`: `402653184L` bytes (384 MB).
- `DEFAULT_MEM_POOL_SCRIPT`: `67108864L` bytes (64 MB).
- `MAX_OPEN_FILES`: 256 active I/O channels.
- `MAX_ARRAY_DIMS`: 8 dimensions.

---

## 2. Header Definitions (C17)

```c
#define BPP_VERSION_MAJOR 6
#define BPP_VERSION_MINOR 5
#define BPP_VERSION_PATCH 2
```
