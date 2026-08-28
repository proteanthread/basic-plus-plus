# `file` Sequential & Random File I/O Engine (`libscript`)

## 1. Architectural Purpose & Overview

The `file` subsystem (`engine/src/runtime/file.c`) implements sequential, binary, and random-access disk file operations in BASIC++.

### Key Architectural Invariants:
- **Channel Table**: Maps file numbers (`#1` through `#255`) to file control blocks.
- **Record Buffers**: Backs `FIELD`, `GET`, `PUT`, `LSET`, `RSET`, `CVI`, `CVS`, `CVD`, `MKI$`, `MKS$`, `MKD$`.

---

## 2. Technical API Signatures (C17)

```c
int file_open(int channel, const char *path, FileMode mode, int record_len);
int file_close(int channel);
int file_get(int channel, long record_num);
int file_put(int channel, long record_num);
```
